#include "in_flash_manage.h"
#include "stmflash.h"



void iap_start(void)
{
	uint8_t buf[12]={0xaa,0xaa,0x55,0x55};

	STMFLASH_Erase(BOOTLOADER_PARA_ADDR);
	STMFLASH_Write((uint32_t )BOOTLOADER_PARA_ADDR,(uint32_t *)buf,3);
	
	//delay_ms(100);
	
	NVIC_SystemReset();
}



//xxxxxxxxxxxxxxxxxxxxxxxxxxx

STU_HIS StuHis;


void AnsyHis(void)
{

  
}
void ResetHis(void)
{
  
}

void InitHis(void)
{
	
 


}




