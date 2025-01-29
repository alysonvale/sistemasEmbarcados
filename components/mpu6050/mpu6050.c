#include "mpu6050.h"
#include "esp_log.h"

#define TAG "MPU6050"

// Função para escrever em um registrador do MPU6050
static esp_err_t mpu6050_write_register(i2c_port_t i2c_port, uint8_t i2c_address, uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Função para ler múltiplos bytes de um registrador
static esp_err_t mpu6050_read_registers(i2c_port_t i2c_port, uint8_t i2c_address, uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Inicializa o MPU6050
esp_err_t mpu6050_init(i2c_port_t i2c_port, uint8_t i2c_address) {
    esp_err_t ret = mpu6050_write_register(i2c_port, i2c_address, MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "MPU6050 inicializado com sucesso");
    } else {
        ESP_LOGE(TAG, "Erro ao inicializar MPU6050");
    }
    return ret;
}

// Lê os valores do acelerômetro
esp_err_t mpu6050_read_accel(i2c_port_t i2c_port, uint8_t i2c_address, float *ax, float *ay, float *az) {
    uint8_t data[6];
    esp_err_t ret = mpu6050_read_registers(i2c_port, i2c_address, MPU6050_REG_ACCEL_XOUT_H, data, 6);
    if (ret == ESP_OK) {
        *ax = (int16_t)((data[0] << 8) | data[1]) / 16384.0f;
        *ay = (int16_t)((data[2] << 8) | data[3]) / 16384.0f;
        *az = (int16_t)((data[4] << 8) | data[5]) / 16384.0f;
    }
    return ret;
}

// Lê os valores do giroscópio
esp_err_t mpu6050_read_gyro(i2c_port_t i2c_port, uint8_t i2c_address, float *gx, float *gy, float *gz) {
    uint8_t data[6];
    esp_err_t ret = mpu6050_read_registers(i2c_port, i2c_address, MPU6050_REG_GYRO_XOUT_H, data, 6);
    if (ret == ESP_OK) {
        *gx = (int16_t)((data[0] << 8) | data[1]) / 131.0f;
        *gy = (int16_t)((data[2] << 8) | data[3]) / 131.0f;
        *gz = (int16_t)((data[4] << 8) | data[5]) / 131.0f;
    }
    return ret;
}

// Lê a temperatura
esp_err_t mpu6050_read_temp(i2c_port_t i2c_port, uint8_t i2c_address, float *temperature) {
    uint8_t data[2];
    esp_err_t ret = mpu6050_read_registers(i2c_port, i2c_address, MPU6050_REG_TEMP_OUT_H, data, 2);
    if (ret == ESP_OK) {
        int16_t raw_temp = (int16_t)((data[0] << 8) | data[1]);
        *temperature = raw_temp / 340.0f + 36.53f;
    }
    return ret;
}

// Lê todos os dados
esp_err_t mpu6050_read_all(i2c_port_t i2c_port, uint8_t i2c_address, mpu6050_data_t *data) {
    esp_err_t ret = mpu6050_read_accel(i2c_port, i2c_address, &data->accel_x, &data->accel_y, &data->accel_z);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_read_gyro(i2c_port, i2c_address, &data->gyro_x, &data->gyro_y, &data->gyro_z);
    if (ret != ESP_OK) return ret;
    ret = mpu6050_read_temp(i2c_port, i2c_address, &data->temperature);
    return ret;
}
