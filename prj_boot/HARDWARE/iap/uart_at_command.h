#ifndef _UART_AT_COMMAND_H_
#define _UART_AT_COMMAND_H_


#include "sys.h"
#include "byte_fifo.h"


extern STU_BYTE_QUEUE uartQueue;


void process_at_command(uint8_t* pdatain,uint8_t d_length );
void task_process_uart_data(void);













#endif









