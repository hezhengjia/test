#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "byte_fifo.h"




#define USART_REC_LEN  			200  	//定义最大接收字节数 200



extern STU_BYTE_QUEUE uart_queue;



void uart_init(u32 bound);
void uart_sent_bytes(uint8_t *p_data,uint16_t len);
#endif


