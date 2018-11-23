#ifndef __UART_PACKT_H__
#define  __UART_PACKT_H__

#include "sys.h"




typedef void (*dataout_fun)(uint16_t cmd,uint8_t*pdatas,uint16_t datalen); // Flash_ReadData


#define UART_FRAM_LEN  300



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


#endif






