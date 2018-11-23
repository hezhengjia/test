#include "rng.h"
#include "stm32f4xx_rcc.h"
#include "uart.h"
//初始化RNG
uint8_t RNG_Init(void)
{
    uint16_t retry=0;

    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);//开启RNG时钟,来自PLL48CLK

    RNG_Cmd(ENABLE);	//使能RNG

    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET&&retry<1000)	//等待随机数就绪
    {
        retry++;
        delay_ms(1);
    }
    if(retry>=1000)return 1;//随机数产生器工作不正常
    return 0;
}
//得到随机数
//返回值:获取到的随机数
uint32_t RNG_Get_RandomNum(void)
{
    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);	//等待随机数就绪
    return RNG_GetRandomNumber();
}

//生成[min,max]范围的随机数
int RNG_Get_RandomRange(int min,int max)
{
    return RNG_Get_RandomNum()%(max-min+1) +min;
}



