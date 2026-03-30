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
extern float alpha;


//as i cant use atof -> uses too much memory
float parse_float(const char *s){
    float result = 0.0f;
    float frac = 0.1f;
    int sign = 1;

    if(*s == '-'){ sign = -1; s++; }

    while(*s && *s != '.'){
        result = result * 10 + (*s - '0');  // *s-'0' -> '0' is 48 -> convert str to int
        s++;
    }

    if(*s == '.'){
        s++;
        while(*s){
            result += (*s - '0') * frac;
            frac *= 0.1f;
            s++;
        }
    }

    return result * sign;
}

void print_float(float val){
    int i = (int)val;
    int frac = (int)((val - i) * 1000);

    if(frac < 0) frac = -frac;

    printf("%d.%03d", i, frac);
}


int __io_putchar(int ch){
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


void UART_Init(void){
    HAL_UART_Receive_IT(&huart2, &rx_char, 1);
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\033[2J");   // clear screen
    printf("\033[H");    // cursor home
    printf("<<< Hominum Debugging Interface v0.0.1 >>>\r\n");
    printf("Type help to get started\r\n");
    printf("~ ");
}


void UART_Process(void){
    if(cmd_ready){
        cmd_ready = 0;
        processCommands(rx_buffer);
        printf("~ ");
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART2){

        if(imu_stream_enabled){
            if(rx_char == 'q'){
                imu_stream_enabled = 0;
                printf("\r\nExit IMU\r\n~ ");
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


void processCommands(char *cmd){
    char *token = strtok(cmd, " ");
    if(!token) return;

    if(strcmp(token, "help") == 0){
        printf(
            "\r\nCommands:\r\n"
            " help\r\n"
            " get alpha\r\n"
            " set alpha <val>\r\n"
            " imu\r\n"
        );
    }

    else if(strcmp(token, "get") == 0){
        char *param = strtok(NULL, " ");

        if(param && strcmp(param, "alpha") == 0){
            printf("alpha=");
            print_float(alpha);
            printf("\r\n");
        } else {
            printf("unknown param\r\n");
        }
    }

    else if(strcmp(token, "set") == 0){
        char *param = strtok(NULL, " ");
        char *value = strtok(NULL, " ");

        if(param && value && strcmp(param, "alpha") == 0){
            alpha = parse_float(value);

            printf("alpha=");
            print_float(alpha);
            printf("\r\n");
        } else {
            printf("usage: set alpha <val>\r\n");
        }
    }

    else if(strcmp(token, "imu") == 0){
        printf("IMU stream (q to quit)\r\n");
        imu_stream_enabled = 1;
    }

    else {
        printf("Invalid command\r\n");
    }
}