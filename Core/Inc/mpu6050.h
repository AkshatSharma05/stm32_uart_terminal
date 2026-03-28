#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"

#define MPU6050_ADDR    (0x68<<1)
#define WHO_AM_I        0x75
#define PWR_MGMT_1      0x6B
#define MPU_ACCEL_ADDR  0x3B
#define MPU_GYRO_ADDR   0x43
#define MPU_TEMP_ADDR   0x41

#define ACCEL_CONFIG    0x1C
#define GYRO_CONFIG     0x1B

#define ACCEL_SCALE     16384.0f
#define GYRO_SCALE      131.0f


typedef struct {
    float x;
    float y;
    float z;
} MPU6050_Data;

HAL_StatusTypeDef mpu6050_init();
HAL_StatusTypeDef mpu6050_whoami(uint8_t *who);
HAL_StatusTypeDef mpu6050_readAccel(MPU6050_Data *accel);
HAL_StatusTypeDef mpu6050_readGyro(MPU6050_Data *gyro);

#endif