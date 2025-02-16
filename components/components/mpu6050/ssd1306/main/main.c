#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"

void app_main() {
    ssd1306_init();

    while (1) {
        // 🔍 Verifica se o display ainda está conectado
        if (!ssd1306_check_connection()) {
            ssd1306_reconnect();
        }

        // Exibe mensagem principal
        ssd1306_display_text("SSD1306 OK!\n♥ → ★ ☹");
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Desenha um smiley face 🙂
        const uint8_t smiley_face[] = {
            0x00, 0x3C, 0x42, 0xA5, 0xA1, 0xA5, 0x42, 0x3C,
            0x00, 0x3C, 0x42, 0xA5, 0xA1, 0xA5, 0x42, 0x3C
        };
        ssd1306_display_image(smiley_face, 16, 16);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Desenha um coração ♥
        const uint8_t heart[] = {
            0x0C, 0x1E, 0x3F, 0x7F, 0xFF, 0xFE, 0x7C, 0x38,
            0x0C, 0x1E, 0x3F, 0x7F, 0xFF, 0xFE, 0x7C, 0x38
        };
        ssd1306_display_image(heart, 16, 16);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Desenha uma seta →
        const uint8_t arrow_right[] = {
            0x00, 0x08, 0x18, 0x38, 0x78, 0xF8, 0x78, 0x38,
            0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        ssd1306_display_image(arrow_right, 16, 16);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Desenha uma estrela ★
        const uint8_t star[] = {
            0x00, 0x10, 0x54, 0x38, 0x10, 0xFE, 0x10, 0x38,
            0x54, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        ssd1306_display_image(star, 16, 16);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Desenha um rosto triste ☹
        const uint8_t sad_face[] = {
            0x3C, 0x42, 0xA1, 0x81, 0xA5, 0x99, 0x42, 0x3C,
            0x3C, 0x42, 0xA1, 0x81, 0xA5, 0x99, 0x42, 0x3C
        };
        ssd1306_display_image(sad_face, 16, 16);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
