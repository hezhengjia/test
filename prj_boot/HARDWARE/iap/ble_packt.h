#ifndef BLE_PACKET
#define BLE_PACKET

#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "in_flash_manage.h"
//#include "protocol.h"


#define UART_FRAM_LEN  300



void ble_unpack_data(uint8_t *pata,uint8_t datalen);
uint8_t ble_packet_data(uint8_t cmd,uint8_t*pdata,uint8_t datalen,uint8_t*pout);



void fill_default_mqtt_data_and_send(void);
#endif



