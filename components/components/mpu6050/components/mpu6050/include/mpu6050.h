#ifndef MPU6050_H
#define MPU6050_H

#include "esp_err.h"
#include "driver/i2c.h"

// Endereço do MPU6050 (0x68 ou 0x69 se AD0 estiver em HIGH)
#define MPU6050_ADDRESS 0x68  

// Definições de registradores do MPU6050
#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_WHO_AM_I     0x75

// Configuração do barramento I2C
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 21  // Pino SDA
#define I2C_MASTER_SCL_IO 22  // Pino SCL
#define I2C_MASTER_FREQ_HZ 10000  // 🔹 Redução da velocidade do I2C

// Estrutura para armazenar os dados do MPU6050
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    float roll;
    float pitch;
} mpu6050_data_t;

// Declaração das funções
esp_err_t i2c_master_init();
esp_err_t mpu6050_write_register(uint8_t reg_addr, uint8_t data);
esp_err_t mpu6050_read_registers(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t test_mpu6050();
esp_err_t mpu6050_init();
esp_err_t mpu6050_read_data(mpu6050_data_t *sensor_data);
void calculate_euler_angles(mpu6050_data_t *sensor_data);

#endif // MPU6050_H
