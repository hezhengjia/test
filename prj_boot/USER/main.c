#include "sys.h"
#include "delay.h"
#include "uart.h"
#include "stmflash.h"
#include "iap.h"
#include "in_flash_manage.h"
#include "protocol.h"
#include "timer.h"
#ifdef EX_FLASH
#include "w25qxx.h"
#endif

#define UPGRADE_TIMES (1000/50*60*2)
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2

#ifdef EX_FLASH
	W25QXX_Init();
#endif

#ifdef BOOT_LOADER
	if (IsNeedUpdata())
#else
	while(1)
#endif
	{
		Uart_485Init(115200);
		InitHis();
		TIM3_Int_Init(500 - 1, 8400 - 1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����500��Ϊ50ms
		while(1)
		{
			task_uart_upfile();
		}
	}
	jump_to_app(APP_ADDR);
}






