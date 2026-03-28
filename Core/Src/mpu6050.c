#include "mpu6050.h"

extern I2C_HandleTypeDef hi2c1;

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

    status =  HAL_I2C_Mem_Write(&hi2c1, (uint16_t)MPU6050_ADDR, GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) return status;

    return status;
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