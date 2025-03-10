#ifndef DS18B20_H
#define DS18B20_H

#include "owb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DS18B20_ERROR_UNKNOWN = -1,  
    DS18B20_OK = 0,       
    DS18B20_ERROR_DEVICE,  
    DS18B20_ERROR_CRC,     
    DS18B20_ERROR_OWB,     
    DS18B20_ERROR_NULL,  
} DS18B20_ERROR;

typedef enum
{
    DS18B20_RESOLUTION_INVALID = -1,  
    DS18B20_RESOLUTION_9_BIT   = 9,  
    DS18B20_RESOLUTION_10_BIT  = 10,  
    DS18B20_RESOLUTION_11_BIT  = 11,  
    DS18B20_RESOLUTION_12_BIT  = 12, 
} DS18B20_RESOLUTION;

typedef struct
{
    bool init;                     
    bool solo;                    
    bool use_crc;                  
    const OneWireBus * bus;        
    OneWireBus_ROMCode rom_code;   
    DS18B20_RESOLUTION resolution; 
} DS18B20_Info;

DS18B20_Info * ds18b20_malloc(void);

void ds18b20_free(DS18B20_Info ** ds18b20_info);

void ds18b20_init(DS18B20_Info * ds18b20_info, const OneWireBus * bus, OneWireBus_ROMCode rom_code);

void ds18b20_init_solo(DS18B20_Info * ds18b20_info, const OneWireBus * bus);

void ds18b20_use_crc(DS18B20_Info * ds18b20_info, bool use_crc);

bool ds18b20_set_resolution(DS18B20_Info * ds18b20_info, DS18B20_RESOLUTION resolution);

DS18B20_RESOLUTION ds18b20_read_resolution(DS18B20_Info * ds18b20_info);

OneWireBus_ROMCode ds18b20_read_rom(DS18B20_Info * ds18b20_info);

bool ds18b20_convert(const DS18B20_Info * ds18b20_info);

void ds18b20_convert_all(const OneWireBus * bus);

float ds18b20_wait_for_conversion(const DS18B20_Info * ds18b20_info);

DS18B20_ERROR ds18b20_read_temp(const DS18B20_Info * ds18b20_info, float * value);

DS18B20_ERROR ds18b20_convert_and_read_temp(const DS18B20_Info * ds18b20_info, float * value);

DS18B20_ERROR ds18b20_check_for_parasite_power(const OneWireBus * bus, bool * present);

#ifdef __cplusplus
}
#endif

#endif  // DS18B20_H