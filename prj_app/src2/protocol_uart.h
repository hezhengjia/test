#ifndef protocol_data_h___
#define  protocol_data_h___

#include "sys.h"
#include "byte_fifo.h"


#define UART_FRAM_LEN  2048

#define ACK                                 0x81
#define SET_NETWORK_PARAMETERS              0x82
#define READ_NETWORK_PARAMETERS             0x83
#define SET_FACTORY                         0x84
#define UPFILE_START                        0x85
#define UPFILE_DATA                         0x86
#define UPFILE_PAR                          0x88
#define INFORMATION_PAR                     0x89
//information
#define MCU_ACK                             0x01
#define MCU_NETWORK_PARAMETERS              0x03
#define MCU_ASK_PACKET                      0x06
#define MCU_UPFILE_RESULT                   0x07
#define MCU_ASK_UPFILE_PAR                  0x08
#define MCU_INFORMATION_PAR                 0x09


#define PIGS_ACK  0x41
#define PIGS_SENSORS  0x42
#define PIGS_FOODS_RESULT 0x43
#define PIGS_NFO 0x44

#define UPGRADE_PACKET_SIZE  256

typedef void (*dataout_fun)(uint16_t cmd,uint8_t*pdatas,uint16_t datalen); // Flash_ReadData

extern uint32_t g_ticktime_50ms;


/*
** è§£åŒ…æ•°æ®
** datain,ä¸²å£æ•°æ®å…¥å£
** call_util_get_dataæ¥æ”¶åˆ°æ•°æ®çš„å›è°ƒ
** return,trueæ¥æ”¶ä¸€å¸§æ•°æ®
*/
bool uart_unpackage_data(uint8_t datain,dataout_fun call_util_get_data);
/*
** å‡½æ•°è¯´æ˜ï¼šæ‰“åŒ…å‘é€
** cmd:ä¼ å…¥çš„å‘½ä»¤
** p_in_datas,datalen æ•°æ®éƒ¨åˆ†çš„æŒ‡é’ˆå’Œé•¿åº¦
** p_out_data:æ‰“åŒ…å®Œå…¨çš„æ•°æ®
**  è¿”å›å€¼ä¸ºæ‰“åŒ…å·æ•°æ®çš„é•¿åº¦
*/
uint16_t uart_package_data(uint16_t cmd,uint8_t* p_in_datas,uint16_t datalen,uint8_t*p_out_datas);
void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdata,uint16_t datalen);



#define  uart_sent_bytes RS485_Send_Data


/*
** ä¸²å£å‡çº§å‡½æ•°
*/
void task_uart_upfile(void);
void WriteFlgBootSys(void);






bool IsNeedUpdata(void);
void CleanUpflg(void);
void WriteUpflg(uint8_t* datain,uint8_t len);













/*
Íø¹Ø·¢ËÍack 0x31

*/
void send_ack2pigs(uint16_t cmd,uint16_t answer_cmd,uint8_t code,uint8_t key);

/*
Íø¹Ø·¢ËÍ½øÁÏÖ¸Áî0x32
Íø¹Ø·¢ËÍÎ¹ÁÏÖ¸Áî0x33

*/
void send_food2pigs(uint16_t cmd,uint16_t foods,uint8_t key);//;

/*
Íø¹Ø·¢ËÍ¿ª¹ØµÆÖ¸Áî0x34
Íø¹Ø·¢ËÍÆÁÄ»ÏÔÊ¾Ö¸Áî0x35
Íø¹Ø·¢ËÍÆÁÄ»ÏÔÊ¾Ö¸Áî0x3a
*/
void send_led_lcd2pigs(uint16_t cmd,uint8_t ledlcd,uint8_t key);


/*
Íø¹Ø·¢ËÍ³ÆÖØ´«¸ĞÆ÷¹éÁãÖ¸Áî0x36
Íø¹Ø·¢ËÍÇëÇóËÇÎ¹Æ÷´«¸ĞÆ÷×´Ì¬Ö¸Áî0x37
.Íø¹Ø·¢ËÍÇëÇóËÇÎ¹Æ÷Éè±¸ĞÅÏ¢Ö¸Áî0x39


*/

void send_cmd2pigs(uint16_t cmd,uint8_t key);

/*
Íø¹Ø·¢ËÍÇëÇóËÇÎ¹Æ÷Êµ¼ÊµÄ½øÎ¹ÖØÁ¿Ö¸Áî0x38
0x00:ÇëÇóËÇÎ¹Æ÷Êµ¼Ê½øÁÏÖØÁ¿£¬
0x01:ÇëÇóËÇÎ¹Æ÷Êµ¼ÊÎ¹ÁÏÖØÁ¿

*/
void send_cmd_read_foods2pigs(uint16_t cmd,uint8_t key,uint8_t feed_out);




#endif


