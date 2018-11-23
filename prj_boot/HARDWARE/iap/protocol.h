#ifndef protocol_data_h___
#define  protocol_data_h___

#include "sys.h"
#include "byte_fifo.h"


#define UPGRADE_PACKET_SIZE  256

#define ACK									0x81
#define SET_NETWORK_PARAMETERS				0x82
#define READ_NETWORK_PARAMETERS				0x83
#define SET_FACTORY							0x84
#define UPFILE_START    					0x85
#define UPFILE_DATA							0x86
#define UPFILE_PAR							0x88
#define INFORMATION_PAR						0x89
//information
#define MCU_ACK								0x01
#define MCU_NETWORK_PARAMETERS				0x03
#define MCU_ASK_PACKET						0x06
#define MCU_UPFILE_RESULT					0x07
#define MCU_ASK_UPFILE_PAR					0x08
#define MCU_INFORMATION_PAR					0x09


extern uint32_t g_ticktime_50ms;
void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdata,uint16_t datalen);
/*
** 串口升级函数
*/
void task_uart_upfile(void);
void WriteFlgBootSys(bool disversion);






bool IsNeedUpdata(void);
void CleanUpflg(void);
void WriteUpflg(uint8_t* datain,uint8_t len);



#endif


