#include "uart.h"
#include "delay.h"


STU_BYTE_QUEUE uart_queue;
uint8_t uart_buf[MAX_UART_BUF];


/* TXD  PD8
** RXD  PC11
** RTS  PB14
*/

/************************************************************************
** 函数名称: Uart_485Init								
** 函数功能: 串口1初始化函数	
** 入口参数: u32 bound:波特率
** 出口参数: 无
** 备    注: 
************************************************************************/
void Uart_485Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART1时钟

	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_USART3); //GPIOA10复用为USART1

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PA9，PA10

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化PA9，PA10



	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口1

	USART_Cmd(USART3, ENABLE);  //使能串口1 


	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); //初始化PG8

	//        	RS485_TX_EN=1;				//默认为接收模式	
	RS485_TX_EN=0;				//默认为接收模式
	queue_byte_init(&uart_queue,uart_buf,MAX_UART_BUF);
  
        
}
void USART3_IRQHandler(void)                	//串口1中断服务程序
{


	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//uint8_t res=USART_ReceiveData(USART3);
		queue_byte_in(&uart_queue,USART_ReceiveData(USART3));
		
  } 

} 
void uart_sent_bytes(uint8_t *p_data,uint16_t len)
{
	uint16_t i;
	uint32_t count = 0;
	RS485_TX_EN=1;			//???????
	delay_ms(5);
	for (i=0; i<len; i++)
	{
		count = 0;
		while( ((USART3->SR&0X40)==0) && ((count++)<0xfff) ){}
		//while( (UART4->SR&0X40)==0);
		USART3->DR = p_data[i];
	}
	delay_ms(5);
	RS485_TX_EN=0;				//默认为接收模式
}































