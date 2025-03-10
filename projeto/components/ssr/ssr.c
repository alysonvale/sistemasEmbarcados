#include "ssr.h"
#include "esp_log.h"

#define TAG "SSR"

// Inicializa o SSR com base no modo configurado
esp_err_t ssr_init(ssr_t *ssr) {
    if (!ssr) return ESP_ERR_INVALID_ARG;

    ssr->state = 0; // Estado inicial desligado
    ssr->duty_cycle = 0; // PWM começa em 0%

    if (ssr->mode == SSR_MODE_ON_OFF) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << ssr->gpio),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t ret = gpio_config(&io_conf);
        ESP_LOGI(TAG, "SSR inicializado no modo ON-OFF");
        return ret;

    } else if (ssr->mode == SSR_MODE_PWM) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = ssr->pwm_timer,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .freq_hz = ssr->pwm_freq,
            .clk_cfg = LEDC_USE_APB_CLK
        };
        esp_err_t ret = ledc_timer_config(&ledc_timer);
        if (ret != ESP_OK) return ret;

        ledc_channel_config_t ledc_channel = {
            .gpio_num = ssr->gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = ssr->pwm_channel,
            .timer_sel = ssr->pwm_timer,
            .duty = 0,
            .hpoint = 0
        };
        ret = ledc_channel_config(&ledc_channel);
        ESP_LOGI(TAG, "SSR inicializado no modo PWM, freq: %lu Hz", (unsigned long) ssr->pwm_freq);
        return ret;
    }

    return ESP_ERR_INVALID_ARG;
}

// Liga ou desliga o SSR no modo ON-OFF
esp_err_t ssr_set_state(ssr_t *ssr, uint8_t state) {
    if (!ssr || ssr->mode != SSR_MODE_ON_OFF) return ESP_ERR_INVALID_ARG;

    ssr->state = state;
    esp_err_t ret = gpio_set_level(ssr->gpio, state);
    ESP_LOGI(TAG, "SSR %s", state ? "LIGADO" : "DESLIGADO");

    return ret;
}

// Alterna o estado do SSR (ON/OFF)
esp_err_t ssr_toggle(ssr_t *ssr) {
    if (!ssr || ssr->mode != SSR_MODE_ON_OFF) return ESP_ERR_INVALID_ARG;

    return ssr_set_state(ssr, !ssr->state);
}

// Ajusta o duty cycle no modo PWM (0-100%)
esp_err_t ssr_set_duty(ssr_t *ssr, uint8_t duty) {
    if (!ssr || ssr->mode != SSR_MODE_PWM) return ESP_ERR_INVALID_ARG;
    
    if (duty > 100) duty = 100; // Garante que não passa de 100%
    
    uint32_t duty_scaled = (duty * 1023) / 100;
    ssr->duty_cycle = duty;

    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, ssr->pwm_channel, duty_scaled);
    if (ret != ESP_OK) return ret;

    ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, ssr->pwm_channel);
    ESP_LOGI(TAG, "SSR PWM Duty: %d%%", duty);

    return ret;
}
