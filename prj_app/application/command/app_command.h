/********************************************************************************/
/*@file            :       app_command.h		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Tuesday, November 20, 2018		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2017 OFO. All rights reserved.	*/
/********************************************************************************/

#ifndef __APP_COMMAND_H__
#define __APP_COMMAND_H__

/****************************************INCLUDE*************************************************/
//for different layer, need inculde corresponding hearder file.
#include "app.h"
 
#define UUID_STRING_LENGTH  					32

#define	BEGIN_CREATE_JOSN(buf)					json_pos=0;json_pos=my_json_add_string(json_pos,(char*)buf,"{");

#define	JOSN_ADD_STRING(buf, type, string)		json_pos=my_json_add_valuestring(json_pos,(char*)buf,type,string);
#define	JOSN_ADD_INT(buf, type, value)			json_pos=my_json_add_valueint(json_pos,(char*)buf,type,value);
#define	JOSN_ADD_FLOAT(buf, type, value)		json_pos=my_json_add_valuedouble(json_pos,(char*)buf,type,value);
#define	JOSN_ADD_RAW_STRING(buf, type, string)	json_pos=my_json_add_raw_string(json_pos,(char*)buf,type,string);

#define	END_CREATE_JOSN(buf)					json_pos=my_json_add_string(json_pos-1,(char*)buf,"}");

/****************************************** MACROS **********************************************/
typedef struct tag_my_uuid_t
{
    int type;
	int value;
    char a_uuid[UUID_STRING_LENGTH + 1];
}my_uuid_t;

/****************************************** TYPEDEFS *********************************************/


/****************************************** Constants  *******************************************/


/*************************************** GLOBAL VARIABLES ****************************************/
uint32_t element_string_get_mac(char *pbuf);
uint32_t element_string_get_new_uuid(char *pbuf);

/****************************************** FUNCTIONS ********************************************/
uint32_t my_json_add_string(uint32_t pos, char *buf, char *string);
uint32_t my_json_add_valuestring(uint32_t pos, char *buf, char *string, const char* svalue);
uint32_t my_json_add_valueint(uint32_t pos, char *buf, char *string, uint32_t value);
uint32_t my_json_add_valuedouble(uint32_t pos, char *buf, char *string, double value);
uint32_t my_json_add_raw_string(uint32_t pos, char *buf, char *string, const char* svalue);

uint16_t cmd_error_resoponse(int cmd, my_uuid_t *p_msg_uuid, uint8_t code, const char *buf);

uint16_t get_register_packet(uint8_t *buf, uint32_t type);

uint32_t cmd_s2c_food_in_out(uint32_t cmd, uint32_t para2, uint32_t para3);
uint32_t cmd_s2c_led_period(uint32_t cmd, uint32_t para2, uint32_t para3);
uint32_t cmd_s2c_get_timestamp(uint32_t cmd, uint32_t para2, uint32_t para3);
uint32_t cmd_s2c_06_12_in_out(uint32_t cmd, uint32_t para2, uint32_t para3);
void cmd_c2s_asynchronous_to_net(int cmd, my_uuid_t *pmsg_uuid,uint8_t feed_id,int real_amount,int16_t gsensro,uint8_t code);

uint8_t uart_data2pigs_all_command(int cmd,uint16_t foods,uint8_t key);











#endif //__APP_COMMAND_H__
