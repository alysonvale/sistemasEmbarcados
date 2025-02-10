#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mpu6050.h"

#define MAX_ERROS 3  // Número máximo de falhas antes de reiniciar tudo

TaskHandle_t mpuTaskHandle = NULL;  // Handle da task para ser destruída

void reset_system()
{
    printf("🔄 Reiniciando I2C, MPU6050 e task...\n");

    // 1. Deletar a task atual antes de reiniciar o sistema
    if (mpuTaskHandle != NULL)
    {
        vTaskDelete(mpuTaskHandle);
        mpuTaskHandle = NULL;
        printf("✅ Task mpu_task deletada!\n");
    }

    // 2. Desinstalar driver I2C
    i2c_driver_delete(I2C_MASTER_NUM);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 3. Reinicializar I2C e MPU6050
    if (i2c_master_init() == ESP_OK && mpu6050_init() == ESP_OK)
    {
        printf("✅ MPU6050 e I2C reinicializados com sucesso!\n");

        // 4. Criar a task novamente
        xTaskCreate(mpu_task, "mpu_task", 4096, NULL, 5, &mpuTaskHandle);
        printf("🚀 Task mpu_task recriada com sucesso!\n");
    }
    else
    {
        printf("⚠️ Erro ao reinicializar MPU6050! Tentando novamente em 5s...\n");
        vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda antes de tentar reinicializar
        reset_system();  // Chama a função recursivamente até dar certo
    }
}

void mpu_task(void *pvParameter)
{
    mpu6050_data_t sensor_data;
    int falha_count = 0;

    while (1)
    {
        if (mpu6050_read_data(&sensor_data) == ESP_OK)
        {
            calculate_euler_angles(&sensor_data);
            printf("Accel: X=%d, Y=%d, Z=%d | Gyro: X=%d, Y=%d, Z=%d | Roll: %.2f° | Pitch: %.2f°\n",
                   sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
                   sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z,
                   sensor_data.roll, sensor_data.pitch);

            falha_count = 0;  // Resetar contador de falhas
        }
        else
        {
            printf("❌ Falha ao ler os dados do MPU6050\n");
            falha_count++;

            if (falha_count >= MAX_ERROS)
            {
                reset_system(); // Reinicializa completamente o sistema
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main()
{
    printf("🚀 Iniciando ESP32 com MPU6050...\n");

    if (i2c_master_init() != ESP_OK || test_mpu6050() != ESP_OK || mpu6050_init() != ESP_OK)
    {
        printf("❌ Falha ao configurar MPU6050! Reiniciando em 5s...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
        reset_system(); // Se falhar no início, tenta reinicializar
        return;
    }

    xTaskCreate(mpu_task, "mpu_task", 4096, NULL, 5, &mpuTaskHandle);
}
