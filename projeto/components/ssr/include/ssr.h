#ifndef SSR_H
#define SSR_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

typedef enum {
    SSR_MODE_ON_OFF, 
    SSR_MODE_PWM     
} ssr_mode_t;

typedef struct {
    gpio_num_t gpio;          
    ssr_mode_t mode;          
    ledc_channel_t pwm_channel; 
    ledc_timer_t pwm_timer;     
    uint32_t pwm_freq;        
    uint8_t state;            
    uint8_t duty_cycle;       
} ssr_t;

/// Inicializa o SSR com base no modo configurado
esp_err_t ssr_init(ssr_t *ssr);

/// Liga ou desliga o SSR (somente no modo ON-OFF)
esp_err_t ssr_set_state(ssr_t *ssr, uint8_t state);

/// Alterna o estado do SSR (ON/OFF)
esp_err_t ssr_toggle(ssr_t *ssr);

/// Define o duty cycle do SSR no modo PWM (0-100%)
esp_err_t ssr_set_duty(ssr_t *ssr, uint8_t duty);

#endif // SSR_H
