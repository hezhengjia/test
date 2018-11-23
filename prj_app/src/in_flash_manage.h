#ifndef __IN_SLASH_MANAGE_H22_
#define __IN_SLASH_MANAGE_H22_
//#include "sys.h"
#include "stdbool.h"
#include "stdint.h"
/*================================================================
flash:
start				end				size				left			item
0x8000000			0x8004000		16k					1008k		bootloader
0x8004000			0x8008000		16k					992k			bootloader_flag
0x8008000			0x800c000		16k					976k			His_data
0x800c000			0x8010000		16k					960k			rsv2
0x8010000			0x8020000		64k					896			rsv3


0x8020000			0x8040000		128k				256k			app
0x8040000			0x8060000		128k				128k			bvk_app
0x8060000			0x8080000		128k				0k			rsv4



================================================================*/


#define BOOTLOADER_ADDR (0x8000000)
#define BOOTLOADER_SIZE (16*1024)



#define BOOTLOADER_PARA_ADDR (BOOTLOADER_ADDR+BOOTLOADER_SIZE)
#define BOOTLOADER_PARA_SIZE (16*1024)

#define HIS_ADDR (BOOTLOADER_PARA_ADDR+BOOTLOADER_PARA_SIZE)
#define HIS_SIZE (16*1024)

#define RSV2_ADDR (HIS_ADDR+HIS_SIZE)
#define RSV2_SIZE (16*1024)

#define RSV3_ADDR (RSV2_ADDR+RSV2_SIZE)
#define RSV3_SIZE (64*1024)

#define APP_ADDR (RSV3_ADDR+RSV3_SIZE)
#define APP_SIZE (128*1024)

#define APP_BVK_ADDR (APP_ADDR+APP_SIZE)   
#define APP_BVK_SIZE  (128*1024)


#define RSV4_ADDR (APP_BVK_ADDR+APP_BVK_SIZE)
#define RSV4_SIZE (128*1024)


#define MAX_IP_LEN 50
#define MAX_GATWAY_ID  20
#define EEPROM_SIZE 500

#define DEVICE_REGISTERED  0xaa


#pragma pack(1)
typedef struct
{
	uint8_t remote_ip[MAX_IP_LEN];
	uint32_t remote_port;
	uint8_t net_connect_state;//0TCP  1SSL
	uint8_t username[33];
	uint8_t password[33];
	uint8_t keepAliveInterval;//心跳包时间
	uint8_t qos;
	uint8_t cleansession;
	uint8_t clientID[30];
	uint8_t registered;

	bool dhcpstatus;
	
	uint8_t ip[4];
	uint8_t netmask[4];
	uint8_t gateway[4];
	uint8_t dns1[4];
	uint8_t dns2[4];	
	uint8_t mac[6];
    uint8_t gatewayid[MAX_GATWAY_ID];
    uint8_t registerted;

    uint32_t heart_rate_ms;
	
	
} STU_PARA;
typedef struct
{
	STU_PARA StuPara;
	uint8_t empty[EEPROM_SIZE-sizeof(STU_PARA)];
	
} STU_HIS;
#pragma pack()
extern STU_HIS StuHis;


#define HIS_HEAD 0xaa
#define HIS_BUF_LEN ((sizeof(STU_HIS)+5+3)/4)
void ResetHis(void);
void InitHis(void);
void AnsyHis(void);



#define AN_RESET  1
#define AN_WRITE  2

void task_ansy_his(void);

void ask_ansy(uint8_t action);



#endif




