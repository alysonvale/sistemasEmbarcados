#include "mpu6050.h"
#include <stdio.h>
#include <math.h>

// 🔹 Inicializar o I2C corretamente
esp_err_t i2c_master_init()
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // 🔹 Ativa pull-up interno
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // 🔹 Ativa pull-up interno
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    if (err != ESP_OK)
    {
        return err;
    }

    return i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0);
}

// 🔹 Escrever em um registrador do MPU6050
esp_err_t mpu6050_write_register(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDRESS, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

// 🔹 Ler múltiplos registradores do MPU6050
esp_err_t mpu6050_read_registers(uint8_t reg_addr, uint8_t *data, size_t len)
{
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDRESS, &reg_addr, 1, data, len, pdMS_TO_TICKS(1000));
    return err;
}

// 🔹 Teste de comunicação WHO_AM_I
esp_err_t test_mpu6050()
{
    uint8_t who_am_i;
    esp_err_t err = mpu6050_read_registers(MPU6050_REG_WHO_AM_I, &who_am_i, 1);

    if (err == ESP_OK && who_am_i == 0x68)
    {
        printf("✅ MPU6050 detectado corretamente!\n");
        return ESP_OK;
    }
    else
    {
        printf("❌ Erro ao detectar MPU6050! Código de erro: %d\n", err);
        return ESP_FAIL;
    }
}

// 🔹 Inicializar o MPU6050
esp_err_t mpu6050_init()
{
    esp_err_t ret;
    ret = mpu6050_write_register(MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) return ret;

    return ESP_OK;
}

// 🔹 Ler dados do MPU6050
esp_err_t mpu6050_read_data(mpu6050_data_t *sensor_data)
{
    uint8_t raw_data[14];
    esp_err_t ret = mpu6050_read_registers(MPU6050_REG_ACCEL_XOUT_H, raw_data, 14);
    
    if (ret == ESP_OK)
    {
        sensor_data->accel_x = (int16_t)(raw_data[0] << 8 | raw_data[1]);
        sensor_data->accel_y = (int16_t)(raw_data[2] << 8 | raw_data[3]);
        sensor_data->accel_z = (int16_t)(raw_data[4] << 8 | raw_data[5]);
        sensor_data->gyro_x  = (int16_t)(raw_data[8] << 8 | raw_data[9]);
        sensor_data->gyro_y  = (int16_t)(raw_data[10] << 8 | raw_data[11]);
        sensor_data->gyro_z  = (int16_t)(raw_data[12] << 8 | raw_data[13]);
    }
    return ret;
}

// 🔹 Calcular ângulos de Euler
void calculate_euler_angles(mpu6050_data_t *sensor_data)
{
    sensor_data->roll  = atan2(sensor_data->accel_y, sensor_data->accel_z) * 180 / M_PI;
    sensor_data->pitch = atan2(-sensor_data->accel_x, sqrt(sensor_data->accel_y * sensor_data->accel_y + sensor_data->accel_z * sensor_data->accel_z)) * 180 / M_PI;
}