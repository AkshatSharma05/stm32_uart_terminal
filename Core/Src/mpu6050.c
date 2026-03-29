#include "mpu6050.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CALIB_SAMPLES 500

extern I2C_HandleTypeDef hi2c1;

MPU6050_Data gyro_bias = {0, 0, 0};  

static uint8_t gyro_calibrated = 0; 

HAL_StatusTypeDef mpu6050_init(){
    //Wake Up MPU6050
    HAL_StatusTypeDef status;
    uint8_t data = 0x00;
    status =  HAL_I2C_Mem_Write(&hi2c1, (uint16_t)MPU6050_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);    
    if(status != HAL_OK) return status;
    
    HAL_Delay(100);

    //Accel and Gyro config -> range (+-) 2g
    status =  HAL_I2C_Mem_Write(&hi2c1, (uint16_t)MPU6050_ADDR, ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) return status;

    data = 0x10;
    status =  HAL_I2C_Mem_Write(&hi2c1, (uint16_t)MPU6050_ADDR, GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) return status;

    return status;
} 

HAL_StatusTypeDef mpu6050_calibrateGyro(void) {
    MPU6050_Data gyro;
    float sum_x = 0, sum_y = 0, sum_z = 0;

    // Discard first 50 samples-> maybe unstable
    for (int i = 0; i < 50; i++) {
        mpu6050_readGyro(&gyro);
        HAL_Delay(2);
    }

    for (int i = 0; i < CALIB_SAMPLES; i++) {
        HAL_StatusTypeDef status = mpu6050_readGyro(&gyro);
        if (status != HAL_OK) return status;

        sum_x += gyro.x;
        sum_y += gyro.y;
        sum_z += gyro.z;

        HAL_Delay(2);
    }

    gyro_bias.x = sum_x / CALIB_SAMPLES;
    gyro_bias.y = sum_y / CALIB_SAMPLES;
    gyro_bias.z = sum_z / CALIB_SAMPLES;
    gyro_calibrated = 1;

    return HAL_OK;
}

HAL_StatusTypeDef mpu6050_whoami(uint8_t *who){
    return HAL_I2C_Mem_Read(&hi2c1, (uint16_t)MPU6050_ADDR, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, who, 1, 100);
}

HAL_StatusTypeDef mpu6050_readAccel(MPU6050_Data *accel){
    HAL_StatusTypeDef status;

    uint8_t raw[6]; // x, y, z -> each with high and low components of 8 bit each 
    status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)MPU6050_ADDR, MPU_ACCEL_ADDR, I2C_MEMADD_SIZE_8BIT, raw, 6, 100);
    if(status != HAL_OK) return status;

    // int16_t because the output is signed 16 bit in 2s complement 
    accel->x = (int16_t)(raw[0] << 8 | raw[1]) / ACCEL_SCALE; 
    accel->y = (int16_t)(raw[2] << 8 | raw[3]) / ACCEL_SCALE;
    accel->z = (int16_t)(raw[4] << 8 | raw[5]) / ACCEL_SCALE;
    
    return status;
}

HAL_StatusTypeDef mpu6050_readGyro(MPU6050_Data *gyro){
    HAL_StatusTypeDef status;

    uint8_t raw[6]; // x, y, z -> each with high and low components of 8 bit each 
    status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)MPU6050_ADDR, MPU_GYRO_ADDR, I2C_MEMADD_SIZE_8BIT, raw, 6, 100);
    if(status != HAL_OK) return status;

    // int16_t because the output is signed 16 bit in 2s complement 
    gyro->x = (int16_t)(raw[0] << 8 | raw[1]) / GYRO_SCALE; 
    gyro->y = (int16_t)(raw[2] << 8 | raw[3]) / GYRO_SCALE;
    gyro->z = (int16_t)(raw[4] << 8 | raw[5]) / GYRO_SCALE;

    return status;
}

//Pass NULL in gyro to only work with accel -> only roll and pitch
MPU6050_Data mpu6050_rpy_accel(MPU6050_Data *accel, MPU6050_Data *gyro){
    MPU6050_Data rpy;

    // Convert radians to degrees
    rpy.x = atan2(accel->y, accel->z) * (180.0 / M_PI);
    rpy.y = atan2(-accel->x, sqrt(accel->y*accel->y + accel->z*accel->z)) * (180.0 / M_PI);

    if(gyro != NULL){
        static float prev_z = 0.0f;
        static uint32_t last_tick = 0;

        uint32_t now = HAL_GetTick();
        float dt;

        if(last_tick == 0) {
            dt = 0.0f;
        } else {
            dt = (now - last_tick) / 1000.0f; 
        }
        last_tick = now;

        rpy.z = prev_z + gyro->z * dt; 
        prev_z = rpy.z;

    } else rpy.z = 0; 

    return rpy;
}

MPU6050_Data mpu6050_rpy_gyro(MPU6050_Data *gyro) {
    MPU6050_Data rpy;
    static float prev_x = 0.0f, prev_y = 0.0f, prev_z = 0.0f;
    static uint32_t last_tick = 0;

    // Subtract bias
    float gx = gyro->x - gyro_bias.x;
    float gy = gyro->y - gyro_bias.y;
    float gz = gyro->z - gyro_bias.z;

    // Compute dt
    uint32_t now = HAL_GetTick();
    float dt = (last_tick == 0) ? 0.0f : (now - last_tick) / 1000.0f;
    last_tick = now;

    // Integrate to get angles
    rpy.x = prev_x + gx * dt; 
    rpy.y = prev_y + gy * dt;
    rpy.z = prev_z + gz * dt;

    prev_x = rpy.x;
    prev_y = rpy.y;
    prev_z = rpy.z;

    return rpy;
}


