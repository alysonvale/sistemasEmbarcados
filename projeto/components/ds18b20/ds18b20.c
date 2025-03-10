#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#include "ds18b20.h"
#include "owb.h"

static const char * TAG = "ds18b20";

// Tempo máximo de conversão a 12 bits (em ms)
static const int T_CONV_MS = 750;

// Comandos do DS18B20
#define DS18B20_FUNCTION_TEMP_CONVERT       0x44
#define DS18B20_FUNCTION_SCRATCHPAD_WRITE   0x4E
#define DS18B20_FUNCTION_SCRATCHPAD_READ    0xBE
#define DS18B20_FUNCTION_SCRATCHPAD_COPY    0x48
#define DS18B20_FUNCTION_EEPROM_RECALL      0xB8
#define DS18B20_FUNCTION_POWER_SUPPLY_READ  0xB4

// Scratchpad do DS18B20: 9 bytes
typedef struct
{
    uint8_t temperature[2];    // [0] LSB, [1] MSB
    uint8_t trigger_high;
    uint8_t trigger_low;
    uint8_t configuration;
    uint8_t reserved[3];
    uint8_t crc;
} __attribute__((packed)) Scratchpad;


static void _init(DS18B20_Info * ds, const OneWireBus * bus)
{
    if (ds)
    {
        ds->bus = bus;
        memset(&ds->rom_code, 0, sizeof(ds->rom_code));
        ds->use_crc = false;
        ds->resolution = DS18B20_RESOLUTION_INVALID;
        ds->solo = false;
        ds->init = true;
    }
    else
    {
        ESP_LOGE(TAG, "_init: ds18b20_info is NULL");
    }
}

static bool _is_init(const DS18B20_Info * ds)
{
    if (!ds)
    {
        ESP_LOGE(TAG, "ds18b20_info is NULL");
        return false;
    }
    if (!ds->init)
    {
        ESP_LOGE(TAG, "ds18b20_info is not initialised");
        return false;
    }
    return true;
}

static bool _address_device(const DS18B20_Info * ds)
{
    bool present = false;
    if (_is_init(ds))
    {
        owb_reset(ds->bus, &present);
        if (present)
        {
            if (ds->solo)
            {
                // Só há 1 sensor no barramento => Skip ROM
                owb_write_byte(ds->bus, OWB_ROM_SKIP);
            }
            else
            {
                // Vários sensores => Match ROM
                owb_write_byte(ds->bus, OWB_ROM_MATCH);
                owb_write_rom_code(ds->bus, ds->rom_code);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Nenhum dispositivo responde no barramento (_address_device)");
        }
    }
    return present;
}


static bool _check_resolution(DS18B20_RESOLUTION r)
{
    return (r >= DS18B20_RESOLUTION_9_BIT && r <= DS18B20_RESOLUTION_12_BIT);
}


static float _decode_temp(uint8_t lsb, uint8_t msb, DS18B20_RESOLUTION resolution)
{
    float result = 0.0f;
    if (_check_resolution(resolution))
    {
        // Máscaras para remover bits indefinidos (< 12 bits)
        static const uint8_t mask[4] = { ~0x07, ~0x03, ~0x01, ~0x00 };
        int index = resolution - DS18B20_RESOLUTION_9_BIT;  
        uint8_t lsb_masked = mask[index] & lsb;

        int16_t raw = (msb << 8) | lsb_masked;
        result = raw / 16.0f;
    }
    else
    {
        ESP_LOGE(TAG, "_decode_temp: resolução inválida: %d", resolution);
    }
    return result;
}


static DS18B20_ERROR _read_scratchpad(const DS18B20_Info * ds, Scratchpad * sp)
{
    if (!sp)
    {
        return DS18B20_ERROR_NULL;
    }

    DS18B20_ERROR err = DS18B20_ERROR_UNKNOWN;

    if (_address_device(ds))
    {
        // Comando "Read Scratchpad"
        if (owb_write_byte(ds->bus, DS18B20_FUNCTION_SCRATCHPAD_READ) == OWB_STATUS_OK)
        {
           
            if (owb_read_bytes(ds->bus, (uint8_t *)sp, sizeof(*sp)) == OWB_STATUS_OK)
            {
                err = DS18B20_OK;
                if (ds->use_crc)
                {
                   
                    if (owb_crc8_bytes(0, (uint8_t *)sp, sizeof(*sp)) != 0)
                    {
                        ESP_LOGE(TAG, "Scratchpad com CRC inválido");
                        err = DS18B20_ERROR_CRC;
                    }
                }
                else
                {
                   
                    bool dummy_present;
                    owb_reset(ds->bus, &dummy_present);
                }
            }
            else
            {
                ESP_LOGE(TAG, "falha owb_read_bytes");
                err = DS18B20_ERROR_OWB;
            }
        }
        else
        {
            ESP_LOGE(TAG, "falha owb_write_byte(SCRATCHPAD_READ)");
            err = DS18B20_ERROR_OWB;
        }
    }
    else
    {
        err = DS18B20_ERROR_DEVICE;
    }

    return err;
}

static bool _write_scratchpad(const DS18B20_Info * ds, const Scratchpad * sp)
{
    bool ok = false;
    if (_is_init(ds))
    {
        if (_address_device(ds))
        {
            
            owb_write_byte(ds->bus, DS18B20_FUNCTION_SCRATCHPAD_WRITE);
            owb_write_bytes(ds->bus, (uint8_t *)&sp->trigger_high, 3);
            ok = true;
        }
    }
    return ok;
}

// ======================================================
// ============ Implementações das Funções API ==========
// ======================================================

DS18B20_Info * ds18b20_malloc(void)
{
    DS18B20_Info * ds = malloc(sizeof(*ds));
    if (ds)
    {
        memset(ds, 0, sizeof(*ds));
    }
    else
    {
        ESP_LOGE(TAG, "malloc falhou");
    }
    return ds;
}

void ds18b20_free(DS18B20_Info ** ds_ptr)
{
    if (ds_ptr && *ds_ptr)
    {
        free(*ds_ptr);
        *ds_ptr = NULL;
    }
}

void ds18b20_init(DS18B20_Info * ds, const OneWireBus * bus, OneWireBus_ROMCode rom_code)
{
    if (ds)
    {
        _init(ds, bus);
        ds->rom_code = rom_code;  // salva qual dispositivo
        ds->resolution = ds18b20_read_resolution(ds);
    }
    else
    {
        ESP_LOGE(TAG, "ds18b20_init: ds é NULL");
    }
}

void ds18b20_init_solo(DS18B20_Info * ds, const OneWireBus * bus)
{
    if (ds)
    {
        _init(ds, bus);
        ds->solo = true;
        ds->resolution = ds18b20_read_resolution(ds);
    }
    else
    {
        ESP_LOGE(TAG, "ds18b20_init_solo: ds é NULL");
    }
}

void ds18b20_use_crc(DS18B20_Info * ds, bool use_crc)
{
    if (_is_init(ds))
    {
        ds->use_crc = use_crc;
    }
}

bool ds18b20_set_resolution(DS18B20_Info * ds, DS18B20_RESOLUTION r)
{
    if (!_is_init(ds))
    {
        return false;
    }

    if (!_check_resolution(r))
    {
        ESP_LOGE(TAG, "Resolução inválida: %d", r);
        return false;
    }

    // Lê scratchpad completo
    Scratchpad sp = {0};
    DS18B20_ERROR err = _read_scratchpad(ds, &sp);
    if (err == DS18B20_OK || err == DS18B20_ERROR_CRC)
    {
        
        uint8_t conf_value = (((r - 1) & 0x03) << 5) | 0x1F;
        sp.configuration = conf_value;

    
        if (_write_scratchpad(ds, &sp))
        {
            ds->resolution = r;
            return true;
        }
        else
        {
            ESP_LOGE(TAG, "Falha ao escrever config no scratchpad");
            return false;
        }
    }
    return false;
}

DS18B20_RESOLUTION ds18b20_read_resolution(DS18B20_Info * ds)
{
    DS18B20_RESOLUTION r = DS18B20_RESOLUTION_INVALID;
    if (_is_init(ds))
    {
        Scratchpad sp = {0};
        DS18B20_ERROR err = _read_scratchpad(ds, &sp);
        if (err == DS18B20_OK || err == DS18B20_ERROR_CRC)
        {
            uint8_t bits = (sp.configuration >> 5) & 0x03;  // R1..R0
            DS18B20_RESOLUTION tmp = bits + DS18B20_RESOLUTION_9_BIT;
            if (_check_resolution(tmp))
            {
                r = tmp;
            }
            else
            {
                ESP_LOGE(TAG, "Config indica resolução inválida: 0x%02X", sp.configuration);
            }
        }
    }
    return r;
}

OneWireBus_ROMCode ds18b20_read_rom(DS18B20_Info * ds)
{
    OneWireBus_ROMCode code = {0};
    if (_is_init(ds))
    {
        if (!ds->solo)
        {
            ESP_LOGW(TAG, "ds18b20_read_rom() só funciona se solo=true");
            return code;
        }

        bool presence;
        owb_reset(ds->bus, &presence);
        if (presence)
        {
            // Comando “Read ROM”
            owb_write_byte(ds->bus, OWB_ROM_READ);
            // Chama owb_read_rom() do owb.h
            owb_read_rom(ds->bus, &code);
        }
    }
    return code;
}


bool ds18b20_convert(const DS18B20_Info * ds)
{
    if (_is_init(ds))
    {
        if (_address_device(ds))
        {
            owb_write_byte(ds->bus, DS18B20_FUNCTION_TEMP_CONVERT);
            return true;
        }
    }
    return false;
}

void ds18b20_convert_all(const OneWireBus * bus)
{
    if (bus)
    {
        bool presence;
        owb_reset(bus, &presence);
        if (presence)
        {
            owb_write_byte(bus, OWB_ROM_SKIP);
            owb_write_byte(bus, DS18B20_FUNCTION_TEMP_CONVERT);
            // Se estiver em parasita, ligue strong pull-up aqui.
        }
    }
}


float ds18b20_wait_for_conversion(const DS18B20_Info * ds)
{
    if (!_is_init(ds))
    {
        return 0.0f;
    }

    // Cada bit a menos reduz o tempo pela metade
    int shift = DS18B20_RESOLUTION_12_BIT - ds->resolution; 
    if (shift < 0 || shift > 3)
    {
        shift = 0; // fallback
    }

    int wait_ms = T_CONV_MS >> shift;
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    return (float)wait_ms;
}


DS18B20_ERROR ds18b20_read_temp(const DS18B20_Info * ds, float * out_value)
{
    if (!_is_init(ds))
    {
        return DS18B20_ERROR_UNKNOWN;
    }

    // Lê scratchpad completo
    Scratchpad sp = {0};
    DS18B20_ERROR err = _read_scratchpad(ds, &sp);
    if (err == DS18B20_OK || err == DS18B20_ERROR_CRC)
    {
       
        if (sp.reserved[1] == 0x0c && sp.temperature[1] == 0x05 && sp.temperature[0] == 0x50)
        {
            ESP_LOGE(TAG, "Leu valor de power-on (85.0), possivel DS18B20 não configurado");
            err = DS18B20_ERROR_DEVICE;
        }
        else
        {
            float temp = _decode_temp(sp.temperature[0], sp.temperature[1], ds->resolution);
            if (out_value)
            {
                *out_value = temp;
            }
            err = DS18B20_OK;
        }
    }
    return err;
}

DS18B20_ERROR ds18b20_convert_and_read_temp(const DS18B20_Info * ds, float * out_value)
{
    if (!_is_init(ds))
    {
        return DS18B20_ERROR_UNKNOWN;
    }

    DS18B20_ERROR err = DS18B20_ERROR_DEVICE;

    // 1) Inicia conversão
    if (ds18b20_convert(ds))
    {
        // 2) Espera tempo máximo
        ds18b20_wait_for_conversion(ds);

        // 3) Lê temperatura
        err = ds18b20_read_temp(ds, out_value);
    }
    return err;
}

DS18B20_ERROR ds18b20_check_for_parasite_power(const OneWireBus * bus, bool * present)
{
    if (!bus || !present)
    {
        return DS18B20_ERROR_NULL;
    }

    bool rst_present;
    DS18B20_ERROR err = (DS18B20_ERROR)owb_reset(bus, &rst_present);
    if (err == DS18B20_OK && rst_present)
    {
        owb_write_byte(bus, OWB_ROM_SKIP);
        owb_write_byte(bus, DS18B20_FUNCTION_POWER_SUPPLY_READ);

        uint8_t bit = 1;
        owb_read_bit(bus, &bit);

        // bit = 0 => parasita
        *present = (bit == 0);
    }
    return err;
}
