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
** 解包数据
** datain,串口数据入口
** call_util_get_data接收到数据的回调
** return,true接收一帧数据
*/
bool uart_unpackage_data(uint8_t datain,dataout_fun call_util_get_data);
/*
** 函数说明：打包发送
** cmd:传入的命令
** p_in_datas,datalen 数据部分的指针和长度
** p_out_data:打包完全的数据
**  返回值为打包号数据的长度
*/
uint16_t uart_package_data(uint16_t cmd,uint8_t* p_in_datas,uint16_t datalen,uint8_t*p_out_datas);
void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdata,uint16_t datalen);



#define  uart_sent_bytes RS485_Send_Data


/*
** 串口升级函数
*/
void task_uart_upfile(void);
void WriteFlgBootSys(void);






bool IsNeedUpdata(void);
void CleanUpflg(void);
void WriteUpflg(uint8_t* datain,uint8_t len);













/*
���ط���ack 0x31

*/
void send_ack2pigs(uint16_t cmd,uint16_t answer_cmd,uint8_t code,uint8_t key);

/*
���ط��ͽ���ָ��0x32
���ط���ι��ָ��0x33

*/
void send_food2pigs(uint16_t cmd,uint16_t foods,uint8_t key);//;

/*
���ط��Ϳ��ص�ָ��0x34
���ط�����Ļ��ʾָ��0x35
���ط�����Ļ��ʾָ��0x3a
*/
void send_led_lcd2pigs(uint16_t cmd,uint8_t ledlcd,uint8_t key);


/*
���ط��ͳ��ش���������ָ��0x36
���ط���������ι��������״ָ̬��0x37
.���ط���������ι���豸��Ϣָ��0x39


*/

void send_cmd2pigs(uint16_t cmd,uint8_t key);

/*
���ط���������ι��ʵ�ʵĽ�ι����ָ��0x38
0x00:������ι��ʵ�ʽ���������
0x01:������ι��ʵ��ι������

*/
void send_cmd_read_foods2pigs(uint16_t cmd,uint8_t key,uint8_t feed_out);




#endif


