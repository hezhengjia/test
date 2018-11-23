#include "uart.h"












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

#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
  uint8_t res=USART_ReceiveData(USART3);
    queue_byte_in(&uart_queue,USART_ReceiveData(USART3));
		
			 
  } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
void RS485_Send_Data(uint8_t *buf,uint16_t len)
{
	uint16_t t;
	RS485_TX_EN=1;			//设置为发送模式
  	for(t=0;t<len;t++)		//循环发送数据
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //等待发送结束		
    USART_SendData(USART3,buf[t]); //发送数据
	}	 
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //等待发送结束		  
	RS485_TX_EN=0;				//设置为接收模式	
}

//void Uart2Init(u32 bound)
//{
//   //GPIO端口设置
//  GPIO_InitTypeDef GPIO_InitStructure;
//	USART_InitTypeDef USART_InitStructure;

//	
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART1时钟
// 
//	//串口1对应引脚复用映射
//	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART1); //GPIOA9复用为USART1
//	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART1); //GPIOA10复用为USART1
//	
//	//USART1端口配置
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA9与GPIOA10
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
//	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

//   //USART1 初始化设置
//	USART_InitStructure.USART_BaudRate = bound;//波特率设置
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
//	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
//  USART_Init(USART2, &USART_InitStructure); //初始化串口1
//	
//  USART_Cmd(USART2, ENABLE);  //使能串口1 
//	
//	
//	NVIC_InitTypeDef NVIC_InitStructure;
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启相关中断

//	//Usart1 NVIC 配置
//  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

//		//PG8推挽输出，485模式控制  
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //GPIOG8
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
//	GPIO_Init(GPIOG,&GPIO_InitStructure); //初始化PG8
//    
//    	RS485_TX_EN=0;				//默认为接收模式	
//        queue_byte_init(&uart_queue,uart_buf,MAX_UART_BUF);
//        
//}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
//void RS485_Send_Data(uint8_t *buf,uint16_t len)
//{
//	uint16_t t;
//	RS485_TX_EN=1;			//设置为发送模式
//  	for(t=0;t<len;t++)		//循环发送数据
//	{
//	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
//    USART_SendData(USART2,buf[t]); //发送数据
//	}	 
//	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		  
//	RS485_TX_EN=0;				//设置为接收模式	
//}

//void USART2_IRQHandler(void)                	//串口1中断服务程序
//{

//#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
//	{

//    queue_byte_in(&uart_queue,USART_ReceiveData(USART2));
//		
//			 
//  } 
//#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntExit();  											 
//#endif
//} 





/************************************************************************
** 函数名称: fputc									
** 函数功能: 重定向printf到串口
** 入口参数: 无	
** 出口参数: 无													
************************************************************************/
//int fputc(int ch, FILE *f)
//{
//  //往串口上发送一个字节数据
//  USART_SendData(USART1, (uint8_t) ch);

//  //循环等待发送完毕
//  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
//  {}

//  return ch;
//}



//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
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
    while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
    USART1->DR = (u8) ch;
    return ch;
}
#endif


//初始化IO 串口1
//bound:波特率
void uart_printf_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟

    //串口1对应引脚复用映射
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10

    //USART1端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

    //USART1 初始化设置
    USART_InitStructure.USART_BaudRate = bound;//波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART1, &USART_InitStructure); //初始化串口1

    USART_Cmd(USART1, ENABLE);  //使能串口1

    //USART_ClearFlag(USART1, USART_FLAG_TC);


    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、



}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
    u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
    OSIntEnter();
#endif
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
     if(Res>10)
     {
     printf("1\r\n");
     }

    }
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
    OSIntExit();
#endif
}



































