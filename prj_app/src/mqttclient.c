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
    //创建网络连接
    LOG("start\r\n");
    LOG("remote ip：%s,port:%0d\r\n",StuHis.StuPara.remote_ip,StuHis.StuPara.remote_port);

    while(1)
    {
        if(net_insert_status)
        {
            *pmysock = transport_open((s8*)StuHis.StuPara.remote_ip,StuHis.StuPara.remote_port);
            if(*pmysock >= 0)
            {
                LOG("conected\r\n");
                break;
            }
            else
            {
                LOG("connected err,wait 5secr\n");
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
				//重连服务器
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
				//重连服务器
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



//定义用户消息结构体
MQTT_USER_MSG  mqtt_user_msg;
uint8_t revtopic[MSG_TOPIC_LEN];







void deliverMessage(MQTTString *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg);

/************************************************************************
** 函数名称: mqtt_thread
** 函数功能: MQTT任务
** 入口参数: void *pvParameters：任务参数
** 出口参数: 无
** 备    注:
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

    //获取当前滴答，作为心跳包起始时间
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
//表明无数据交换
        no_mqtt_msg_exchange = 1;//read msg=======================================

//推送消息
        FD_ZERO(&readfd);
        FD_SET(mysock,&readfd);

//等待可读事件
		tv.tv_sec = 0;
		tv.tv_usec = 300;
        
        select(mysock+1,&readfd,NULL,NULL,&tv);

//判断MQTT服务器是否有数据
        if(FD_ISSET(mysock,&readfd) != 0)
        {
            pbuf=QUEUE_MALLOC(MSG_MAX_LEN);
            //读取数据包--注意这里参数为0，不阻塞
            type = ReadPacketTimeout(mysock,pbuf,MSG_MAX_LEN,0);
            if(type != -1)
            {
                LOG("type:%d\r\n",type);
                mqtt_pktype_ctl(type,pbuf,MSG_MAX_LEN);

                //表明有数据交换
                no_mqtt_msg_exchange = 0;
                //获取当前滴答，作为心跳包起始时间
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
                    //表明有数据交换
                    no_mqtt_msg_exchange = 0;
                    //获取当前滴答，作为心跳包起始时间
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
                    //表明有数据交换
                    no_mqtt_msg_exchange = 0;
                    //获取当前滴答，作为心跳包起始时间
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

        //这里主要目的是定时向服务器发送PING保活命令
        if((xTaskGetTickCount() - curtick) >=StuHis.StuPara.heart_rate_ms)
        {
            curtick = xTaskGetTickCount();
            //判断是否有数据交换
            if(no_mqtt_msg_exchange == 0)
            {
                //如果有数据交换，这次就不需要发送PING消息
                continue;
            }

            if(my_mqtt_send_pingreq(mysock) < 0)
            {
                //重连服务器
                LOG("发送ping失败....\r\n");
                goto MQTT_reconnect;
            }
              
            //心跳成功
            LOG("发送ping作为心跳成功....\r\n");
            //表明有数据交换
            no_mqtt_msg_exchange = 0;
        }
    }

MQTT_reconnect:
    net_connect_flg=0;
    //关闭链接
    transport_close();
    //重新链接服务器
    goto MQTT_START;
}


/************************************************************************
** 函数名称: my_mqtt_send_pingreq
** 函数功能: 发送MQTT心跳包
** 入口参数: 无
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
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

    //等待可读事件
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
        return -1;

    //有可读事件
    if(FD_ISSET(sock,&readfd) == 0)
        return -2;

    if(MQTTPacket_read(buf, buflen, transport_getdata) != PINGRESP)
        return -3;

    return 0;

}

/************************************************************************
** 函数名称: MQTTClientInit
** 函数功能: 初始化客户端并登录服务器
** 入口参数: s32 sock:网络描述符
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
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

    //创建MQTT客户端连接参数
    connectData.willFlag = 0;
    //MQTT版本
    connectData.MQTTVersion = 4;
    //客户端ID--必须唯一

	element_string_get_mac((char *)buf);
	memset(fst_client_id_str, 0, sizeof(fst_client_id_str));
	strcpy (fst_client_id_str, "gateway_");
    strcat (fst_client_id_str, (char *)buf);

    connectData.clientID.cstring = fst_client_id_str;
    //保活间隔
    connectData.keepAliveInterval = KEEPLIVE_TIME;
    //用户名
    connectData.username.cstring = NULL;
    //用户密码
    connectData.password.cstring = NULL;
    //清除会话
    connectData.cleansession = 1;

    //串行化连接消息
    len = MQTTSerialize_connect(buf, buflen, &connectData);
    //发送TCP数据
	LOG("%s\r\n", fst_client_id_str);
    if(transport_sendPacketBuffer(buf, len) < 0)
    {
		ASSERT(0, 0);
        return -1;
    }

    //等待可读事件--等待超时
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
    {
        ASSERT(0, 0);
        return -2;
    }
    //有可读事件--没有可读事件
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
    //拆解连接回应包
    if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
    {
        ASSERT(0, 0);
        return -5;
    }
    if(sessionPresent == 1)
        return 1;//不需要重新订阅--服务器已经记住了客户端的状态
    else
        return 0;//需要重新订阅
}


/************************************************************************
** 函数名称: MQTTSubscribe
** 函数功能: 订阅消息
** 入口参数: s32 sock：套接字
**           s8 *topic：主题
**           enum QoS pos：消息质量
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
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

    //复制主题
    topicString.cstring = (char *)topic;
    //订阅质量
    req_qos = pos;

    //串行化订阅消息
    len = MQTTSerialize_subscribe(buf, buflen, 0, PacketID++, 1, &topicString, &req_qos);
    //发送TCP数据
    if(transport_sendPacketBuffer(buf, len) < 0)
        return -1;

    //等待可读事件--等待超时
    if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
        return -2;
    //有可读事件--没有可读事件
    if(FD_ISSET(sock,&readfd) == 0)
        return -3;

    //等待订阅返回--未收到订阅返回
    if(MQTTPacket_read(buf, buflen, transport_getdata) != SUBACK)
        return -4;

    //拆订阅回应包
    if(MQTTDeserialize_suback(&packetidbk,1, &conutbk, &qosbk, buf, buflen) != 1)
        return -5;

    //检测返回数据的正确性
    if((qosbk == 0x80)||(packetidbk != (PacketID-1)))
        return -6;

    //订阅成功
    return 0;
}

/************************************************************************
** 函数名称: UserMsgCtl
** 函数功能: 用户消息处理函数
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: 无
** 备    注:
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg)
{
    //这里处理数据只是打印，用户可以在这里添加自己的处理方式
    LOG("****收到客户端自己订阅的消息！！****\r\n");
    //返回后处理消息
    switch(msg->msgqos)
    {
        case 0:
            LOG("消息质量：QoS0\r\n");
            break;
        case 1:
            LOG("消息质量：QoS1\r\n");
            break;
        case 2:
            LOG("消息质量：QoS2\r\n");
            break;
        default:
            LOG("错误的消息质量\r\n");
            break;
    }
    msg->topic[MSG_TOPIC_LEN-1]=0;
    msg->msg[MSG_MAX_LEN-1]=0;
    LOG("消息主题：%s\r\n",msg->topic);
    LOG("消息类容：%s\r\n",msg->msg);
    LOG("消息长度：%d\r\n",msg->msglenth);

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

    //处理完后销毁数据
    msg->valid  = 0;
}

/************************************************************************
** 函数名称: GetNextPackID
** 函数功能: 产生下一个数据包ID
** 入口参数: 无
** 出口参数: u16 packetid:产生的ID
** 备    注:
************************************************************************/
u16 GetNextPackID(void)
{
    static u16 pubpacketid = 0;
    return pubpacketid++;
}
#include "malloc.h"
/************************************************************************
** 函数名称: mqtt_msg_publish
** 函数功能: 用户推送消息
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注:
************************************************************************/

s32 MQTTMsgPublish(s32 sock, s8 *topic, s8 qos, s8 retained,u8* msg,u32 msg_len)
{
    u8 *buf;
    s32 ret=0;
    s32 buflen =MSG_MAX_LEN,len;
    MQTTString topicString = MQTTString_initializer;
    u16 packid = 0;


    buf=(u8*)mymalloc_mmc(MSG_MAX_LEN+1);

    //填充主题
    topicString.cstring = (char *)topic;

    //填充数据包ID
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
    //推送消息
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
    //质量等级0，不需要返回
    if(qos == QOS0)
    {
          ret =0;
        goto END_TT;
    }
    //等级1
    if(qos == QOS1)
    {
        //等待PUBACK
        if(WaitForPacket(sock,PUBACK,5) < 0)
        {
              ret =-3;
        goto END_TT;
        }
          ret =1;
        goto END_TT;
    }
    //等级2
    if(qos == QOS2)
    {
        //等待PUBREC
        if(WaitForPacket(sock,PUBREC,5) < 0)
        {
			LOG("PUBREC\r\n");
			ret =-3;
			goto END_TT;
        }
        //发送PUBREL
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
        //等待PUBCOMP
        if(WaitForPacket(sock,PUBCOMP,5) < 0)
        {
			LOG("PUBCOMP\r\n");
			ret =-7;
			goto END_TT;
        }
        ret =2;
        goto END_TT;
    }
    //等级错误
    ret=-8;
    END_TT:
    myfree_mmc(buf);
    return ret;
}

/************************************************************************
** 函数名称: ReadPacketTimeout
** 函数功能: 阻塞读取MQTT数据
** 入口参数: s32 sock:网络描述符
**           u8 *buf:数据缓存区
**           s32 buflen:缓冲区大小
**           u32 timeout:超时时间--0-表示直接查询，没有数据立即返回
** 出口参数: -1：错误,其他--包类型
** 备    注:
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

        //等待可读事件--等待超时
        if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
            return -1;
        //有可读事件--没有可读事件
        if(FD_ISSET(sock,&readfd) == 0)
            return -1;
    }
    //读取TCP/IP事件
    return MQTTPacket_read(buf, buflen, transport_getdata);
}


/************************************************************************
** 函数名称: deliverMessage
** 函数功能: 接受服务器发来的消息
** 入口参数: MQTTMessage *msg:MQTT消息结构体
**           MQTT_USER_MSG *mqtt_user_msg:用户接受结构体
**           MQTTString  *TopicName:主题
** 出口参数: 无
** 备    注:
************************************************************************/
void deliverMessage(MQTTString  *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg)
{
    //消息质量
    mqtt_user_msg->msgqos = msg->qos;
    //保存消息
    memcpy(mqtt_user_msg->msg,msg->payload,msg->payloadlen);
    mqtt_user_msg->msg[msg->payloadlen] = 0;
    //保存消息长度
    mqtt_user_msg->msglenth = msg->payloadlen;
    //消息主题
    memcpy((char *)mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len);
    mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
    //消息ID
    mqtt_user_msg->packetid = msg->id;
    //标明消息合法
    mqtt_user_msg->valid = 1;
}


/************************************************************************
** 函数名称: mqtt_pktype_ctl
** 函数功能: 根据包类型进行处理
** 入口参数: u8 packtype:包类型
** 出口参数: 无
** 备    注:
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
            //拆析PUBLISH消息
            if(MQTTDeserialize_publish(&msg.dup,(int*)&msg.qos, &msg.retained, &msg.id, &receivedTopic,(unsigned char **)&msg.payload, &msg.payloadlen, buf, buflen) != 1)
                return;
            //接受消息
            deliverMessage(&receivedTopic,&msg,&mqtt_user_msg);

            //消息质量不同，处理不同
            if(msg.qos == QOS0)
            {
                //QOS0-不需要ACK
                //直接处理数据
                UserMsgCtl(&mqtt_user_msg);
                return;
            }
            //发送PUBACK消息
            if(msg.qos == QOS1)
            {
                len =MQTTSerialize_puback(buf,buflen,mqtt_user_msg.packetid);
                if(len == 0)
                    return;
                //发送返回
                if(transport_sendPacketBuffer(buf,len)<0)
                    return;
                //返回后处理消息
                UserMsgCtl(&mqtt_user_msg);
                return;
            }

            //对于质量2,只需要发送PUBREC就可以了
            if(msg.qos == QOS2)
            {
                len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, mqtt_user_msg.packetid);
                if(len == 0)
                    return;
                //发送返回
                transport_sendPacketBuffer(buf,len);
            }
            break;
        case  PUBREL:
            //解析包数据，必须包ID相同才可以
            rc = MQTTDeserialize_ack(&msg.type,&msg.dup, &msg.id, buf,buflen);
            if((rc != 1)||(msg.type != PUBREL)||(msg.id != mqtt_user_msg.packetid))
                return ;
            //收到PUBREL，需要处理并抛弃数据
            if(mqtt_user_msg.valid == 1)
            {
                //返回后处理消息
                UserMsgCtl(&mqtt_user_msg);
            }
            //串行化PUBCMP消息
            len = MQTTSerialize_pubcomp(buf,buflen,msg.id);
            if(len == 0)
                return;
            //发送返回--PUBCOMP
            transport_sendPacketBuffer(buf,len);
            break;
        case   PUBACK://等级1客户端推送数据后，服务器返回
            break;
        case   PUBREC://等级2客户端推送数据后，服务器返回
            break;
        case   PUBCOMP://等级2客户端推送PUBREL后，服务器返回
            break;
        default:
            break;
    }
}

/************************************************************************
** 函数名称: WaitForPacket
** 函数功能: 等待特定的数据包
** 入口参数: s32 sock:网络描述符
**           u8 packettype:包类型
**           u8 times:等待次数
** 出口参数: >=0:等到了特定的包 <0:没有等到特定的包
** 备    注:
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
        //读取数据包
        type = ReadPacketTimeout(sock,buf,buflen,2);
        if(type != -1)
            mqtt_pktype_ctl(type,buf,buflen);
        n++;
    }
    while((type != packettype)&&(n < times));

    myfree_mmc(buf);
    //收到期望的包
    if(type == packettype)
        return 0;
    else
        return -1;
}
