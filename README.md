# STM32 UART Terminal & IMU Interface

## Overview
This project focuses on building a simple and interactive debugging system using an STM32 microcontroller. An MPU6050 IMU is interfaced over I2C, and sensor data is processed using a complementary filter to obtain stable orientation estimates.

A UART-based terminal is developed to allow real-time communication with the system. Users can view both raw and filtered sensor data and adjust parameters dynamically through terminal commands.

The system is designed to make debugging, monitoring, and tuning embedded applications easier, and can be integrated into larger robotics projects like MR_ROBOT.
