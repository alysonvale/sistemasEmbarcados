idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES mpu6050 driver
    PRIV_REQUIRES mpu6050
)
