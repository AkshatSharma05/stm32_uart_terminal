#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

/* Private variables */
static uint8_t rx_char;
static char rx_buffer[RX_BUF_SIZE];
static uint8_t rx_index = 0;
static uint8_t cmd_ready = 0;
extern uint8_t imu_stream_enabled;

/* Redirect printf */
int __io_putchar(int ch){
  HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
  return ch;
}

/* Init */
void UART_Init(void){
  HAL_UART_Receive_IT(&huart2, &rx_char, 1);
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\033[2J");   // Clear screen
  printf("\033[H");    // Move cursor to top-left (home)
  printf("<<< Hominum Debugging Interface v0.0.1 >>>\r\n");
  printf("Type help to get started! \r\n");
  printf("~ ");
}

/* Main loop handler */
void UART_Process(void){
  if(cmd_ready){
    cmd_ready = 0;
    processCommands(rx_buffer);
    printf("~ ");
  }
}

/* Interrupt callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart->Instance == USART2){

    if(imu_stream_enabled){
      if(rx_char == 'q'){
        imu_stream_enabled = 0;
        printf("\r\nExiting IMU mode\r\n~ ");
      }

      HAL_UART_Receive_IT(&huart2, &rx_char, 1);
      return;
    }

    if(rx_char == '\r' || rx_char == '\n'){
      rx_buffer[rx_index] = '\0';
      rx_index = 0;
      cmd_ready = 1;
      printf("\r\n");
    } 
    else{ 
      if(rx_index < RX_BUF_SIZE - 1){
        rx_buffer[rx_index++] = rx_char;
        HAL_UART_Transmit(&huart2, &rx_char, 1, HAL_MAX_DELAY);
      }
    }

    HAL_UART_Receive_IT(&huart2, &rx_char, 1);
  }
}

/* Command parser */
void processCommands(char *cmd){
  char *token = strtok(cmd, " ");
  if(token == NULL) return;

  if(strcmp(token, "help") == 0){
    printf(
        "\r\n"
        "----------------------\r\n"
        " Command Interface\r\n"
        "----------------------\r\n"
        " help      -> show commands\r\n"
        " set x val -> set value of param x to val\r\n"
        " get x     -> read value\r\n"
        " save      -> save values to memory\r\n"
    );

  } else if (strcmp(token, "get") == 0){
    printf("Print all tokens!\r\n");

  } else if (strcmp(token, "set") == 0){
    char *param = strtok(NULL, " ");
    char *value = strtok(NULL, " ");

    if(param && value){
      int val = atoi(value);
      printf("setting \"%s\" -> %d\r\n", param, val);
    }

  } else if (strcmp(token, "imu") == 0){
    printf("Starting IMU stream... (press 'q' to exit)\r\n");
    imu_stream_enabled = 1;

  }else {
    printf("Invalid Command.\r\n");
  }
}