#include "usart.h"


u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
STU_BYTE_QUEUE uart_queue;



#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	uint32_t count = 0;
	count = 0;
	while( ((UART4->SR&0X40)==0) && ((count++)<0xfff) ){}
    UART4->DR = (u8) ch;
    return ch;
}

//TX-PA0 RX-PA1
void uart_init(u32 bound) 
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 							
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);							

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4); 									
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4); 									


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 									
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;											
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;										
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 											
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 											
	GPIO_Init(GPIOA,&GPIO_InitStructure); 													

	USART_InitStructure.USART_BaudRate = bound;												
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;								
	USART_InitStructure.USART_StopBits = USART_StopBits_1;									
	USART_InitStructure.USART_Parity = USART_Parity_No;										
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;			
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;							
	USART_Init(UART4, &USART_InitStructure); 												
	USART_Cmd(UART4, ENABLE); 							 									
	USART_ClearFlag(UART4, USART_FLAG_TC);

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);											

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;										
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;									
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;										
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;											
	NVIC_Init(&NVIC_InitStructure);	

	queue_byte_init(&uart_queue,USART_RX_BUF,USART_REC_LEN);
}
void uart_sent_bytes(uint8_t *p_data,uint16_t len)
{
	uint16_t i;
	uint32_t count = 0;
	for (i=0; i<len; i++)
	{
		count = 0;
		while( ((UART4->SR&0X40)==0) && ((count++)<0xfff) ){}
		//while( (UART4->SR&0X40)==0);
		UART4->DR = p_data[i];
	}
}
void UART4_IRQHandler(void)                	//串口1中断服务程序
{

    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  					//接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
		USART_ClearITPendingBit(UART4,USART_IT_RXNE);
		queue_byte_in(&uart_queue,USART_ReceiveData(UART4));
    }

}






