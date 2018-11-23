#include "main.h"
#include "in_flash_manage.h"
#include "rtt_printf.h"
#include "rtc.h"


extern uint8_t app_main_lv;
#define TAG 		"[MAIN:%d]"
#define LOG(fmt,args...) do{if (app_main_lv) printf(TAG fmt, __LINE__, ##args);}while(0)

uint8_t app_main_lv = 1;
static void app_main_print_control(int argc, char **argv)
{
	if (0 == strcmp(argv[1], "off"))
		app_main_lv = 0;
	else if (0 == strcmp(argv[1], "on"))
		app_main_lv = 1;
}
DECLAREE_CMD_FUNC("MAIN", app_main_print_control);

/*--------------- Tasks Priority -------------*/
#define MAIN_TASK_PRIO              ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO              ( tskIDLE_PRIORITY + 4 )
#define APP_TASK_PRIO               ( tskIDLE_PRIORITY + 2 )
#define MQTT_TASK_PRIO              ( tskIDLE_PRIORITY +4 )
#define ETH_LINK_TASK_PRIORITY      ( tskIDLE_PRIORITY + 3 )

#define MQTT_STK    				(configMINIMAL_STACK_SIZE*6)
#define DHCP_STK    				(configMINIMAL_STACK_SIZE*2)
#define APP_STK    					(configMINIMAL_STACK_SIZE*8)

const uint8_t version[12] __attribute__ ((at(0x8020000+1024)))={VERSION_H,VERSION_L,VERSION_T,'2','d','a','n',HARDWARE_VERSION_H,HARDWARE_VERSION_L,HARDWARE_VERSION_T};

void Main_task(void * pvParameters);

int main(void)
{

    SCB->VTOR = FLASH_BASE | 0x20000; 
//      SCB->VTOR = FLASH_BASE | 0x00000;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    uart_printf_init(921600);//printf
//    Uart2Init(115200);
    //配置串口1
    Uart_485Init(115200);//485
    LOG("system start\r\n");
    /* Init task */
    xTaskCreate(Main_task, (int8_t *) "Main", configMINIMAL_STACK_SIZE *4, NULL,MAIN_TASK_PRIO, NULL);

    LOG("vTaskStartScheduler be call\r\n");
    
    /* 启动调度器，任任务开始执行*/
    vTaskStartScheduler();

    /* We should never get here as control is now taken by the scheduler */
    /*如果一切正常，main函数不知执行到这里，如果执行到这里很可能是内存不足导致空闲任务无法创建*/
    for( ;; );
}

void Main_task(void * pvParameters)
{
    InitHis();
    stm32f4_rtc_Init();
//    ResetHis();
    IWDG_Init(6,1250); //溢出时间为10s	
    xTaskCreate(task_app, (int8_t *) "app", APP_STK, NULL, APP_TASK_PRIO, NULL);
    while(RNG_Init())	 		//初始化随机数发生器
    {
		delay_ms(200);
		LOG("RNG Error! Trying...\r\n");
    }
	LOG("rand number:0x%08x\r\n",RNG_Get_RandomNum());
    LED_Init();
    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    LOG(">>configint eth......\r\n");
    ETH_BSP_Config();
    if((EthStatus&(0x01)) == 0)
    {
        LOG(">>wire is not insert\r\n");
    }
    else
    {
        LOG(">>config ..OK\r\n");
    }
    LwIP_Init();
    LOG(">>lwip init.OK start dhcp\r\n");
    xTaskCreate(LwIP_DHCP_task, (int8_t *) "DHCP", DHCP_STK, NULL,DHCP_TASK_PRIO, NULL);

    //http_server_socket_init();

    xTaskCreate(mqtt_thread, (int8_t *) "MQTT", MQTT_STK, NULL, MQTT_TASK_PRIO, NULL);
    Beep();
    LOG(">>sys init OK\r\n");

    for( ;; )
    {
        vTaskDelete(NULL);
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    LOG("assert_failed:%s-%d\r\n",file,line);
    delay_ms1(100);
    /* Infinite loop */
//    while (1)
//    {}
}
#endif


