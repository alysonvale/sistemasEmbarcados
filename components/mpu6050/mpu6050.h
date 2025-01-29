#ifndef MPU6050_H
#define MPU6050_H

#include "driver/i2c.h"

// Endereço padrão do MPU6050
#define MPU6050_I2C_ADDR 0x68

// Registradores do MPU6050
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H 0x43
#define MPU6050_REG_TEMP_OUT_H 0x41

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temperature;
} mpu6050_data_t;

// Inicializa o MPU6050
esp_err_t mpu6050_init(i2c_port_t i2c_port, uint8_t i2c_address);

// Lê os valores do acelerômetro
esp_err_t mpu6050_read_accel(i2c_port_t i2c_port, uint8_t i2c_address, float *ax, float *ay, float *az);

// Lê os valores do giroscópio
esp_err_t mpu6050_read_gyro(i2c_port_t i2c_port, uint8_t i2c_address, float *gx, float *gy, float *gz);

// Lê a temperatura do sensor
esp_err_t mpu6050_read_temp(i2c_port_t i2c_port, uint8_t i2c_address, float *temperature);

// Lê todos os dados (acelerômetro, giroscópio, temperatura)
esp_err_t mpu6050_read_all(i2c_port_t i2c_port, uint8_t i2c_address, mpu6050_data_t *data);

#endif // MPU6050_H
