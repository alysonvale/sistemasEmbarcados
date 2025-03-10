# **Biblioteca MPU6050 para ESP-IDF**

## **Visão Geral**
Esta biblioteca fornece uma interface fácil de usar para o sensor MPU6050 ao trabalhar com o **ESP-IDF** no ESP32. O MPU6050 é um dispositivo de rastreamento de movimento de **6 eixos**, combinando um **acelerômetro de 3 eixos**, um **giroscópio de 3 eixos** e um **sensor de temperatura integrado**.

A biblioteca permite:
- Ler os dados do **acelerômetro** (X, Y, Z).
- Ler os dados do **giroscópio** (X, Y, Z).
- Ler a **temperatura** do sensor.
- Integrar o sensor facilmente em projetos ESP32 via **I2C**.

---

## **Características**
- Inicialização e configuração completa do **MPU6050**.
- Comunicação via **I2C**.
- Funções para leitura do acelerômetro, giroscópio e temperatura.
- Implementação otimizada para eficiência e leveza.

---

## **Requisitos**

### **Hardware**
- **Placa ESP32**
- **Módulo MPU6050**
- **Conexões I2C** (SDA, SCL)

### **Software**
- **ESP-IDF** instalado e configurado.
- **CMake/Make** configurado para compilar com ESP-IDF.

---

## **Conexões**

| **Pino MPU6050** | **Pino ESP32** |
|------------------|----------------|
| VCC              | 3.3V           |
| GND              | GND            |
| SDA              | GPIO 21        |
| SCL              | GPIO 22        |

⚠️ **Nota:** Certifique-se de que os resistores de pull-up **(4.7kΩ)** estejam conectados nas linhas SDA e SCL.

---

## **Instalação**
1. Clone este repositório dentro da pasta `components/` do seu projeto ESP-IDF:
   ```bash
   git clone https://github.com/alysonvale/mpu6050.git
