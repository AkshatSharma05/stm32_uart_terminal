#ifndef UART_H
#define UART_H

#include "stm32f1xx_hal.h"

#define RX_BUF_SIZE 64

void UART_Init(void);
void UART_Process(void);
void processCommands(char *cmd);

#endif