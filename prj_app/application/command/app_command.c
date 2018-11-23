/********************************************************************************/
/*@file            :       app_command.c		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Tuesday, November 20, 2018		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2017 Hosea All rights reserved.	*/
/********************************************************************************/

/****************************************INCLUDE*************************************************/

//for different layer, need inculde corresponding hearder file.
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "app.h"
#include "app_command.h"
#include "cJSON.h"
#include "rtc.h"
#include "protocol_uart.h"
#include "in_flash_manage.h"

/**************************** Register our own command **************************/
extern uint8_t app_command_log_lv;
#define TAG 		"[CMD:%d]"
#define LOG(fmt,args...) do{if (app_command_log_lv) printf(TAG fmt, __LINE__, ##args);}while(0)

uint8_t app_command_log_lv = 1;
static void app_command_print_control(int argc, char **argv)
{
	if (0 == strcmp(argv[1], "off"))
		app_command_log_lv = 0;
	else if (0 == strcmp(argv[1], "on"))
		app_command_log_lv = 1;
}
DECLAREE_CMD_FUNC("CMD", app_command_print_control);
/*******************************************************************************/

uint32_t my_json_add_string(uint32_t pos, char *buf, char *string)
{
	strcpy(&buf[pos], string);
	pos += strlen(string);
	buf[pos] = 0;
	return pos;
}

uint32_t my_json_add_valuestring(uint32_t pos, char *buf, char *string, const char* svalue)
{
	pos += sprintf((char*)&buf[pos],"\"%s\":\"%s\",", string, svalue);
	return pos;
}
uint32_t my_json_add_raw_string(uint32_t pos, char *buf, char *string, const char* svalue)
{
	pos += sprintf((char*)&buf[pos],"\"%s\":%s,", string, svalue);
	return pos;
}
uint32_t my_json_add_valueint(uint32_t pos, char *buf, char *string, uint32_t value)
{
	pos += sprintf((char*)&buf[pos],"\"%s\":%d,", string, value);
	return pos;
}

uint32_t my_json_add_valuedouble(uint32_t pos, char *buf, char *string, double value)
{
	pos += sprintf((char*)&buf[pos],"\"%s\":%f,", string, value);
	return pos;
}



uint32_t element_string_get_new_uuid(char *pbuf)
{
	uint32_t loop;
	
	if (pbuf == NULL)
	{
		ASSERT(0, 0);
		return 0;
	}
	for (loop=0; loop<4; loop++)
	{
		sprintf(&(pbuf[loop*8]),"%08x", RNG_Get_RandomNum());
	}
	pbuf[loop*8] = 0;
	return loop*8;
}

uint32_t element_string_get_mac(char *pbuf)
{
	if (pbuf == NULL)
	{
		ASSERT(0, 0);
		return 0;
	}
	return sprintf(pbuf,"%02x%02x%02x%02x%02x%02x",
		StuHis.StuPara.mac[0],\
		StuHis.StuPara.mac[1],\
		StuHis.StuPara.mac[2],\
		StuHis.StuPara.mac[3],\
		StuHis.StuPara.mac[4],\
		StuHis.StuPara.mac[5]\
	);
}

uint32_t element_string_get_feed_id(uint8_t p_feed_id, char *pbuf)
{
	if (pbuf == NULL)
	{
		ASSERT(0, 0);
		return 0;
	}	
	return sprintf(pbuf, "%03d", p_feed_id);
}

uint32_t element_string_get_feed_list(uint8_t *p_feed_list, uint32_t number, char *pbuf)
{
	uint32_t loop;
		
	if (pbuf == NULL)
	{
		ASSERT(0, 0);
		return 0;
	}

	ASSERT(number < 128, 0);
	
	pbuf[0] = '[';
	for (loop=0; loop<number; loop++)
	{
		sprintf(&pbuf[1 + loop*6], "\"%03d\",", *p_feed_list);
		p_feed_list++;
	}
	pbuf[loop*6] = ']';
	pbuf[1 + loop*6] = 0;
	return 1 + loop*6;
}



/*******************************handle item command************************************************/
static uint8_t get_feed_list(int cmd,cJSON *root,uint8_t *list)
{
    uint8_t list_number=0,flg=0,i;
    cJSON *feeder_ids;

    if(cmd == CMD_CTL_UI2006 ||cmd==CMD_CTL_ZERO2008||cmd== CMD_CTL_ONLINE2010 || cmd==CMD_RD_SENSORS2012)
    {
        flg=1;

    }
    if(0==flg) return 0;
    feeder_ids     = cJSON_GetObjectItem( root, "feeder_ids");


    if(cmd == CMD_CTL_ONLINE2010 || cmd == CMD_RD_SENSORS2012)
    {
        list_number=0;
        for(i=0; i<MAX_DEVICE; i++)
            list[list_number++]=list_number;
        return list_number;
    }

    if(feeder_ids!=NULL)
    {
        int  array_size   = cJSON_GetArraySize ( feeder_ids );
        if(0==array_size)
        {
            list_number=0;
            for(i=0; i<MAX_DEVICE; i++)
                list[list_number++]=list_number;
            return list_number;
        }
        for(int iCnt = 0 ; iCnt < array_size ; iCnt ++ )
        {
            cJSON * pSub = cJSON_GetArrayItem(feeder_ids, iCnt);
            if(NULL == pSub || pSub->type!=cJSON_String)
            {

                if(NULL == pSub)
                {
                    printf("feed all pigs\r\n");
                    for(i=0; i<MAX_DEVICE; i++)
                        list[list_number++]=list_number;
                }
                continue ;
            }

            if(list_number<(LIST_SIZE-1))
                list[list_number++]=atoi(pSub->valuestring);
            printf("feeder_ids[%d] : %s  %d-%d\r\n",iCnt,pSub->valuestring,list_number-1,list[list_number-1]);
        }
    }
    return list_number;
}

bool ana_get_feed_id(uint32_t cmd, cJSON *root, uint8_t *out_list, uint16_t *out_list_number)
{
	cJSON *feeder_id;

	*out_list_number = 0;

    feeder_id = cJSON_GetObjectItem(root, "feeder_id");
    if(!feeder_id)
    {
        printf("no feeder_id\r\n");
        *out_list_number = get_feed_list(cmd,root, out_list);
    }
    else if(feeder_id->type!=cJSON_String)
    {
		return false;
    }
	else
	{
		*out_list = atoi(feeder_id->valuestring);
		*out_list_number = 1;		
	}
	return true;
}

void cmd_s2c_online_devices_to_net(uint16_t list_number2,int cmd,my_uuid_t *p_msg_uuid,uint8_t *list2,uint8_t code)
{
    uint8_t *pbuf;
	uint32_t json_pos;
	char temp_buf[50];
	
    pbuf=mymalloc_mmc(1500);
//    tmp32=0;
 //   tmp16=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"feeders\":[",cmd+1,p_msg_uuid);
	
	BEGIN_CREATE_JOSN(pbuf);
	JOSN_ADD_INT(pbuf,			"msg_type", 	cmd + 1);
	if (p_msg_uuid->type == 0)
	{
		JOSN_ADD_INT(pbuf,		"msg_uuid", 	p_msg_uuid->value);
	}
	else
	{
		JOSN_ADD_STRING(pbuf,	"msg_uuid", 	p_msg_uuid->a_uuid);
	}
	
	ASSERT( element_string_get_feed_list(list2, list_number2, temp_buf) < sizeof(temp_buf), 0);
	JOSN_ADD_RAW_STRING(pbuf, 		"feeder_ids", 	temp_buf);
	
	JOSN_ADD_INT(pbuf,			"code", 	code);
	END_CREATE_JOSN(pbuf);

    queue_malloc_in(&stu_queue_malloc_tx,pbuf,json_pos);
    myfree_mmc(pbuf);
}

static void  pack_sensor_packet(STU_DATAS studata,uint8_t *pbuf,uint16_t *plen,int send_cmd, my_uuid_t *p_msg_uuid,uint8_t feed_id)
{
    int16_t tmp16,tmp16_2,tmp16_3,tmp2[3],tatol,pbuf_len,startp;
    float tmpf[3];
	uint32_t json_pos;
	char temp_buf[50];
	
    if(studata.len==0)
    {
 //       *plen=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"code\":%d}",send_cmd,msg_uuid,feed_id,DEVICE_NOT_ONLINE);
		
		BEGIN_CREATE_JOSN(pbuf);
		
		JOSN_ADD_INT(pbuf,			"msg_type", 	send_cmd);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf,		"msg_uuid", 	p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf,	"msg_uuid", 	p_msg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf,		"feeder_id", 	temp_buf);
		JOSN_ADD_INT(pbuf,			"code", 		DEVICE_NOT_ONLINE);

		END_CREATE_JOSN(pbuf);
        return;
    }
    tatol = 0;
    pbuf_len = 0;
    startp = 16;

    tmp16=studata.pdata[startp+3];
    tmp16=(tmp16<<8)|studata.pdata[startp+4];
    if(studata.pdata[startp+3] == 0xff && studata.pdata[startp+4]==0xff)
	{
		tmp16 = -1000;
	}

    tmp2[0]=studata.pdata[startp+1];
    tmp2[0]=(tmp2[0]<<8)|studata.pdata[startp+2];
    tmpf[0]=tmp2[0];
    if(0xffff == (uint16_t)tmp2[0])
    {
        tmpf[0] = -1000;
    }
    else
    {
        tmpf[0] = tmpf[0]/100;
    }

 //   tatol=sprintf((char*)pbuf+pbuf_len,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"feeder_mac\":\"%02X%02X%02X\",\"device_status\":%d,\"g-sensor\":%.2f,\"weight\":%d,",\
					send_cmd,msg_uuid,feed_id,\
                  studata.pdata[1], studata.pdata[2], studata.pdata[3],\
                  (studata.pdata[startp]==0xff)? -1000:studata.pdata[startp],\
                  tmpf[0],\
                  tmp16);
	
		BEGIN_CREATE_JOSN(pbuf);
		
		JOSN_ADD_INT(pbuf,			"msg_type", 	send_cmd);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf,		"msg_uuid", 	p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf,	"msg_uuid",		p_msg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf,		"feeder_id",	temp_buf);
		
		sprintf(temp_buf, "%02x%02x%02x", studata.pdata[1], studata.pdata[2], studata.pdata[3]);
		JOSN_ADD_STRING(pbuf,		"feeder_mac",	temp_buf);
		JOSN_ADD_INT(pbuf,			"device_status",(studata.pdata[startp]==0xff)? -1000:studata.pdata[startp]);
		JOSN_ADD_FLOAT(pbuf,		"g_sensor",		tmpf[0]);
		JOSN_ADD_INT(pbuf,			"weight",		tmp16);
		
    //======================================================================================================

    startp+=5;//16
    //21

    pbuf_len+=tatol;
//    tatol=sprintf((char*)pbuf+pbuf_len,"\"humidity\":%d,\"env_temp\":%d,\"pig_temp\":%d,\"sow_action\":%d,\"boar_action\":%d,",\
                  (studata.pdata[startp]==0xff)? -1000:studata.pdata[startp],\
                  (studata.pdata[startp+1]==0xff)? -1000:studata.pdata[startp+1],\
                  (studata.pdata[startp+2]==0xff)? -1000:studata.pdata[startp+2],\
                  (studata.pdata[startp+3]==0xff)? -1000:studata.pdata[startp+3],\
                  (studata.pdata[startp+4]==0xff)? -1000:studata.pdata[startp+4]\
                 );
	JOSN_ADD_INT(pbuf, 	"humidity", 			(studata.pdata[startp]==0xff)? -1000:studata.pdata[startp]);
	JOSN_ADD_INT(pbuf, 	"env_temp", 			(studata.pdata[startp+1]==0xff)? -1000:studata.pdata[startp+1]);
	JOSN_ADD_INT(pbuf, 	"pig_temp", 			(studata.pdata[startp+2]==0xff)? -1000:studata.pdata[startp+2]);
	JOSN_ADD_INT(pbuf, 	"sow_action", 			(studata.pdata[startp+3]==0xff)? -1000:studata.pdata[startp+3]);
	JOSN_ADD_INT(pbuf, 	"boar_action", 			(studata.pdata[startp+4]==0xff)? -1000:studata.pdata[startp+4]);

    startp+=5;//
    pbuf_len+=tatol;

    tmp16=studata.pdata[startp];
    tmp16=(tmp16<<8)|studata.pdata[startp+1];
    tmp16_2=studata.pdata[startp+2];
    tmp16_2=(tmp16_2<<8)|studata.pdata[startp+3];
    tmp16_3=studata.pdata[startp+4];
    tmp16_3=(tmp16_3<<8)|studata.pdata[startp+5];

    if(tmp16==0xffff)
        tmp16=-1000;
    if(tmp16_2==0xffff)
        tmp16_2=-1000;
    if(tmp16_3==0xffff)
        tmp16_3=-1000;

    startp+=6;


    tmp2[0]=studata.pdata[startp];//34
    tmp2[0]=(tmp2[0]<<8)|studata.pdata[startp+1];

    tmp2[1]=studata.pdata[startp+2];
    tmp2[1]=(tmp2[1]<<8)|studata.pdata[startp+3];

    tmp2[2]=studata.pdata[startp+4];
    tmp2[2]=(tmp2[2]<<8)|studata.pdata[startp+5];
    startp+=6;


    if(tmp2[0]==0xffff)
    {
        tmpf[0]=-1000;
    }
    else
    {
        tmpf[0]=tmp2[0];
        tmpf[0]=tmpf[0]/100;

    }
    if(tmp2[1]==0xffff)
    {
        tmpf[1]=-1000;
    }
    else
    {
        tmpf[1]=tmp2[1];
        tmpf[1]=tmpf[1]/10;

    }
    if(tmp2[2]==0xffff)
    {
        tmpf[2]=-1000;
    }
    else
    {

        tmpf[2]=tmp2[2];
        tmpf[2]=tmpf[2]/10;

    }

	JOSN_ADD_INT(pbuf, 	"distance_1", 			tmp16);
	JOSN_ADD_INT(pbuf, 	"distance_2", 			tmp16_2);
	JOSN_ADD_INT(pbuf, 	"distance_vertical", 	tmp16_3);
	JOSN_ADD_FLOAT(pbuf, "NH3", 				tmpf[0]);
	JOSN_ADD_FLOAT(pbuf, "CO2", 				tmpf[1]);
	JOSN_ADD_FLOAT(pbuf, "illumination", 		tmpf[2]);
	
	END_CREATE_JOSN(pbuf);

    *plen = json_pos;
}


void cmd_c2s_asynchronous_to_net1(int cmd, my_uuid_t *p_msg_uuid,uint8_t feed_id,uint8_t code,STU_DATAS studata)
{
    uint16_t len;
    uint8_t*pbuf;
	uint32_t json_pos;
	char temp_buf[50];
	
    char *sortware_version,*hardware_version,*str_mac;

    if(CMD_CTL_ONLINE2010==cmd)  return;

    pbuf=mymalloc_mmc(500);
	json_pos = 0;
    if(CMD_RD_SENSORS2012==cmd)
    {
        pack_sensor_packet(studata,pbuf,&len,cmd+1,p_msg_uuid,feed_id);
		json_pos = len;
    }
    else if(REQUEST_NET_INFOR_9000==cmd)
    {
        sortware_version=mymalloc_mmc(50);
        hardware_version=mymalloc_mmc(50);
        str_mac=mymalloc_mmc(50);
        memset(sortware_version,0,50);
        memset(hardware_version,0,50);
        memset(str_mac,0,50);

        sprintf(sortware_version,"%d.%d.%d",version[0],version[1],version[2]);
        sprintf(hardware_version,"%d.%d.%d",version[7],version[8],version[9]);
		element_string_get_mac(str_mac);

 //       len=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"remote_ip\":\"%s\",\"remote port\":%d,\"software version\":%s,\"hardware version\":%s,\"mac\":%s,\"code\"%d}", \
                    cmd+1,msg_uuid,&StuHis.StuPara.remote_ip[0],StuHis.StuPara.remote_port,sortware_version,hardware_version,str_mac,code);
		
		BEGIN_CREATE_JOSN(pbuf);
		JOSN_ADD_INT(pbuf,			"msg_type",			cmd + 1);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf,		"msg_uuid",			p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf,	"msg_uuid",			p_msg_uuid->a_uuid);
		}
		JOSN_ADD_STRING(pbuf,		"remote_ip",		(char *)&StuHis.StuPara.remote_ip[0]);
		JOSN_ADD_INT(pbuf,			"remote port",		StuHis.StuPara.remote_port);
		JOSN_ADD_STRING(pbuf,		"software version",	sortware_version);
		JOSN_ADD_STRING(pbuf,		"hardware_version",	hardware_version);
		JOSN_ADD_STRING(pbuf,		"mac",				str_mac);

		
		END_CREATE_JOSN(pbuf);
		
		myfree_mmc(str_mac);
		myfree_mmc(hardware_version);
        myfree_mmc(sortware_version);
    }
    else    if(REQUEST_DEVICE_INFOR_9002==cmd && 0==code)
    {
        sortware_version=mymalloc_mmc(50);
        hardware_version=mymalloc_mmc(50);
        str_mac=mymalloc_mmc(50);
        memset(sortware_version,0,50);
        memset(hardware_version,0,50);
        memset(str_mac,0,50);


        sprintf(sortware_version,"%d.%d.%d",studata.pdata[16],studata.pdata[17],studata.pdata[18]);
        sprintf(hardware_version,"%d.%d.%d",studata.pdata[19],studata.pdata[20],studata.pdata[21]);
        sprintf(str_mac,"%02X%02X%02X",studata.pdata[1],studata.pdata[2],studata.pdata[3]);

//        len=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"software version\":%s,\"hardware version\":%s,\"feeder_mac\":%s,\"code\"%d}", \
                    cmd+1,msg_uuid,studata.pdata[0],sortware_version,hardware_version,str_mac,code);
		
		BEGIN_CREATE_JOSN(pbuf);
		JOSN_ADD_INT(pbuf, 		"msg_type",			cmd + 1);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf, 	"msg_uuid",			p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf, "msg_uuid",		p_msg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf, "feeder_id", 			temp_buf);
		
		JOSN_ADD_STRING(pbuf, "software version", 	sortware_version);
		JOSN_ADD_STRING(pbuf, "hardware_version", 	hardware_version);
		JOSN_ADD_STRING(pbuf, "feeder_mac", 		str_mac);
		JOSN_ADD_INT(pbuf, 	"code", 				code);

		END_CREATE_JOSN(pbuf);
		
        myfree_mmc(sortware_version);
        myfree_mmc(hardware_version);
        myfree_mmc(str_mac);
    }
    else if (REQUEST_MQTT_INFOR_9004 == cmd )
    {
  //      len=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"code\":%d}",cmd+1,msg_uuid,code);
		
		BEGIN_CREATE_JOSN(pbuf);
		JOSN_ADD_INT(pbuf, 		"msg_type", 	cmd + 1);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf, 	"msg_uuid", 	p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf, "msg_uuid", 	p_msg_uuid->a_uuid);
		}
		JOSN_ADD_INT(pbuf, 	"code", 				code);

		END_CREATE_JOSN(pbuf);		
    }
    else
    {
//        len=sprintf((char*)pbuf,"{\"msg_type\":%04d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"code\":%d}",cmd+1,msg_uuid,feed_id,code);
		BEGIN_CREATE_JOSN(pbuf);
		JOSN_ADD_INT(pbuf, 		"msg_type", 	cmd + 1);
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf, 	"msg_uuid", 	p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf, "msg_uuid", 	p_msg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf, "feeder_id", 			temp_buf);
		
		JOSN_ADD_INT(pbuf, 	"code", 				code);

		END_CREATE_JOSN(pbuf);		
    }
	len = json_pos;
    queue_malloc_in(&stu_queue_malloc_tx,pbuf,len);
    myfree_mmc(pbuf);
}

/*
int cmd                    --->CMD_FOOD_IN2000  or CMD_FOOD_OUT2002
int msg_uuid             ---> int type
uint8_t feed_id          --->int device uuid
int real_amount         --->int unit g
uint8_t motorstate     ---> 0--->ok   1--->¶Â×ª
,uint32_t gsensro       ---> 0.01¾«¶È
uint8_t code             --->0-->ok    10-->ËÇÎ¹Æ÷¹ÊÕÏ9--->ËÇÎ¹Æ÷³¬Ê±   8--->ËÇÎ¹Æ÷²»ÔÚÏß
*/
void cmd_c2s_asynchronous_to_net(int cmd, my_uuid_t *pmsg_uuid,uint8_t feed_id,int real_amount,int16_t gsensro,uint8_t code)
{
    uint16_t json_pos;
    uint8_t*pbuf;
    float ss;
	char temp_buf[50];
    ss=gsensro;
    ss=ss/100;
    if(0xffff == (uint16_t)gsensro)
    {
        ss = -1000;
    }
    pbuf=mymalloc_mmc(500);

    if(0==code||12==code)
	{
//		len=sprintf((char*)pbuf,"{\"msg_type\":%d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"\":%d,\"motor\":%d,\"g_sensor\":%.2f,\"code\":%d}",\
			1+cmd,msg_uuid,feed_id,real_amount,0,ss,code);

		BEGIN_CREATE_JOSN(pbuf);
		
		JOSN_ADD_INT(pbuf, 			"msg_type", 	1+cmd);
		if (pmsg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf, 		"msg_uuid", 	pmsg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf, 	"msg_uuid", 	pmsg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf, 		"feeder_id", 	temp_buf);
		
		JOSN_ADD_INT(pbuf, 			"real_amount", 	real_amount);
		JOSN_ADD_INT(pbuf, 			"motor", 		0);
		JOSN_ADD_FLOAT(pbuf, 		"g_sensor", 	ss);
		JOSN_ADD_INT(pbuf, 			"code", 		code);

		END_CREATE_JOSN(pbuf);
	}
	else
	{
//        len=sprintf((char*)pbuf,"{\"msg_type\":%d,\"msg_uuid\":\"%s\",\"feeder_id\":\"%03d\",\"code\":%d}",1+cmd,msg_uuid,feed_id,code);
		
		BEGIN_CREATE_JOSN(pbuf);
		
		JOSN_ADD_INT(pbuf, 			"msg_type", 	1+cmd);
		if (pmsg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf,		"msg_uuid", 	pmsg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf,	"msg_uuid", 	pmsg_uuid->a_uuid);
		}
		
		ASSERT( element_string_get_feed_id(feed_id, temp_buf) < sizeof(temp_buf), 0);
		JOSN_ADD_STRING(pbuf, 		"feeder_id", 	temp_buf);
		
		JOSN_ADD_INT(pbuf, 			"code", 		code);

		END_CREATE_JOSN(pbuf);		
	}

    queue_malloc_in(&stu_queue_malloc_tx,pbuf,json_pos);

    myfree_mmc(pbuf);
}

/*******************handle all error response***********************/
uint16_t get_register_packet(uint8_t *buf, uint32_t type)
{
	uint32_t json_pos, length;
	char tempbuf[50];
	
	BEGIN_CREATE_JOSN(buf);
	JOSN_ADD_INT(buf, 		"msg_type",		8000);
	
	length = element_string_get_new_uuid(tempbuf);
	ASSERT(length < sizeof(tempbuf), 0);
	JOSN_ADD_STRING(buf, 	"msg_uuid",		tempbuf);
	
	length = element_string_get_mac(tempbuf);
	ASSERT(length < sizeof(tempbuf), 0);
	JOSN_ADD_STRING(buf, 	"mac_address",	tempbuf);

	JOSN_ADD_INT(buf, 		"register_type",type);
	END_CREATE_JOSN(buf);

	return json_pos;
}

uint16_t cmd_error_resoponse(int cmd, my_uuid_t *p_msg_uuid, uint8_t code, const char *buf)
{
    uint8_t *pbuf;
	uint32_t json_pos;

	pbuf=mymalloc_mmc(1500);
	
	BEGIN_CREATE_JOSN(pbuf);
	JOSN_ADD_INT(pbuf,			"msg_type", 	cmd + 1);
	if (p_msg_uuid)
	{
		if (p_msg_uuid->type == 0)
		{
			JOSN_ADD_INT(pbuf,		"msg_uuid", 	p_msg_uuid->value);
		}
		else
		{
			JOSN_ADD_STRING(pbuf,	"msg_uuid", 	p_msg_uuid->a_uuid);
		}
	}
	JOSN_ADD_INT(pbuf, 			"code", 		code);
	END_CREATE_JOSN(pbuf);

    queue_malloc_in(&stu_queue_malloc_tx,pbuf,json_pos);
    myfree_mmc(pbuf);

	return json_pos;
}




void add_id2wait_list(int cmd,my_uuid_t *p_msg_uuid,uint8_t feed_id);


/******************handle service to client command***********************/
uint32_t cmd_s2c_food_in_out(uint32_t cmd, uint32_t para2, uint32_t para3)
{
	cJSON *root = (cJSON *)para2;
    my_uuid_t *p_msg_uuid = (my_uuid_t *)para3;

	cJSON *amount;
	uint8_t feed_id;
	uint16_t list_number;
	uint8_t *list;
	
	amount = cJSON_GetObjectItem( root, "amount");
	if( (amount == NULL) || (amount->type != cJSON_Number) )
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
	
    list = mymalloc_mmc(LIST_SIZE);
	if (list == NULL)
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
	
	if (!ana_get_feed_id(cmd, root, list, &list_number))
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		myfree_mmc(list);
		return 0;
	}
	feed_id = *list;
	myfree_mmc(list);
	if (list_number != 1)
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}

	if(cmd == CMD_FOOD_IN2000)
	{
		open_relay=1;
	}

    uint16_t j;
    uint8_t cmd485=GATE_FOOD_OUT33;
    if(cmd==CMD_FOOD_IN2000)
    {
        cmd485=GATE_FOOD_IN32;
    }
//     1.send food msg 2 pigs
    prepare_wait_485_cmd(feed_id);
    for(j=0; j<3; j++)
    {
        send_food2pigs(cmd485, amount->valueint, feed_id);
        if(true==wait_485_cmd(PIGS_ACK,WAIT_PIG_TIME,feed_id))
        {
            break;
        }
    }

    if(Stu485rev.cmd!=PIGS_ACK)
    {
        cmd_c2s_asynchronous_to_net(cmd,p_msg_uuid,feed_id,0,0,DEVICE_NOT_ONLINE);
    }
	else
	{
		add_id2wait_list(cmd,p_msg_uuid,feed_id);
	}
	return 0;
}

uint32_t cmd_s2c_led_period(uint32_t cmd, uint32_t para2, uint32_t para3)
{
	cJSON *root = (cJSON *)para2;
    my_uuid_t *p_msg_uuid = (my_uuid_t *)para3;
    uint16_t j;
    uint8_t wait_cmd;
    STU_DATAS studatas;

	cJSON *amount;
	uint8_t feed_id;
	uint16_t list_number;
	uint8_t *list;
	if(CMD_CTL_LED2004 == cmd)
	{
		amount     = cJSON_GetObjectItem( root, "color");
		if(amount==NULL ||amount->type!=cJSON_Number)
		{
			ASSERT(0, 0);
			cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
			return 0;
		}
	}
	else if(CMD_ST_PERIOD2014 == cmd)
	{
		amount     = cJSON_GetObjectItem( root, "sow_period");
		if(amount==NULL ||amount->type!=cJSON_Number)
		{
			ASSERT(0, 0);
			cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
			return 0;
		}
	}
	else
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
    list = mymalloc_mmc(LIST_SIZE);
	if (list == NULL)
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
	
	if (!ana_get_feed_id(cmd, root, list, &list_number))
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		myfree_mmc(list);
		return 0;
	}
	feed_id = *list;
	myfree_mmc(list);
	if (list_number != 1)
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}

    prepare_wait_485_cmd(feed_id);
    for(j=0; j<3; j++)
    {
        wait_cmd= uart_data2pigs_all_command(cmd,amount->valueint,feed_id);

        if(true==wait_485_cmd(wait_cmd,WAIT_PIG_TIME,feed_id))
        {
            break;
        }
    }
//    2.get onlines 2 list3
    cmd_c2s_asynchronous_to_net1(cmd,p_msg_uuid,feed_id,Stu485rev.cmd!=wait_cmd?DEVICE_NOT_ONLINE:0,studatas);
	return 0;
}

uint32_t cmd_s2c_get_timestamp(uint32_t cmd, uint32_t para2, uint32_t para3)
{
	cJSON *root = (cJSON *)para2;
    my_uuid_t *p_msg_uuid = (my_uuid_t *)para3;
	
	
    uint8_t *pbuf, code_vale;
	uint32_t json_pos;
	uint32_t time_value;
	cJSON *json_time;
	
    
	code_vale = 0;
	json_time = cJSON_GetObjectItem( root, "timestamp");
	if(json_time==NULL ||json_time->type!=cJSON_Number)
	{
		code_vale = 0;
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
	else
	{
		time_value = json_time->valueint;
		set_tick2rtc(time_value);		
	}
	
	pbuf=mymalloc_mmc(1500);
	
	BEGIN_CREATE_JOSN(pbuf);
	JOSN_ADD_INT(pbuf,			"msg_type", 	cmd + 1);
	if (p_msg_uuid->type == 0)
	{
		JOSN_ADD_INT(pbuf,		"msg_uuid", 	p_msg_uuid->value);
	}
	else
	{
		JOSN_ADD_STRING(pbuf,	"msg_uuid", 	p_msg_uuid->a_uuid);
	}
	JOSN_ADD_INT(pbuf, 			"code", 		code_vale);

	END_CREATE_JOSN(pbuf);

    queue_malloc_in(&stu_queue_malloc_tx,pbuf,json_pos);
    myfree_mmc(pbuf);
	
	return 0;
}


uint32_t cmd_s2c_06_12_in_out(uint32_t cmd, uint32_t para2, uint32_t para3)
{
	cJSON *root = (cJSON *)para2;
    my_uuid_t *p_msg_uuid = (my_uuid_t *)para3;
	cJSON *amount;

    uint16_t j,i;
    uint8_t *list2;
    uint16_t list_number2;
    uint8_t cmd_wait;
    STU_DATAS *pstudatas;

    uint8_t *list_offfline;
    uint16_t list_offline_number;

	uint16_t list_number, mount_value;
	uint8_t *list;
	
	if(CMD_CTL_UI2006 == cmd)
	{
		amount = cJSON_GetObjectItem( root, "ui_number");
		if( (amount == NULL) || (amount->type != cJSON_Number) )
		{
			ASSERT(0, 0);
			cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
			return 0;
		}
		mount_value = amount->valueint;
	}
	else
	{
		mount_value = 0;
	}

    list = mymalloc_mmc(LIST_SIZE);
	if (list == NULL)
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		return 0;
	}
	
	if (!ana_get_feed_id(cmd, root, list, &list_number))
	{
		ASSERT(0, 0);
		cmd_error_resoponse(cmd, p_msg_uuid, 2, NULL);
		myfree_mmc(list);
		return 0;
	}

    list_number2 = 0;
    list2 = mymalloc_mmc(LIST_SIZE);
    memset(list2,OFFLINE_DEVICE_ID,LIST_SIZE);

    list_offline_number=0;
    list_offfline = mymalloc_mmc(LIST_SIZE);

    pstudatas=mymalloc_mmc(LIST_SIZE*sizeof(STU_DATAS));
    memset(pstudatas,0,LIST_SIZE*sizeof(STU_DATAS));

//     1.send  msg 2 pigs
    for(i=0; i<list_number; i++)
    {
        prepare_wait_485_cmd(list[i]);
        for(j=0; j<2; j++)
        {
            cmd_wait= uart_data2pigs_all_command(cmd, mount_value, list[i]);
            if(true==wait_485_cmd(cmd_wait,WAIT_PIG_TIME,list[i]))
            {
                list2[list_number2]=list[i];
                pstudatas[list_number2].len=Stu485rev.len;
                pstudatas[list_number2].pdata=mymalloc_mmc(Stu485rev.len);
                memcpy(pstudatas[list_number2].pdata,Stu485rev.data,Stu485rev.len);
                list_number2++;
                break;
            }
        }

        if(Stu485rev.cmd!=cmd_wait)
        {
            list_offfline[list_offline_number++]=list[i];
        }
        IWDG_Feed();
    }

    if(CMD_CTL_ONLINE2010==cmd)
    {
        cmd_s2c_online_devices_to_net(list_number2,cmd,p_msg_uuid,list2,0);
    }
    else
    {
        //    2.get onlines 2 list3
        for(i=0; i<list_number2; i++)
        {
            cmd_c2s_asynchronous_to_net1(cmd,p_msg_uuid,list2[i],0,pstudatas[i]);
        }

        if(CMD_RD_SENSORS2012!=cmd)
        {
            //offline
            pstudatas[0].len=0;
            for(i=0; i<list_offline_number; i++)
            {
                cmd_c2s_asynchronous_to_net1(cmd,p_msg_uuid,list_offfline[i],DEVICE_NOT_ONLINE,pstudatas[0]);
            }
        }
    }
//    3. release
    for(i=0; i<list_number2; i++)
    {
        if(list2[i]!=OFFLINE_DEVICE_ID)
        {
            myfree_mmc(pstudatas[i].pdata);
        }
    }

	myfree_mmc(list);
    myfree_mmc(list2);
    myfree_mmc(pstudatas);	
	
	return 0;
}


uint8_t uart_data2pigs_all_command(int cmd,uint16_t foods,uint8_t key)
{
    switch(cmd)
    {
        case CMD_CTL_LED2004:
        case CMD_CTL_UI2006:
            send_led_lcd2pigs((cmd==CMD_CTL_LED2004)? GATE_CTL_LED34 : GATE_CTL_LCD35,(uint8_t)foods,key);
            return PIGS_ACK;
        case CMD_CTL_ZERO2008:
            send_cmd2pigs(GATE_CTL_ZERO36,key);
            return PIGS_ACK;
        case CMD_ST_PERIOD2014:
            send_led_lcd2pigs(GATE_ST_PERIOD3A,foods,key);
            return PIGS_ACK;
        case  REQUEST_DEVICE_INFOR_9002:
        case  CMD_CTL_ONLINE2010:
            send_cmd2pigs(GATE_RD_INFO39,key);
            return PIGS_NFO;
        case CMD_RD_SENSORS2012:
            send_cmd2pigs(GATE_RD_SENSORS37,key);
            return PIGS_SENSORS;
    }
    printf("err======>%s-%d\r\n",__FILE__,__LINE__);
    return 0;
}
