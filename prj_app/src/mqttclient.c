#include "mqttclient.h"
#include "transport.h"
#include "MQTTPacket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "sockets.h"
#include "in_flash_manage.h"
#include "stm32f4x7_eth_bsp.h"
#include "netconf.h"
#include "stdbool.h"
#include "app.h"
#include "cJSON.h"
#include "malloc.h"
#include "bytes_fifo_malloc.h"
#include "rtc.h"
#include "application.h"


extern uint8_t app_mqtt_lv;
#define TAG 		"[MQTT:%d]"
#define LOG(fmt,args...) do{if (app_mqtt_lv) printf(TAG fmt, __LINE__, ##args);}while(0)

uint8_t app_mqtt_lv = 1;
static void app_mqtt_print_control(int argc, char **argv)
{
	if (0 == strcmp(argv[1], "off"))
		app_mqtt_lv = 0;
	else if (0 == strcmp(argv[1], "on"))
		app_mqtt_lv = 1;
}
DECLAREE_CMD_FUNC("MQTT", app_mqtt_print_control);


static uint8_t ask_subcribe=0;
uint8_t net_connect_flg=0;
uint8_t fst_gate_regster = 0;
uint8_t fst_first_regster = 0;
static void ana_register_msg(uint8_t*msg)
{
    cJSON *root;
    cJSON *msg_type;
    cJSON *gate_id;
    cJSON *code;

    root = cJSON_Parse((char*)msg);
    if(!root)
    {
        LOG("jason format err\r\n");
        return ;
    }

    msg_type = cJSON_GetObjectItem(root, "msg_type");

    if( (0==msg_type)||(msg_type->type!=cJSON_Number)||(msg_type->valueint != 8001 ))//  ||  ||
    {
        LOG("msg_type err\r\n");
        goto END ;

    }

    gate_id = cJSON_GetObjectItem(root, "gateway_id");
    if(!gate_id || gate_id->type!=cJSON_String)
    {
        LOG(" gate_id err\r\n");
        goto END ;

    }
    code = cJSON_GetObjectItem(root, "code");
    if( (!code)||code->type!=cJSON_Number||(code->valueint!=0))
    {
        LOG("code err\r\n");
        goto END ;
    }

	uint32_t time_value;
	cJSON *json_time;
	json_time = cJSON_GetObjectItem( root, "timestamp");
	if (json_time)
	{
		time_value = json_time->valueint;
		set_tick2rtc(time_value);		
	}
	
    memset(StuHis.StuPara.gatewayid,0,MAX_GATWAY_ID);
    memcpy(StuHis.StuPara.gatewayid,gate_id->valuestring,strlen(gate_id->valuestring));
    StuHis.StuPara.registerted = DEVICE_REGISTERED;
    ask_ansy(AN_WRITE);
	
    ask_subcribe = 1;
	fst_gate_regster = 1;
	if (fst_first_regster == 0)
	{
		fst_first_regster = 1;
	}
    LOG("register ok\r\n");
END:
    if (root)
        cJSON_Delete(root);
}

static s32 open_and_init_mqtt(s32* pmysock)
{
    //������������
    LOG("start\r\n");
    LOG("remote ip��%s,port:%0d\r\n",StuHis.StuPara.remote_ip,StuHis.StuPara.remote_port);

    while(1)
    {
        if(net_insert_status)
        {
            *pmysock = transport_open((s8*)StuHis.StuPara.remote_ip,StuHis.StuPara.remote_port);
            if(*pmysock >= 0)
            {
                LOG("conected�\\r\n");
                break;
            }
            else
            {
                LOG("connected err,wait 5sec�\r\n");
            }
        }
        else
        {
            LOG("wire not insert\r\n");
        }
        vTaskDelay(5000/portTICK_RATE_MS);
    }

    return  MQTTClientInit(*pmysock);

}


const uint8_t topic_regist[]="topic/manager/gateway/upward";
const uint8_t topic_regist_ack[]="topic/manager/gateway/downward";
//const uint8_t topic_send_msg[]="topic/{gateway_id}/gateway_to_app";
//const uint8_t topic_rev_msg[]="topic/{gateway_id}/app_to_gateway";


static bool subcribe_msg(s32*psocket,uint8_t *buf, uint32_t type)
{
	switch (type)
	{
		case 0:
			if(MQTTSubscribe(*psocket,(s8*)topic_regist_ack,QOS2) < 0)
			{
				//����������
				LOG("subcribe %s err1...\r\n",topic_regist_ack);
				return false;
			}
			else
			{
				LOG("subcribe %s success \r\n",topic_regist_ack);
			}
			break;
		case 1:
			sprintf((char*)buf,"topic/%s/app_to_gateway",StuHis.StuPara.gatewayid);
			if(MQTTSubscribe(*psocket,(s8*)buf,QOS2) < 0)
			{
				//����������
				LOG("subcribe %s err...\r\n",buf);
				return false;
			}
			else
			{
				LOG("subcribe %s success \r\n",buf);
			}
			break;
		default:
			return false;
		//	break;
	}
    return true;
}



//�����û���Ϣ�ṹ��
MQTT_USER_MSG  mqtt_user_msg;
uint8_t revtopic[MSG_TOPIC_LEN];







void deliverMessage(MQTTString *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg);

/************************************************************************
** ��������: mqtt_thread
** ��������: MQTT����
** ��ڲ���: void *pvParameters���������
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void mqtt_thread(void *pvParameters)
{
    s32 mysock = 0;
    u32 curtick,pubtick;
    u8 no_mqtt_msg_exchange = 1;
    s32 sessionPresent = 0;
    static u8 sedbuf[MSG_TOPIC_LEN];
    uint8_t *pbuf;
    s32 buflen,type,ret;
    uint16_t len;
    uint8_t packets;
    fd_set readfd;
    struct timeval tv;

    memset(revtopic,0,MSG_TOPIC_LEN);

    while(0==get_router_ip)
    {
        LOG("wait dhcp get router ip \r\n");
        vTaskDelay(5000/portTICK_RATE_MS);
    }

MQTT_START:
    sessionPresent =  open_and_init_mqtt(&mysock);//connect ===================
	ask_subcribe = 1;
	fst_gate_regster = 0;
    if(sessionPresent < 0)
    {
        LOG("client connect to server err..\r\n");
        goto MQTT_reconnect;
    }
    pbuf=QUEUE_MALLOC(256);
    if(sessionPresent != 1 && false== subcribe_msg(&mysock,pbuf, 0))//subcribe=======================
    {
        QUEUE_FREE(pbuf);
        goto MQTT_reconnect;
    }
    QUEUE_FREE(pbuf);

    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
    curtick = xTaskGetTickCount();

    pubtick = xTaskGetTickCount()+50000;
    if(StuHis.StuPara.registerted == DEVICE_REGISTERED)
    {
        memset(sedbuf,0,MSG_TOPIC_LEN);
        memset(revtopic,0,MSG_TOPIC_LEN);
        sprintf((char*)sedbuf,"topic/%s/gateway_to_app",StuHis.StuPara.gatewayid);
        sprintf((char*)revtopic,"topic/%s/app_to_gateway",StuHis.StuPara.gatewayid);
    }
    net_connect_flg=1;
    while(1)
    {
//���������ݽ���
        no_mqtt_msg_exchange = 1;//read msg=======================================

//������Ϣ
        FD_ZERO(&readfd);
        FD_SET(mysock,&readfd);

//�ȴ��ɶ��¼�
		tv.tv_sec = 0;
		tv.tv_usec = 300;
        
        select(mysock+1,&readfd,NULL,NULL,&tv);

//�ж�MQTT�������Ƿ�������
        if(FD_ISSET(mysock,&readfd) != 0)
        {
            pbuf=QUEUE_MALLOC(MSG_MAX_LEN);
            //��ȡ���ݰ�--ע���������Ϊ0��������
            type = ReadPacketTimeout(mysock,pbuf,MSG_MAX_LEN,0);
            if(type != -1)
            {
                LOG("type:%d\r\n",type);
                mqtt_pktype_ctl(type,pbuf,MSG_MAX_LEN);

                //���������ݽ���
                no_mqtt_msg_exchange = 0;
                //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
                curtick = xTaskGetTickCount();
            }
            QUEUE_FREE(pbuf);
        }

		if (fst_gate_regster == 0)
        {
            if((xTaskGetTickCount() - pubtick) >(5000))
            {
				uint8_t type;
                pubtick = xTaskGetTickCount();
                pbuf=QUEUE_MALLOC(256);
				
				if(StuHis.StuPara.registerted != DEVICE_REGISTERED )
				{
					type = 0;
				}
				else
				{
					if (fst_first_regster == 0)
					{
						type = 1;
					}
					else
					{
						type = 2;
					}
				}
                buflen=get_register_packet(pbuf, type);
				
                ret = MQTTMsgPublish(mysock,(s8*)topic_regist,QOS2, 0,(u8*)pbuf,buflen);
                
                if(ret >= 0)
                {
                    //���������ݽ���
                    no_mqtt_msg_exchange = 0;
                    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
                    curtick = xTaskGetTickCount();
                    LOG("send register:%s\r\nmsg:%s\r\n",topic_regist,pbuf);
                }
                QUEUE_FREE(pbuf);
            }
        }
        else 
        {
            if(ask_subcribe)
            {
                pbuf=QUEUE_MALLOC(256);
                if( false== subcribe_msg(&mysock,pbuf, 1))//subcribe=======================
                {

                    QUEUE_FREE(pbuf);
                    goto MQTT_reconnect;
                }

                QUEUE_FREE(pbuf);
                
                memset(sedbuf,0,MSG_TOPIC_LEN);
                memset(revtopic,0,MSG_TOPIC_LEN);
                sprintf((char*)sedbuf,"topic/%s/gateway_to_app",StuHis.StuPara.gatewayid);
                sprintf((char*)revtopic,"topic/%s/app_to_gateway",StuHis.StuPara.gatewayid);
                ask_subcribe=0;
            }

            packets=0;
            while(0!=(pbuf=queue_malloc_out(&stu_queue_malloc_tx,&len)))
            {
                buflen=len;

                ret = MQTTMsgPublish(mysock,(s8*)sedbuf,QOS2, 0,(u8*)pbuf,buflen);
                if(ret >= 0)
                {
                    //���������ݽ���
                    no_mqtt_msg_exchange = 0;
                    //��ȡ��ǰ�δ���Ϊ��������ʼʱ��
                    curtick = xTaskGetTickCount();
                    queue_malloc_delete(&stu_queue_malloc_tx,1);
					LOG(">>%s\r\n", pbuf);
                }
                else
                {
					break;
                }
                if(packets++>50)
                {
					LOG("sleep 50 ms\r\n");
					vTaskDelay(50/portTICK_RATE_MS);
					packets=0;
					break;
                }
            }
        }

        //������ҪĿ���Ƕ�ʱ�����������PING��������
        if((xTaskGetTickCount() - curtick) >=StuHis.StuPara.heart_rate_ms)
        {
            curtick = xTaskGetTickCount();
            //�ж��Ƿ������ݽ���
            if(no_mqtt_msg_exchange == 0)
            {
                //��������ݽ�������ξͲ���Ҫ����PING��Ϣ
                continue;
            }

            if(my_mqtt_send_pingreq(mysock) < 0)
            {
                //����������
                LOG("����pingʧ��....\r\n");
                goto MQTT_reconnect;
            }
              
            //�����ɹ�
            LOG("����ping��Ϊ�����ɹ�....\r\n");
            //���������ݽ���
            no_mqtt_msg_exchange = 0;
        }
    }

MQTT_reconnect:
    net_connect_flg=0;
    //�ر�����
    transport_close();
    //�������ӷ�����
    goto MQTT_START;
}


/************************************************************************
** ��������: my_mqtt_send_pingreq
** ��������: ����MQTT������
** ��ڲ���: ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
s32 my_mqtt_send_pingreq(s32 sock)
{
    s32 len;
    u8 buf[200];
    s32 buflen = sizeof(buf);
    fd_set readfd;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO(&readfd);
    FD_SET(sock,&readfd);

    len = MQTTSerialize_pingreq(buf, buflen);
    transport_sendPacketBuffer(buf, len);

    //�ȴ��ɶ��¼�
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
        return -1;

    //�пɶ��¼�
    if(FD_ISSET(sock,&readfd) == 0)
        return -2;

    if(MQTTPacket_read(buf, buflen, transport_getdata) != PINGRESP)
        return -3;

    return 0;

}

/************************************************************************
** ��������: MQTTClientInit
** ��������: ��ʼ���ͻ��˲���¼������
** ��ڲ���: s32 sock:����������
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
char fst_client_id_str[50];
s32 MQTTClientInit(s32 sock)
{
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    u8 buf[100];
    s32 buflen = sizeof(buf);
    s32 len;
    u8 sessionPresent,connack_rc;

    fd_set readfd;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO(&readfd);
    FD_SET(sock,&readfd);

    //����MQTT�ͻ������Ӳ���
    connectData.willFlag = 0;
    //MQTT�汾
    connectData.MQTTVersion = 4;
    //�ͻ���ID--����Ψһ

	element_string_get_mac((char *)buf);
	memset(fst_client_id_str, 0, sizeof(fst_client_id_str));
	strcpy (fst_client_id_str, "gateway_");
    strcat (fst_client_id_str, (char *)buf);

    connectData.clientID.cstring = fst_client_id_str;
    //������
    connectData.keepAliveInterval = KEEPLIVE_TIME;
    //�û���
    connectData.username.cstring = NULL;
    //�û�����
    connectData.password.cstring = NULL;
    //����Ự
    connectData.cleansession = 1;

    //���л�������Ϣ
    len = MQTTSerialize_connect(buf, buflen, &connectData);
    //����TCP����
	LOG("%s\r\n", fst_client_id_str);
    if(transport_sendPacketBuffer(buf, len) < 0)
    {
		ASSERT(0, 0);
        return -1;
    }

    //�ȴ��ɶ��¼�--�ȴ���ʱ
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
    {
        ASSERT(0, 0);
        return -2;
    }
    //�пɶ��¼�--û�пɶ��¼�
    if(FD_ISSET(sock,&readfd) == 0)
    {
        ASSERT(0, 0);
        return -3;
    }

    if(MQTTPacket_read(buf, buflen, transport_getdata) != CONNACK)
    {
        ASSERT(0, 0);
        return -4;
    }
    //������ӻ�Ӧ��
    if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
    {
        ASSERT(0, 0);
        return -5;
    }
    if(sessionPresent == 1)
        return 1;//����Ҫ���¶���--�������Ѿ���ס�˿ͻ��˵�״̬
    else
        return 0;//��Ҫ���¶���
}


/************************************************************************
** ��������: MQTTSubscribe
** ��������: ������Ϣ
** ��ڲ���: s32 sock���׽���
**           s8 *topic������
**           enum QoS pos����Ϣ����
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
s32 MQTTSubscribe(s32 sock,s8 *topic,enum QoS pos)
{
    static u32 PacketID = 0;
    u16 packetidbk = 0;
    s32 conutbk = 0;
    u8 buf[100];
    s32 buflen = sizeof(buf);
    MQTTString topicString = MQTTString_initializer;
    s32 len;
    s32 req_qos,qosbk;

    fd_set readfd;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO(&readfd);
    FD_SET(sock,&readfd);

    //��������
    topicString.cstring = (char *)topic;
    //��������
    req_qos = pos;

    //���л�������Ϣ
    len = MQTTSerialize_subscribe(buf, buflen, 0, PacketID++, 1, &topicString, &req_qos);
    //����TCP����
    if(transport_sendPacketBuffer(buf, len) < 0)
        return -1;

    //�ȴ��ɶ��¼�--�ȴ���ʱ
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
        return -2;
    //�пɶ��¼�--û�пɶ��¼�
    if(FD_ISSET(sock,&readfd) == 0)
        return -3;

    //�ȴ����ķ���--δ�յ����ķ���
    if(MQTTPacket_read(buf, buflen, transport_getdata) != SUBACK)
        return -4;

    //���Ļ�Ӧ��
    if(MQTTDeserialize_suback(&packetidbk,1, &conutbk, &qosbk, buf, buflen) != 1)
        return -5;

    //��ⷵ�����ݵ���ȷ��
    if((qosbk == 0x80)||(packetidbk != (PacketID-1)))
        return -6;

    //���ĳɹ�
    return 0;
}

/************************************************************************
** ��������: UserMsgCtl
** ��������: �û���Ϣ������
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg)
{
    //���ﴦ������ֻ�Ǵ�ӡ���û���������������Լ��Ĵ���ʽ
    LOG("****�յ��ͻ����Լ����ĵ���Ϣ����****\r\n");
    //���غ�����Ϣ
    switch(msg->msgqos)
    {
        case 0:
            LOG("��Ϣ������QoS0\r\n");
            break;
        case 1:
            LOG("��Ϣ������QoS1\r\n");
            break;
        case 2:
            LOG("��Ϣ������QoS2\r\n");
            break;
        default:
            LOG("�������Ϣ����\r\n");
            break;
    }
    msg->topic[MSG_TOPIC_LEN-1]=0;
    msg->msg[MSG_MAX_LEN-1]=0;
    LOG("��Ϣ���⣺%s\r\n",msg->topic);
    LOG("��Ϣ���ݣ�%s\r\n",msg->msg);
    LOG("��Ϣ���ȣ�%d\r\n",msg->msglenth);

    if(fst_gate_regster == 1)
    {
        if(0==memcmp((const char*)revtopic,(const char*)msg->topic,strlen((const char*)msg->topic)))
        {
            queue_malloc_in(&stu_queue_malloc_rx,msg->msg,msg->msglenth);
        }
        else
        {
            LOG("unknow topic:%s \r\n",msg->topic);
        }
    }
	
    else if(0==memcmp((const char*)topic_regist_ack,(const char*)msg->topic,strlen((const char*)msg->topic)))
    {
        ana_register_msg(msg->msg);
    }
    else
    {
        LOG("unknow topic:%s \r\n",msg->topic);
    }

    //���������������
    msg->valid  = 0;
}

/************************************************************************
** ��������: GetNextPackID
** ��������: ������һ�����ݰ�ID
** ��ڲ���: ��
** ���ڲ���: u16 packetid:������ID
** ��    ע:
************************************************************************/
u16 GetNextPackID(void)
{
    static u16 pubpacketid = 0;
    return pubpacketid++;
}
#include "malloc.h"
/************************************************************************
** ��������: mqtt_msg_publish
** ��������: �û�������Ϣ
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/

s32 MQTTMsgPublish(s32 sock, s8 *topic, s8 qos, s8 retained,u8* msg,u32 msg_len)
{
    u8 *buf;
    s32 ret=0;
    s32 buflen =MSG_MAX_LEN,len;
    MQTTString topicString = MQTTString_initializer;
    u16 packid = 0;


    buf=(u8*)mymalloc_mmc(MSG_MAX_LEN+1);

    //�������
    topicString.cstring = (char *)topic;

    //������ݰ�ID
    if((qos == QOS1)||(qos == QOS2))
    {
        packid = GetNextPackID();
    }
    else
    {
        qos = QOS0;
        retained = 0;
        packid = 0;
    }
    //������Ϣ
    len = MQTTSerialize_publish(buf, buflen, 0, qos, retained, packid, topicString, (unsigned char*)msg, msg_len);
    if(len <= 0)
    {
          ret =-1;
        goto END_TT;
    }
    if(transport_sendPacketBuffer(buf, len) < 0)
    {
         ret =-2;
        goto END_TT;
    }
    //�����ȼ�0������Ҫ����
    if(qos == QOS0)
    {
          ret =0;
        goto END_TT;
    }
    //�ȼ�1
    if(qos == QOS1)
    {
        //�ȴ�PUBACK
        if(WaitForPacket(sock,PUBACK,5) < 0)
        {
              ret =-3;
        goto END_TT;
        }
          ret =1;
        goto END_TT;
    }
    //�ȼ�2
    if(qos == QOS2)
    {
        //�ȴ�PUBREC
        if(WaitForPacket(sock,PUBREC,5) < 0)
        {
			LOG("PUBREC\r\n");
			ret =-3;
			goto END_TT;
        }
        //����PUBREL
        len = MQTTSerialize_pubrel(buf, buflen,0, packid);
        if(len == 0)
        {
			LOG("PUBREL\r\n");
			ret =-4;
			goto END_TT;
        }
        if(transport_sendPacketBuffer(buf, len) < 0)
        {
			LOG("\r\n");
			ret =-6;
			goto END_TT;
        }
        //�ȴ�PUBCOMP
        if(WaitForPacket(sock,PUBCOMP,5) < 0)
        {
			LOG("PUBCOMP\r\n");
			ret =-7;
			goto END_TT;
        }
        ret =2;
        goto END_TT;
    }
    //�ȼ�����
    ret=-8;
    END_TT:
    myfree_mmc(buf);
    return ret;
}

/************************************************************************
** ��������: ReadPacketTimeout
** ��������: ������ȡMQTT����
** ��ڲ���: s32 sock:����������
**           u8 *buf:���ݻ�����
**           s32 buflen:��������С
**           u32 timeout:��ʱʱ��--0-��ʾֱ�Ӳ�ѯ��û��������������
** ���ڲ���: -1������,����--������
** ��    ע:
************************************************************************/
s32 ReadPacketTimeout(s32 sock,u8 *buf,s32 buflen,u32 timeout)
{
    fd_set readfd;
    struct timeval tv;

    if(timeout != 0)
    {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&readfd);
        FD_SET(sock,&readfd);

        //�ȴ��ɶ��¼�--�ȴ���ʱ
        if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
            return -1;
        //�пɶ��¼�--û�пɶ��¼�
        if(FD_ISSET(sock,&readfd) == 0)
            return -1;
    }
    //��ȡTCP/IP�¼�
    return MQTTPacket_read(buf, buflen, transport_getdata);
}


/************************************************************************
** ��������: deliverMessage
** ��������: ���ܷ�������������Ϣ
** ��ڲ���: MQTTMessage *msg:MQTT��Ϣ�ṹ��
**           MQTT_USER_MSG *mqtt_user_msg:�û����ܽṹ��
**           MQTTString  *TopicName:����
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void deliverMessage(MQTTString  *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg)
{
    //��Ϣ����
    mqtt_user_msg->msgqos = msg->qos;
    //������Ϣ
    memcpy(mqtt_user_msg->msg,msg->payload,msg->payloadlen);
    mqtt_user_msg->msg[msg->payloadlen] = 0;
    //������Ϣ����
    mqtt_user_msg->msglenth = msg->payloadlen;
    //��Ϣ����
    memcpy((char *)mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len);
    mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
    //��ϢID
    mqtt_user_msg->packetid = msg->id;
    //������Ϣ�Ϸ�
    mqtt_user_msg->valid = 1;
}


/************************************************************************
** ��������: mqtt_pktype_ctl
** ��������: ���ݰ����ͽ��д���
** ��ڲ���: u8 packtype:������
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void mqtt_pktype_ctl(u8 packtype,u8 *buf,u32 buflen)
{
    MQTTMessage msg;
    s32 rc;
    MQTTString receivedTopic;
    u32 len;
    switch(packtype)
    {
        case PUBLISH:
            //����PUBLISH��Ϣ
            if(MQTTDeserialize_publish(&msg.dup,(int*)&msg.qos, &msg.retained, &msg.id, &receivedTopic,(unsigned char **)&msg.payload, &msg.payloadlen, buf, buflen) != 1)
                return;
            //������Ϣ
            deliverMessage(&receivedTopic,&msg,&mqtt_user_msg);

            //��Ϣ������ͬ������ͬ
            if(msg.qos == QOS0)
            {
                //QOS0-����ҪACK
                //ֱ�Ӵ�������
                UserMsgCtl(&mqtt_user_msg);
                return;
            }
            //����PUBACK��Ϣ
            if(msg.qos == QOS1)
            {
                len =MQTTSerialize_puback(buf,buflen,mqtt_user_msg.packetid);
                if(len == 0)
                    return;
                //���ͷ���
                if(transport_sendPacketBuffer(buf,len)<0)
                    return;
                //���غ�����Ϣ
                UserMsgCtl(&mqtt_user_msg);
                return;
            }

            //��������2,ֻ��Ҫ����PUBREC�Ϳ�����
            if(msg.qos == QOS2)
            {
                len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, mqtt_user_msg.packetid);
                if(len == 0)
                    return;
                //���ͷ���
                transport_sendPacketBuffer(buf,len);
            }
            break;
        case  PUBREL:
            //���������ݣ������ID��ͬ�ſ���
            rc = MQTTDeserialize_ack(&msg.type,&msg.dup, &msg.id, buf,buflen);
            if((rc != 1)||(msg.type != PUBREL)||(msg.id != mqtt_user_msg.packetid))
                return ;
            //�յ�PUBREL����Ҫ������������
            if(mqtt_user_msg.valid == 1)
            {
                //���غ�����Ϣ
                UserMsgCtl(&mqtt_user_msg);
            }
            //���л�PUBCMP��Ϣ
            len = MQTTSerialize_pubcomp(buf,buflen,msg.id);
            if(len == 0)
                return;
            //���ͷ���--PUBCOMP
            transport_sendPacketBuffer(buf,len);
            break;
        case   PUBACK://�ȼ�1�ͻ����������ݺ󣬷���������
            break;
        case   PUBREC://�ȼ�2�ͻ����������ݺ󣬷���������
            break;
        case   PUBCOMP://�ȼ�2�ͻ�������PUBREL�󣬷���������
            break;
        default:
            break;
    }
}

/************************************************************************
** ��������: WaitForPacket
** ��������: �ȴ��ض������ݰ�
** ��ڲ���: s32 sock:����������
**           u8 packettype:������
**           u8 times:�ȴ�����
** ���ڲ���: >=0:�ȵ����ض��İ� <0:û�еȵ��ض��İ�
** ��    ע:
************************************************************************/
s32 WaitForPacket(s32 sock,u8 packettype,u8 times)
{
    s32 type;
    u8 *buf;
    u8 n = 0;
    s32 buflen = MSG_MAX_LEN;
    buf=(u8*)mymalloc_mmc(MSG_MAX_LEN+1);

    do
    {
        //��ȡ���ݰ�
        type = ReadPacketTimeout(sock,buf,buflen,2);
        if(type != -1)
            mqtt_pktype_ctl(type,buf,buflen);
        n++;
    }
    while((type != packettype)&&(n < times));

    myfree_mmc(buf);
    //�յ������İ�
    if(type == packettype)
        return 0;
    else
        return -1;
}
