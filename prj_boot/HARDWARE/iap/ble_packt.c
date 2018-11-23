#include "ble_packt.h"


void ble_send_packetdata(uint8_t cmd,uint8_t *data,uint16_t len)
{
    uint8_t outbuf[500];
    uint16_t outlen,send_len;
    uint8_t packetlen;

    outlen=ble_packet_data(cmd,data,len,outbuf);

    send_len=0;
    while(outlen-send_len)
    {
        if( (outlen-send_len)>=20)
        {
            packetlen=20;
        } else
        {
            packetlen=outlen-send_len;
        }

//        BleWriteData(outbuf+send_len,packetlen);
        send_len+=packetlen;

    }

}

static void ble_send_ack(uint8_t askcmd,uint8_t result)
{

    uint8_t indata[2];
    indata[0]=askcmd;
    indata[1]=result;
    ble_send_packetdata(0x01,indata,2);
}
extern uint8_t send_mqtt_flg;


//StuData.mqttid
//void fill_default_mqtt_data_and_send(void)
//{

//    uint8_t len;

//    //host
//    memset(StuData.mqtt_host,0,HOST_LEN);
//    memcpy(StuData.mqtt_host,MQTT_HOST,strlen((const char*)MQTT_HOST));

//    //port
//    StuData.mqtt_port=MQTT_PORT;

//    //id
//    memset(StuData.mqtt_device_id,0,DEVICE_ID_LEN);
//    len=strlen((const char*)MQTT_CLEAN_ID_FIXED);
//    memcpy(StuData.mqtt_device_id,MQTT_CLEAN_ID_FIXED,len);
//    memcpy(StuData.mqtt_device_id+len,StuData.mqttid,strlen((const char*)StuData.mqttid));


//    //ssl
//    StuData.mqtt_ssl=1;


//    //user
//    memset(StuData.mqtt_user,0,USER_LEN);
//    memcpy(StuData.mqtt_user,MQTT_DEFAULT_USER,strlen((const char*)MQTT_DEFAULT_USER));

//    //pwd
//    memset(StuData.mqtt_pass,0,PWD_LEN);
//    memcpy(StuData.mqtt_pass,MQTT_DEFAULT_PWD,strlen((const char*)MQTT_DEFAULT_PWD));

//    send_mqtt_flg=1;
//}


static void ble_ana_packet_out(uint8_t cmd,uint8_t dataslen,uint8_t*datas)
{
    uint8_t buf[17];
    uint8_t len;
//    uint8_t *pdata;
    switch(cmd)
    {
    case 0x82: 
        ble_send_ack(cmd,0);
        break;
    case 0x83:


        ble_send_packetdata(0x02,buf,len);
        break;
    case 0x84:
  
        break;

    }

}

/*
pata:指向来自ble的数据
datalen：指向来自ble的数据长度

ble_ana_packet_out(uint8_t cmd,uint8_t dataslen,uint8_t*datas)对应于解析完成的数据的
*/
void ble_unpack_data(uint8_t *pata,uint8_t datalen)
{
    static uint8_t buf[UART_FRAM_LEN];
    static uint16_t buflen;

    if(pata[0]!=0xaa) return;
    pata++;
    datalen-=3;



    if(pata[1]==1)
    {
        buflen=0;
    }
    memcpy(&buf[buflen],pata+2,datalen);
    buflen+=datalen;
    if(pata[0]==pata[1])
    {
        if(buflen==(buf[1]+2))
            ble_ana_packet_out(buf[0],buf[1],buf+2);
    }


}

/*
cmd:需要发送的命令
pdata:指向需要发送的数据部分
datalen:指向需要发送数据部分的长度
pout:指向打包好的数据
返回值:返回打包好的数据的长度
*/
uint8_t ble_packet_data(uint8_t cmd,uint8_t*pdata,uint8_t datalen,uint8_t*pout)
{
    uint8_t packets,i,outlen=0,leftlen,len;
    uint8_t *p;
    packets=(2+datalen)/17;
    if((2+datalen)%17)
    {
        packets++;
    }
    leftlen=2+datalen;
    p=pdata;
    for(i=0; i<packets; i++)
    {

        pout[outlen++]=0xaa;
        pout[outlen++]=packets;
        pout[outlen++]=i+1;

        if(0==i)
        {
            pout[outlen++]=cmd;
            leftlen--;
            pout[outlen++]=datalen;
            leftlen--;
            if(leftlen>15)
            {
                len=15;
            } else
            {
                len=leftlen;
            }
            memcpy(&pout[outlen],p,len);
            outlen+=len;
            p+=len;
            leftlen-=len;
        } else
        {
            if(leftlen>17)
            {
                len=17;
            } else
            {
                len=leftlen;
            }
            memcpy(&pout[outlen],p,len);
            outlen+=len;
            p+=len;
            leftlen-=len;
        }
    }


    return outlen;


}


