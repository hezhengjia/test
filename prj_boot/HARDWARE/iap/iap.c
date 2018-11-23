
//#include "delay.h"
//#include "usart.h"
//#include "stmflash.h"
#include "iap.h" 
/*

iapfun jump2app; 
u32 iapbuf[512]; 	//2K字节缓存  
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//偏移4个字节
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//偏移2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}		 

*/

void jump_to_app(uint32_t app_address)
{
    typedef void (*_func)(void);

    __disable_irq();

    /* MCU peripherals re-initial. */
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStruct.GPIO_Pin &= ~(GPIO_Pin_13 | GPIO_Pin_14); /* SWDIO/SWCLK */
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
        GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_Init(GPIOE, &GPIO_InitStruct);
        GPIO_Init(GPIOF, &GPIO_InitStruct);
        GPIO_Init(GPIOG, &GPIO_InitStruct);
        GPIO_Init(GPIOH, &GPIO_InitStruct);
        GPIO_Init(GPIOI, &GPIO_InitStruct);

        /* reset systick */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

        /* disable all peripherals clock. */
        RCC->AHB1ENR = (1<<20); /* 20: F4 CCMDAT ARAMEN. */
        RCC->AHB2ENR = 0;
        RCC->AHB3ENR = 0;
        RCC->APB1ENR = 0;
        RCC->APB2ENR = 0;

        /* Switch to default cpu clock. */
        RCC->CFGR = 0;
    } /* MCU peripherals re-initial. */

    /* Disable MPU */
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    /* disable and clean up all interrupts. */
    {
        int i;

        for(i = 0; i < 8; i++)
        {
            /* disable interrupts. */
            NVIC->ICER[i] = 0xFFFFFFFF;

            /* clean up interrupts flags. */
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
    }

    /* Set new vector table pointer */
    SCB->VTOR = app_address;

    /* reset register values */
    __set_BASEPRI(0);
    __set_FAULTMASK(0);

    /* set up MSP and switch to it */
    __set_MSP(*(uint32_t*)app_address);
    __set_PSP(*(uint32_t*)app_address);
    __set_CONTROL(0);

    /* ensure what we have done could take effect */
    __ISB();

    __disable_irq();

    /* never return */
    ((_func)(*(uint32_t*)(app_address + 4)))();
}













