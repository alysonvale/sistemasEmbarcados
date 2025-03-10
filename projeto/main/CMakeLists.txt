idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES ssr driver freertos owb ssd1306 ds18b20 mpu6050 

)
