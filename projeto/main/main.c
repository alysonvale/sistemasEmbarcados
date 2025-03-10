#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "owb.h"
#include "owb_gpio.h"
#include "ds18b20.h"
#include "ssd1306.h"
#include "ssr.h"
#include "mpu6050.h"

static const char *TAG = "APP_MAIN";

// GPIO do sensor de temperatura DS18B20
#define ONE_WIRE_PIN  (12)

// Definições do LED e Botão
#define BUTTON_PIN GPIO_NUM_11  // Pino do botão
#define LED_PIN GPIO_NUM_9      // Pino do LED (ou SSR)
#define TEMPERATURE_THRESHOLD 28.5  // Temperatura limite para acionar o LED

// I2C para MPU6050
#define I2C_MASTER_SDA 17
#define I2C_MASTER_SCL 18
#define I2C_MASTER_NUM I2C_NUM_0

// Controle do LED via botão
typedef enum {
    BUTTON_EVENT_TOGGLE_LED
} button_event_t;

static QueueHandle_t button_event_queue = NULL;
static volatile int led_on = 0;
static volatile int led_intensity = 0;
static ssr_t ssr;

// MPU6050
#define MAX_ERROS 3  
TaskHandle_t mpuTaskHandle = NULL;

void mpu_task(void *pvParameter);


// Função para reiniciar o sistema caso o MPU6050 falhe
void reset_system()
{
    ESP_LOGW(TAG, "🔄 Reiniciando MPU6050 e I2C...");

    if (mpuTaskHandle != NULL) {
        vTaskDelete(mpuTaskHandle);
        mpuTaskHandle = NULL;
        ESP_LOGI(TAG, "✅ Task mpu_task deletada!");
    }

    i2c_driver_delete(I2C_MASTER_NUM);
    vTaskDelay(pdMS_TO_TICKS(500));

    if (i2c_master_init() == ESP_OK && mpu6050_init() == ESP_OK) {
        ESP_LOGI(TAG, "✅ MPU6050 e I2C reinicializados com sucesso!");
        xTaskCreate(mpu_task, "mpu_task", 4096, NULL, 5, &mpuTaskHandle);
    } else {
        ESP_LOGE(TAG, "⚠️ Erro ao reinicializar MPU6050! Tentando novamente...");
        vTaskDelay(pdMS_TO_TICKS(5000));
        reset_system();
    }
}

// Task para ler dados do MPU6050 e detectar vibração
void mpu_task(void *pvParameter)
{
    mpu6050_data_t sensor_data;
    int falha_count = 0;

    while (1)
    {
        if (mpu6050_read_data(&sensor_data) == ESP_OK) {
            calculate_euler_angles(&sensor_data);
            ESP_LOGI(TAG, "Accel: X=%d, Y=%d, Z=%d | Roll: %.2f° | Pitch: %.2f°",
                     sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
                     sensor_data.roll, sensor_data.pitch);

            falha_count = 0;  

            // Se houver uma vibração detectável, acende o LED
            if (abs(sensor_data.accel_x) > 3000 || abs(sensor_data.accel_y) > 3000 || abs(sensor_data.accel_z) > 3000) {
                led_on = 1;
                led_intensity = 100;
                ssr_set_duty(&ssr, led_intensity);
                ESP_LOGW(TAG, "⚠️ Vibração detectada! LED ACESO!");
            }

        } else {
            ESP_LOGE(TAG, "❌ Falha ao ler MPU6050");
            falha_count++;

            if (falha_count >= MAX_ERROS) {
                reset_system(); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Interrupção do botão
void IRAM_ATTR button_isr_handler(void *arg) {
    button_event_t event = BUTTON_EVENT_TOGGLE_LED;
    xQueueSendFromISR(button_event_queue, &event, NULL);
}

// Task para processar eventos do botão
void button_task(void *arg) {
    button_event_t event;

    while (1) {
        if (xQueueReceive(button_event_queue, &event, portMAX_DELAY)) {
            led_on = !led_on;  
            if (led_on) {
                led_intensity = 100;
                ssr_set_duty(&ssr, led_intensity);
                ESP_LOGI(TAG, "LED LIGADO manualmente.");
            } else {
                ssr_set_duty(&ssr, 0);
                ESP_LOGI(TAG, "LED DESLIGADO manualmente.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task para leitura da temperatura e controle do LED
// Task para leitura da temperatura e controle do LED
void temperature_task(void *arg) {
    ssd1306_init();

    owb_gpio_driver_info driver;
    OneWireBus *owb = owb_gpio_initialize(&driver, ONE_WIRE_PIN);
    owb_use_parasitic_power(owb, false);

    DS18B20_Info *ds18b20_info = ds18b20_malloc();
    ds18b20_init_solo(ds18b20_info, owb);
    ds18b20_use_crc(ds18b20_info, true);
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);

    char temp_text[32];

    while (1) {
        float temp_c = 0.0f;
        DS18B20_ERROR err = ds18b20_convert_and_read_temp(ds18b20_info, &temp_c);

        if (err == DS18B20_OK) {
            ESP_LOGI(TAG, "Temperatura: %.2f °C", temp_c);
            snprintf(temp_text, sizeof(temp_text), "Temp: %.2f C", temp_c);

            if (temp_c >= TEMPERATURE_THRESHOLD) {
                led_on = 1;
                led_intensity = 100;
                ssr_set_duty(&ssr, led_intensity);
                ESP_LOGW(TAG, "Temperatura alta! LED ACESO!");
            } else {
                led_on = 0;
                ssr_set_duty(&ssr, 0);
                ESP_LOGW(TAG, "Temperatura baixa! LED DESLIGADO!");
            }
        } else {
            ESP_LOGE(TAG, "Erro ao ler temperatura");
            snprintf(temp_text, sizeof(temp_text), "Erro");
        }

        ssd1306_clear_screen();  // Apaga tudo
        ssd1306_display_text(temp_text); // Exibe o texto fixo no topo do display

        vTaskDelay(pdMS_TO_TICKS(2000));  // Atualiza a cada 2 segundos
    }

    ds18b20_free(&ds18b20_info);
}


void app_main(void) {
    ssr.gpio = LED_PIN;
    ssr.mode = SSR_MODE_PWM;
    ssr.pwm_channel = LEDC_CHANNEL_0;
    ssr.pwm_timer = LEDC_TIMER_0;
    ssr.pwm_freq = 1000;
    ssr_init(&ssr);

    button_event_queue = xQueueCreate(10, sizeof(button_event_t));
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
    xTaskCreate(temperature_task, "temperature_task", 4096, NULL, 10, NULL);
    xTaskCreate(mpu_task, "mpu_task", 4096, NULL, 5, &mpuTaskHandle);

    gpio_config_t button_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&button_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}
