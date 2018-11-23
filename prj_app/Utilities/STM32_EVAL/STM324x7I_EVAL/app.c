
#include "app.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_i2c.h"
#include "iwdg.h"
#include "byte_fifo.h"
#include "uart.h"
#include "in_flash_manage.h"
#include "rng.h"
#include "cJSON.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "protocol_uart.h"
#include "fifo_bytes.h"
#include "bytes_fifo_malloc.h"
#include "mqttclient.h"
#include "rtc.h"
#include "application.h"

#define SUPPORT_LIST							20

/***************add by zhengjia for temp*****************/

extern uint8_t app_log_lv;
#define TAG 		"[APP:%d]"
#define LOG(fmt,args...) do{if (app_log_lv) printf(TAG fmt, __LINE__, ##args);}while(0)

uint8_t app_log_lv = 1;
static void app_print_control(int argc, char **argv)
{
	if (0 == strcmp(argv[1], "off"))
		app_log_lv = 0;
	else if (0 == strcmp(argv[1], "on"))
		app_log_lv = 1;
}
DECLAREE_CMD_FUNC("APP", app_print_control);
//====================================================================================


volatile uint8_t open_relay=0;
void delay_us1(u32 nus)
{
    unsigned short i=0;
    while(nus--)
    {
        i=10;
        while(i--) ;
    }
}

void delay_ms1(u16 nms)
{
    unsigned short i=0;
    while(nms--)
    {
        i=12000;
        while(i--) ;
    }
}


void LED_Init(void)
{


//led bin relay

//    GPIO_InitTypeDef  GPIO_InitStructure;

//    /* Enable the GPIO_LED Clock */
//    RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);


//    /* Configure the GPIO_LED pin */
//    GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);



    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOF时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOF时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOF时钟

    //GPIOF9,F10初始化设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO

    GPIO_SetBits(GPIOF,GPIO_Pin_13 | GPIO_Pin_14);//GPIOF9,F10设置高，灯灭

    //GPIOF9,F10初始化设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO

    GPIO_SetBits(GPIOC,GPIO_Pin_6 | GPIO_Pin_7);//GPIOF9,F10设置高，灯灭


    //GPIOF9,F10初始化设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO

    GPIO_SetBits(GPIOE,GPIO_Pin_9 );//GPIOF9,F10设置高，灯灭


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;//LED0和LED1对应IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO

    GPIO_SetBits(GPIOD,GPIO_Pin_15 );//GPIOF9,F10设置高，灯灭


    LED1_OFF();
    LED2_OFF();
    LED3_OFF();
    LED4_OFF();
    BEEN_OFF();
    RELAY_OFF();

}

void Delay(uint32_t nCount)
{
    vTaskDelay(nCount);
}


void Beep(void)
{
    BEEN_ON();
    vTaskDelay(500/portTICK_RATE_MS);
    BEEN_OFF();
}

STU_485_REV Stu485rev;


typedef struct
{
    int cmd;
    my_uuid_t msg_uuid;		//save end charactor
    uint8_t feed_id;
    uint32_t escape_sec;
} STU_WAIT_CMD_DATA;

/******************for item analyze**************************************/

// 删除长度为len的数组dat中索引为idx的元素。
void remove_array_index(uint8_t *dat, uint16_t *len, int idx)
{
    (*len)--;
    if (idx < 0 || idx >= *len)
        return;
    for (int i = idx; i < *len; i++)
        dat[i] = dat[i+1];
}


typedef struct
{
    STU_WAIT_CMD_DATA stu_wait_cmd_data[SUPPORT_LIST];
    uint8_t lists;

} STU_WAIT_CMD;

STU_WAIT_CMD stu_wait_cmd=
{
    .lists=0,
};


bool find_in_wait_list(STU_WAIT_CMD_DATA stu_data)
{
	uint8_t i;
	
	for(i=0;i<stu_wait_cmd.lists;i++)
	{
		if(0==memcmp(&stu_wait_cmd.stu_wait_cmd_data[i],&stu_data,sizeof(&stu_data)))
		return true;
	}
	return false;

}
void add_id2wait_list(int cmd,my_uuid_t *p_msg_uuid,uint8_t feed_id)
{
    STU_WAIT_CMD_DATA stu_data;

//	ASSERT(msg_uuid[UUID_STRING_LENGTH] == 0, 0);

    stu_data.cmd = cmd;
	stu_data.msg_uuid = *p_msg_uuid;
    stu_data.feed_id = feed_id;
    stu_data.escape_sec=0;

	if(false == find_in_wait_list(stu_data))
	{
		stu_wait_cmd.stu_wait_cmd_data[stu_wait_cmd.lists]=stu_data;
		stu_wait_cmd.lists++;
		stu_wait_cmd.lists%=SUPPORT_LIST;
	}
}

static void delete_id_from_wait_list(uint8_t index)
{
    uint8_t i;
    stu_wait_cmd.lists--;
    if ( index >= stu_wait_cmd.lists)
        return;
    for ( i = index; i < stu_wait_cmd.lists; i++)
    {
        stu_wait_cmd.stu_wait_cmd_data[i] = stu_wait_cmd.stu_wait_cmd_data[i+1];
    }
}

static void check_wiat_list_result(void)
{
    int16_t tmpi16;
    uint16_t tmp16;

    uint8_t i,j;
    STU_WAIT_CMD_DATA stu_data;

    if(0==stu_wait_cmd.lists)  return;

    for(i=0; i<stu_wait_cmd.lists; i++)
    {
        stu_wait_cmd.stu_wait_cmd_data[i].escape_sec++;
        stu_data=stu_wait_cmd.stu_wait_cmd_data[i];
        prepare_wait_485_cmd(stu_data.feed_id);

        for(j=0; j<2; j++)
        {
            send_cmd_read_foods2pigs(GATE_RD_RESULT38,stu_data.feed_id,(stu_data.cmd==CMD_FOOD_OUT2002)?1:0);
            if(true == wait_485_cmd(PIGS_FOODS_RESULT,WAIT_PIG_TIME,stu_data.feed_id))
            {
                if(Stu485rev.data[19]==0 || (Stu485rev.data[19]>=2&&Stu485rev.data[19]<=5))
                {
                    if(Stu485rev.data[19]!=0 && Stu485rev.data[19]!=5)
                    {
                        cmd_c2s_asynchronous_to_net(stu_data.cmd,&stu_data.msg_uuid,stu_data.feed_id,0,0,DEVICE_NO_FOOD+Stu485rev.data[19]-2);
                        delete_id_from_wait_list(i);
                    }
                    else
                    {
                        tmp16=Stu485rev.data[17];
                        tmp16=(tmp16<<8)|Stu485rev.data[18];
                        tmpi16=Stu485rev.data[20];
                        tmpi16=(tmpi16<<8)|Stu485rev.data[21];
                        cmd_c2s_asynchronous_to_net(stu_data.cmd,&stu_data.msg_uuid,stu_data.feed_id,tmp16,tmpi16,(Stu485rev.data[19]==5)?DEVICE_NO_FOOD+Stu485rev.data[19]-2:0);
                        delete_id_from_wait_list(i);
                    }
                }
                break;
            }
        }
        if(  stu_data.escape_sec >= ASK_FOOD_RESULT_TIMEOUT_SEC)//time out
        {
            LOG("result timeout %d\r\n",stu_data.feed_id);
//            send_result_to_net(stu_data.cmd,stu_data.msg_uuid,stu_data.feed_id,0,0,DEVICE_TIME_OUT);
            delete_id_from_wait_list(i);
            continue ;
        }
    }
}

void check_on_line(void)
{
    uint8_t i,cmd_wait,j;
//    uint8_t* list ;

    static uint8_t min=57;
    if(min++<=60) return ;

    min=0;

    for(i=0; i<MAX_DEVICE; i++)
    {
        prepare_wait_485_cmd(i);
        for(j=0; j<2; j++)
        {
            cmd_wait= uart_data2pigs_all_command(CMD_CTL_ONLINE2010,0,i);
            if(true==wait_485_cmd(cmd_wait,WAIT_PIG_TIME,i))
            {
                break;
            }
        }
    }
}

void prepare_wait_485_cmd(uint8_t addr)
{
    Stu485rev.addr=addr;
    Stu485rev.cmd=0;
    Stu485rev.data[0]=0;
}

bool wait_485_cmd(uint8_t cmd,uint16_t ms,uint8_t id)
{
    uint8_t data;
    uint32_t pubtick;

    pubtick=xTaskGetTickCount();
    while(1)
    {
        while ( true==queue_byte_out(&uart_queue,&data) )
        {
            uart_unpackage_data(data,task_protocol_dataout_fun);
        }
        if((xTaskGetTickCount() - pubtick) >(ms))
        {
            return false;
        }
        if(Stu485rev.cmd==cmd && Stu485rev.data[0]==id)
        {
            return true;
        }
        delay_ms(2);
    }
}

BEGIN_MESSAGE_MAP(receive_cmd)
	{CMD_FOOD_IN2000,		cmd_s2c_food_in_out},
	{CMD_FOOD_OUT2002,		cmd_s2c_food_in_out},
	{CMD_CTL_LED2004,		cmd_s2c_led_period},
	{CMD_ST_PERIOD2014,		cmd_s2c_led_period},
	{CMD_GET_TIME_2016,		cmd_s2c_get_timestamp},
	{CMD_CTL_UI2006,		cmd_s2c_06_12_in_out},	
	{CMD_CTL_ZERO2008,		cmd_s2c_06_12_in_out},
	{CMD_CTL_ONLINE2010,	cmd_s2c_06_12_in_out},
	{CMD_RD_SENSORS2012,	cmd_s2c_06_12_in_out},
END_MESSAGE_MAP()

	
void ana_net_datas(uint8_t *data,uint16_t len)
{
//    STU_DATAS StuDatas;
    cJSON *msg_uuid;
    cJSON *root;
    cJSON *msg_type;
	my_uuid_t my_uuid;

    root = cJSON_Parse((char*)data);
    if(!root)
    {
		ASSERT(0, 0);
        cmd_error_resoponse(0, NULL, 1, NULL);
        return;
    }

    msg_type = cJSON_GetObjectItem(root, "msg_type");
    if( (0==msg_type)||(msg_type->type!=cJSON_Number) )
    {
        LOG("msg_type err\r\n");
		cmd_error_resoponse(0, NULL, 2, NULL);
		cJSON_Delete(root);
        return;
    }

    msg_uuid = cJSON_GetObjectItem(root, "msg_uuid");
	
    if(!msg_uuid )
    {
		LOG(" msg_uuid err\r\n");
		cmd_error_resoponse(msg_type->valueint, NULL, 2, NULL);
        cJSON_Delete(root);
        return;
    }
	else
	{
		if (msg_uuid->type == cJSON_String)
		{
			ASSERT(strlen(msg_uuid->valuestring) + 1 == sizeof(my_uuid.a_uuid), 0);
			my_uuid.type = 1;
			memcpy(my_uuid.a_uuid, msg_uuid->valuestring, sizeof(my_uuid.a_uuid));
		}
		else
		{
			my_uuid.type = 0;
			my_uuid.value = msg_uuid->valueint;
		}
	}

	map_table_t key_main;
	key_main = get_map_table_function(receive_cmd, msg_type->valueint);	
	if (key_main != NULL)
	{
		(*key_main)(msg_type->valueint, (uint32_t)root, (uint32_t)&my_uuid);
	}
	else
	{
		ASSERT(0, msg_type->valueint);
	}
	
	if (root)
	{
		cJSON_Delete(root);
	}
	return;	
}

STU_DATAS stu_datas_tx[MAX_ELE_PACKETS];
STU_DATAS stu_datas_rx[MAX_ELE_PACKETS];

STU_QUEUE_MALLOC stu_queue_malloc_tx;
STU_QUEUE_MALLOC stu_queue_malloc_rx;

void task_app(void * pvParameters)
{
    uint8_t data;
    uint8_t *pdata;
    uint16_t len;
    uint8_t tick_sec=0;

    queue_malloc_init(&stu_queue_malloc_tx,stu_datas_tx,MAX_ELE_PACKETS);
    queue_malloc_init(&stu_queue_malloc_rx,stu_datas_rx,MAX_ELE_PACKETS);

    for( ;; )
    {
        IWDG_Feed();
        task_ansy_his();
        while ( true==queue_byte_out(&uart_queue,&data) )
        {
            uart_unpackage_data(data,task_protocol_dataout_fun);
        }
        if(0!=(pdata=queue_malloc_out(&stu_queue_malloc_rx,&len)))
        {
            ana_net_datas(pdata,len);
            queue_malloc_delete(&stu_queue_malloc_rx,1);
        }
        if(tick_sec++>=100)
        {
            check_on_line();
            LED1_TOG();
            if(net_connect_flg)
            {
                LED2_ON();
            }
            else
            {
                LED2_OFF();
            }
            check_wiat_list_result();
            tick_sec=0;
        }

        vTaskDelay(10/portTICK_RATE_MS);
    }
}



