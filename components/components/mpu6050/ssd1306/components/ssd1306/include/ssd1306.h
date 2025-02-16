#ifndef SSD1306_H
#define SSD1306_H

#include "esp_err.h"
#include "driver/i2c.h"

// Configurações do display SSD1306
#define OLED_I2C_ADDRESS 0x3C  // Endereço I2C do display
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES (OLED_HEIGHT / 8)

// Comandos do SSD1306
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

#define OLED_CMD_DISPLAY_ON  0xAF
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_SET_CONTRAST 0x81
#define OLED_CMD_SET_CHARGE_PUMP 0x8D
#define OLED_CMD_SET_SEGMENT_REMAP 0xA1
#define OLED_CMD_SET_COM_SCAN_MODE 0xC8

// 🔧 Inicialização do SSD1306
esp_err_t ssd1306_init();

// 🔄 Reinicializa o barramento I2C se houver falha e espera reconexão
void ssd1306_reconnect();

// 🔍 Verifica se o display SSD1306 está conectado
bool ssd1306_check_connection();

// 🔧 Limpa a tela do display
void ssd1306_clear_screen();

// 🔤 Exibe texto na tela
void ssd1306_display_text(const char *text);

// 🖼 Exibe uma imagem na tela
void ssd1306_display_image(const uint8_t *image, uint8_t width, uint8_t height);

#endif // SSD1306_H
