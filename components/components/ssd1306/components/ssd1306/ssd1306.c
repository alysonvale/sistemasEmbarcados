#include "ssd1306.h"
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "font8x8_basic.h"

#define TAG "SSD1306"
#define SDA_PIN GPIO_NUM_19
#define SCL_PIN GPIO_NUM_18
#define OLED_I2C_ADDRESS 0x3C
#define I2C_FREQ_HZ 400000  // Ajustado para 400kHz

static uint8_t erro_contador = 0;

// 🔧 **Inicializa o barramento I2C**
esp_err_t i2c_master_init() {
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ
    };
    i2c_param_config(I2C_NUM_0, &i2c_conf);
    return i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

// 🔍 **Verifica se o SSD1306 está conectado**
bool ssd1306_check_connection() {
    uint8_t test_data = 0x00;
    esp_err_t err = i2c_master_write_read_device(I2C_NUM_0, OLED_I2C_ADDRESS, &test_data, 1, &test_data, 1, pdMS_TO_TICKS(100));

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "✅ SSD1306 detectado!");
        return true;
    } else {
        ESP_LOGW(TAG, "⚠️ SSD1306 não detectado! Verifique conexões.");
        return false;
    }
}

// 🔧 **Reinicializa o barramento I2C e verifica conexão**
void ssd1306_reconnect() {
    ESP_LOGW(TAG, "⚠️ Erro de conexão com SSD1306. Tentando reconectar...");

    while (!ssd1306_check_connection()) {
        ESP_LOGE(TAG, "❌ SSD1306 desconectado! Aguardando reconexão...");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Aguarda reconexão
    }

    ESP_LOGI(TAG, "🔄 SSD1306 reconectado! Reiniciando ESP32...");
    esp_restart();  // Reinicia o ESP32 assim que o display for reconectado
}

// 🔧 **Envia comandos para o SSD1306**
static esp_err_t ssd1306_send_command(uint8_t command) {
    uint8_t buffer[2] = {OLED_CONTROL_BYTE_CMD_SINGLE, command};
    esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, OLED_I2C_ADDRESS, buffer, sizeof(buffer), pdMS_TO_TICKS(100));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "⚠️ Erro ao enviar comando 0x%X: %s", command, esp_err_to_name(err));
        ssd1306_reconnect();  // **Se falhar, tenta reconectar**
    }
    return err;
}

// 🔧 **Inicializa o SSD1306**
esp_err_t ssd1306_init() {
    i2c_master_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    if (!ssd1306_check_connection()) {
        ssd1306_reconnect();
    }

    esp_err_t err = ESP_OK;

    err |= ssd1306_send_command(OLED_CMD_DISPLAY_OFF);
    err |= ssd1306_send_command(0x20);  // Modo de endereçamento
    err |= ssd1306_send_command(0x00);  // Horizontal
    err |= ssd1306_send_command(OLED_CMD_SET_CHARGE_PUMP);
    err |= ssd1306_send_command(0x14);
    err |= ssd1306_send_command(OLED_CMD_SET_SEGMENT_REMAP);
    err |= ssd1306_send_command(OLED_CMD_SET_COM_SCAN_MODE);
    err |= ssd1306_send_command(OLED_CMD_DISPLAY_ON);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "✅ SSD1306 inicializado com sucesso!");
        erro_contador = 0;
    } else {
        ESP_LOGE(TAG, "❌ Falha ao inicializar SSD1306!");
        ssd1306_reconnect();
    }
    return err;
}

// 🔧 **Limpa a tela do SSD1306**
void ssd1306_clear_screen() {
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        uint8_t buffer[OLED_WIDTH + 1];
        buffer[0] = OLED_CONTROL_BYTE_DATA_STREAM;
        memset(&buffer[1], 0x00, OLED_WIDTH);
        i2c_master_write_to_device(I2C_NUM_0, OLED_I2C_ADDRESS, buffer, sizeof(buffer), pdMS_TO_TICKS(100));
    }
}

// 🔧 **Exibe texto no display**
void ssd1306_display_text(const char *text) {
    if (!ssd1306_check_connection()) {
        ssd1306_reconnect();
    }

    ssd1306_clear_screen();

    for (uint8_t i = 0; i < strlen(text); i++) {
        uint8_t char_map[8];
        memcpy(char_map, font8x8_basic_tr[(uint8_t)text[i]], 8);
        
        uint8_t buffer[9];
        buffer[0] = OLED_CONTROL_BYTE_DATA_STREAM;
        memcpy(&buffer[1], char_map, 8);

        if (i2c_master_write_to_device(I2C_NUM_0, OLED_I2C_ADDRESS, buffer, sizeof(buffer), pdMS_TO_TICKS(100)) != ESP_OK) {
            ssd1306_reconnect();
        }
    }
}

// 🔧 **Exibe imagem no display**
void ssd1306_display_image(const uint8_t *image, uint8_t width, uint8_t height) {
    if (!ssd1306_check_connection()) {
        ssd1306_reconnect();
    }

    ssd1306_clear_screen();

    for (uint8_t page = 0; page < height / 8; page++) {
        uint8_t buffer[width + 1];
        buffer[0] = OLED_CONTROL_BYTE_DATA_STREAM;
        memset(&buffer[1], 0x00, width);  
        memcpy(&buffer[1], &image[page * width], width);

        if (i2c_master_write_to_device(I2C_NUM_0, OLED_I2C_ADDRESS, buffer, sizeof(buffer), pdMS_TO_TICKS(100)) != ESP_OK) {
            ssd1306_reconnect();
        }
    }
}
