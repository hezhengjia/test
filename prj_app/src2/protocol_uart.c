#include "in_flash_manage.h"
#include "protocol_uart.h"
#include "stmflash.h"
#include "uart.h"
#include "main.h"
#include "rtc.h"
bool iap_flag = false;
uint32_t file_size;
uint32_t file_Offset;
uint16_t file_crc;
uint8_t file_cmd;
uint8_t iap_buf[UPGRADE_PACKET_SIZE+16];
uint16_t iap_len = 0;
uint32_t g_ticktime_50ms = 0;


uint16_t CalcCrc16(uint16_t crc,const uint8_t* pchMsg, uint32_t wDataLen)
{
    uint32_t i;
    uint8_t j;
    uint16_t c;
    for (i=0; i<wDataLen; i++)
    {
        c = *(pchMsg+i) & 0x00FF;
        crc^=c;
        for (j=0; j<8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    crc = (crc>>8) + (crc<<8);
    return(crc);
}
bool GetFileCrc(uint16_t crcin,uint16_t* crcout,uint8_t *buf,uint32_t len)
{
    uint16_t tmpcrc=0xffff;

    crcout[0]=CalcCrc16(tmpcrc,(const uint8_t *)APP_BVK_ADDR,file_size);
    if(crcin==crcout[0])
        return true;

    return false;

}

void WriteUpflg(uint8_t* datain,uint8_t len)
{
    uint8_t buf[12];
    memset(buf,0,12);
    buf[0] = 0xaa;
    buf[1] = 0x55;
    STMFLASH_Erase(ADDR_FLASH_SECTOR_1);

    STMFLASH_Write(BOOTLOADER_PARA_ADDR,(uint32_t*)buf,3);
}
void CleanUpflg(void)
{
    STMFLASH_Erase(ADDR_FLASH_SECTOR_1);
}
bool IsNeedUpdata(void)
{
    uint8_t* pdata;
    pdata=(uint8_t*)(   BOOTLOADER_PARA_ADDR);

    if((pdata[0]!=0xaa)||(pdata[1]!=0x55))
    {
        return false;
    }
    return true;
}
//__asm void SystemReset(void)
//{
// MOV R0, #1           //;
// MSR FAULTMASK, R0    //; 清除FAULTMASK 禁止一切中断产生
// LDR R0, =0xE000ED0C  //;
// LDR R1, =0x05FA0004  //;
// STR R1, [R0]         //; 系统软件复位
//
//deadloop
//    B deadloop        //; 死循环使程序运行不到下面的代码
//}

void WriteFlgBootSys(void)
{
//  __set_FAULTMASK(1);
//  NVIC_SystemReset();
//    SystemReset();

//    __ASM("MOV R0, #1");
//    __ASM("MSR FAULTMASK, R0");
//    SCB->AIRCR = 0x05FA0004;
//    for(;;);
    __set_BASEPRI(1);
    NVIC_SystemReset();
	while(1);
}
static void WriteBvkFlash(uint32_t* buf,uint32_t numOfDword)
{
    STMFLASH_Write(APP_BVK_ADDR+file_Offset,(uint32_t*)buf,numOfDword);
    file_Offset+=(numOfDword*4);
}
static bool BvkAppWriteApp(void)
{

    uint32_t readtimes,i;
    uint32_t *pdatasrc;
    uint16_t tmpcrc=0xffff,crcout;


    pdatasrc=(uint32_t *)APP_BVK_ADDR;

    STMFLASH_Erase(ADDR_FLASH_SECTOR_5);
    STMFLASH_Erase(ADDR_FLASH_SECTOR_6);
    STMFLASH_Erase(ADDR_FLASH_SECTOR_7);

    readtimes=file_size/4;
    if(file_size%4)
        readtimes++;
    for(i=0; i<readtimes; i++)
    {
        STMFLASH_Write((APP_ADDR+4*i),&pdatasrc[i],1);//flash_nvic_word_write((uint32_t *)(APP_ADDR+4*i),pdatasrc[i]);
    }

    crcout=CalcCrc16(tmpcrc,(const uint8_t *)APP_ADDR,file_size);
    if(file_crc==crcout)
        return true;
    else
        return false;
}
static bool EraseBvkFlash(uint32_t size)
{
    STMFLASH_Erase(ADDR_FLASH_SECTOR_8);
    STMFLASH_Erase(ADDR_FLASH_SECTOR_9);
    STMFLASH_Erase(ADDR_FLASH_SECTOR_10);
    return true;
}

static uint8_t keytemp=255;
uint8_t get_key(void)
{
    return keytemp;
}


static void send_ack(uint16_t cmd,uint16_t answer_cmd,uint8_t *pdata,uint16_t data_len)
{

    uint8_t outbuf[150];
    uint16_t len;
    uint8_t sendbuf[150],sendlen=0;

    sendbuf[0] = get_key();
    sendlen++;


    sendbuf[sendlen++]=answer_cmd;
    memcpy(&sendbuf[sendlen],pdata,data_len);
    sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}



static void send_data(uint16_t cmd,uint8_t *pdata,uint16_t data_len)
{


    uint8_t outbuf[150];
    uint16_t len;
    uint8_t sendbuf[150],sendlen=0;

    sendbuf[0] = get_key();
    sendlen++;

    memcpy(&sendbuf[sendlen],pdata,data_len);
    sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);
}
static void RfRevFileSendResult(uint8_t result)
{


    uint8_t sendbuf[4],sendlen;
    uint8_t outbuf[10],len;

    sendlen = 0;
    sendbuf[sendlen++] = result;

    len=uart_package_data(MCU_UPFILE_RESULT,sendbuf,sendlen,outbuf);
    uart_sent_bytes(outbuf,len);
}
static void AskPacket(uint32_t packet)
{
    uint8_t sendbuf[4],sendlen;
    uint8_t outbuf[10],len;

    sendlen = 0;
    sendbuf[sendlen++] = packet>>8;
    sendbuf[sendlen++] = packet;

    len=uart_package_data(MCU_ASK_PACKET,sendbuf,sendlen,outbuf);
    uart_sent_bytes(outbuf,len);

}

static bool RfWritePacket(uint8_t*rev_buf,uint32_t maxbuf,uint32_t *rev_lenth,uint32_t *packet )
{
    uint8_t write_all,ask_flg=0;
    write_all=false;
    memcpy(&rev_buf[rev_lenth[0]],&iap_buf[2],UPGRADE_PACKET_SIZE);
    rev_lenth[0]+=UPGRADE_PACKET_SIZE;
    if((file_Offset+rev_lenth[0])<file_size)
    {
        ask_flg=1;

    }
    if(rev_lenth[0]>=maxbuf)
    {
        WriteBvkFlash((uint32_t*)rev_buf,maxbuf/4);
        rev_lenth[0]-=maxbuf;
    }
    if((file_Offset+rev_lenth[0])>=file_size)
    {
        if(rev_lenth[0])
        {

            WriteBvkFlash((uint32_t*)rev_buf,rev_lenth[0]/4);
            rev_lenth[0]=0;
        }
        write_all=true;
    }
    if(ask_flg)
    {
        packet[0]++;
        AskPacket(packet[0]);
    }

    return write_all;
}

void task_uart_upfile(void)
{

#define TIME_OUT_1000_MS   (1000/20)
#define MAX_REV_BUF_LEN         256

#define STA_SEND_RESULT 0x01
#define STA_ASK_DATA 0x00

    uint8_t data,i;
    bool crc_ok=false;
    uint16_t crcout;
    uint32_t packet=0,revpacket;
    uint32_t rev_lenth = 0;

    uint16_t tickout1000ms=0;

    uint8_t sta=STA_ASK_DATA;

    uint8_t rev_buf[MAX_REV_BUF_LEN+16];


    while(1)
    {
        for(i=0; i<3; i++)
        {
            file_cmd = 0;
            send_data(MCU_ASK_UPFILE_PAR,rev_buf,0);
            delay_ms(15);
            while ( true==queue_byte_out(&uart_queue,&data) )
            {
                uart_unpackage_data(data,task_protocol_dataout_fun);
            }
            if (file_cmd == UPFILE_PAR)
            {
                break;
            }

        }
        if (file_cmd != UPFILE_PAR)
        {
//          StuHis.StuPara.iap_flag = false;
            AnsyHis();
            return;
        }
        else
        {
            break;
        }

    }
    iap_len = 0;
    rev_lenth = 0;
    packet = 0;
    AskPacket(packet);
    EraseBvkFlash(0);
    while(1)
    {
        while ( true==queue_byte_out(&uart_queue,&data) )
        {
            uart_unpackage_data(data,task_protocol_dataout_fun);
        }

        if(iap_len)
        {
            iap_len = 0;
            if(file_cmd==UPFILE_DATA)
            {
                file_cmd = 0;
                tickout1000ms=0;
                revpacket = iap_buf[0];
                revpacket = (revpacket<<8)+iap_buf[1];
                if((revpacket&0xffff)==(packet&0xffff))
                {
//                  if((packet==64)&& (true!=IsRightBoard((char*)&rfidRev[3])))
//                  {
//                      RfRevFileSendResult(1);
//                      iap_flag = false;
//                      return;
//                  }

                    if(true==(RfWritePacket(rev_buf,MAX_REV_BUF_LEN,&rev_lenth,&packet)))
                    {
                        crc_ok=GetFileCrc(file_crc,&crcout,rev_buf,file_size);
                        if(true==crc_ok)
                        {
                            RfRevFileSendResult(0);

                            if (true == BvkAppWriteApp())
                            {
                                RfRevFileSendResult(0);
                                CleanUpflg();
                                WriteFlgBootSys();
                            }
                            else
                            {
                                RfRevFileSendResult(1);
                                return;
                            }

                        }
                        else
                        {
                            RfRevFileSendResult(1);
//                          StuHis.StuPara.iap_flag = false;
                            AnsyHis();
                            return;
                        }
                    }
                }
            }
        }

        //V_FeedWdog();


        if(g_ticktime_50ms == 0)
        {
            continue;
        }
        g_ticktime_50ms = 0;

        if(sta==STA_ASK_DATA)
        {
            AskPacket(packet);
        }
        if(tickout1000ms++>TIME_OUT_1000_MS)
        {
            if(true==crc_ok)
            {
                WriteFlgBootSys();
            }
//          StuHis.StuPara.iap_flag = false;
            AnsyHis();
            return;
        }
    }


}

void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdatas,uint16_t datalen);
static uint8_t cover_data(uint8_t indata,uint8_t*out_data)
{
    uint8_t  len=0;
    if(0xaa==indata)
    {
        out_data[len++]=0xab;
        out_data[len++]=0x01;

    }
    else if(0xab==indata)
    {
        out_data[len++]=0xab;
        out_data[len++]=0x02;
    }
    else
    {
        out_data[len++]=indata;
    }
    return len;
}


static uint8_t uncover_data(uint8_t predata,uint8_t nowdata)
{
    uint8_t r_value=0xff;
    r_value= nowdata;
    if(0xab==predata)
    {
        if(0x01==nowdata)
        {
            r_value= 0xaa;
        }
        else if(0x02==nowdata)
        {
            r_value= 0xab;
        }
        else
        {
            r_value= nowdata;//error=========================
        }
    }

    return r_value;
}
uint16_t uart_package_data(uint16_t cmd,uint8_t* p_in_datas,uint16_t datalen,uint8_t*p_out_datas)
{
    uint16_t len=0,i;
    uint8_t xor_result=0,tmp;
    //head
    xor_result^=0xaa;
    p_out_datas[len++]=0xaa;

#ifdef EN_MCU_USE_TIME_CONTROL
    if(0==(cmd&0xff00))
    {
        //cmd
        tmp=cmd>>8;
        xor_result^=tmp;
        p_out_datas[len++]=tmp;

        tmp=cmd;
        xor_result^=tmp;
        p_out_datas[len++]=tmp;

        //datalen
        tmp=datalen>>8;
        xor_result^=tmp;
        p_out_datas[len++]=tmp;

        tmp=datalen;
        xor_result^=tmp;
        p_out_datas[len++]=tmp;

        //datas
        if(datalen)
        {
            for(i=0; i<datalen; i++)
            {
                tmp=p_in_datas[i];
                xor_result^=tmp;
                p_out_datas[len++]=tmp;
            }
        }

        //xor
        tmp=xor_result;
        p_out_datas[len++]=tmp;

        return len;
    }
#endif
    //cmd
    tmp=cmd>>8;
    xor_result^=tmp;
    len+=cover_data(tmp,&p_out_datas[len]);
    tmp=cmd;
    xor_result^=tmp;
    len+=cover_data(tmp,&p_out_datas[len]);

    //datalen
    tmp=datalen>>8;
    xor_result^=tmp;
    len+=cover_data(tmp,&p_out_datas[len]);
    tmp=datalen;
    xor_result^=tmp;
    len+=cover_data(tmp,&p_out_datas[len]);

    //datas
    if(datalen)
    {
        for(i=0; i<datalen; i++)
        {
            tmp=p_in_datas[i];
            xor_result^=tmp;
            len+=cover_data(tmp,&p_out_datas[len]);

        }


    }

    //xor
    tmp=xor_result;
    len+=cover_data(tmp,&p_out_datas[len]);

    return len;


}

bool uart_unpackage_data(uint8_t datain,dataout_fun call_util_get_data)
{

    static uint8_t sta,pre;
    static uint8_t buf[UART_FRAM_LEN+5];//cmd len datas.. xor
    static uint16_t buf_len,left;
    static uint8_t data_xor;
    uint8_t tmp;
    uint16_t tmp16;
//head
    if(datain == 0xaa)
    {
        data_xor=0;
        pre=datain;
        data_xor^=datain;
        buf_len=0;
        sta=1;
        return false;
    }

    switch(sta)
    {

    case 1://cmd
    case 2://cmd
    case 3://lenth
    case 4://lenth
        if(datain == 0xab)
        {
            break;
        }
        else
        {
            tmp=uncover_data(pre,datain);
            data_xor^=tmp;
            buf[buf_len++]=tmp;
            sta++;
            if(5==sta)//判断长度是否合法
            {
                tmp16=buf[2];
                tmp16<<=8;
                tmp16|=buf[3];
                left=tmp16;
                if(tmp16>(UART_FRAM_LEN))//长度错误
                {
                    sta=0;
                    break;
                }
                else if(tmp16==0)
                {
                    sta++;
                }
            }
        }
        break;
    case 5://datas
        if(datain == 0xab)
        {
            break;
        }
        else
        {
            tmp=uncover_data(pre,datain);
            data_xor^=tmp;
            buf[buf_len++]=tmp;
            left--;
            if(0==left)
            {
                sta++;
            }
        }
        break;
    case 6://xor
        if(datain == 0xab)
        {
            break;
        }
        else
        {
            tmp=uncover_data(pre,datain);
            data_xor^=tmp;
            buf[buf_len++]=tmp;
            if(0==data_xor)
            {
                call_util_get_data(((uint16_t)buf[0]<<8)|buf[1],&buf[4],((uint16_t)buf[2]<<8)|buf[3]);
                pre=datain;
                sta=0;
                return true;
            }
            sta=0;
        }
        break;
    default:
        sta=0;
        break;
    }

    pre=datain;
    return false;
}



void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdata,uint16_t datalen)
{

    uint8_t *pdatas;
    uint8_t buf[100],len;
    uint8_t string_len;


    if(cmd>=PIGS_ACK && cmd<=PIGS_NFO)
    {
        if (pdata[0] != Stu485rev.addr) return;

        Stu485rev.len=datalen;
        memcpy(Stu485rev.data,pdata,datalen);
        Stu485rev.cmd=cmd;
        return;
    }
    if (pdata[0] != get_key()) return;
    pdatas = &pdata[1];
    switch(cmd)
    {
    case SET_NETWORK_PARAMETERS://上位机器配置网络参数
        //0x82+ip(4bytes)+port(2byte)+dhcp(1byte)+本机IP+网关+子网掩码+DNS1+DNS2
        //memcpy(test_buf,pdatas,datalen);
        len = 0;
        string_len = 0;
        while(pdatas[string_len++] != 0);

        memcpy(StuHis.StuPara.remote_ip,&pdatas[len],string_len);
        len+=string_len;
        StuHis.StuPara.remote_port = ( (pdatas[len]<<8) | pdatas[len+1]  );
        len+=2;
        StuHis.StuPara.dhcpstatus = pdatas[len];
        len++;
        memcpy(StuHis.StuPara.ip,&pdatas[len],4);
        len+=4;
        memcpy(StuHis.StuPara.gateway,&pdatas[len],4);
        len+=4;//网关
        memcpy(StuHis.StuPara.netmask,&pdatas[len],4);
        len+=4;//子网掩码
        memcpy(StuHis.StuPara.dns1,&pdatas[len],4);
        len+=4;//DNS1
        memcpy(StuHis.StuPara.dns2,&pdatas[len],4);
        len+=4;//DNS2
        buf[0] = 0;
        send_ack(MCU_ACK,SET_NETWORK_PARAMETERS,buf,1);
        AnsyHis();
        delay_ms(200);
        WriteFlgBootSys();
        break;
    case READ_NETWORK_PARAMETERS://读取网络参数
        //0x82+ip(4bytes)+port(2byte)+dhcp(1byte)+本机IP+网关+子网掩码+DNS1+DNS2
        len = 0;

        memcpy(&buf[len],StuHis.StuPara.remote_ip,strlen((char*)StuHis.StuPara.remote_ip));
        len+=strlen((char*)StuHis.StuPara.remote_ip);
        buf[len] = 0;
        len++;
        buf[len] = StuHis.StuPara.remote_port>>8;
        len++;
        buf[len] = StuHis.StuPara.remote_port;
        len++;
        buf[len] = StuHis.StuPara.dhcpstatus;
        len++;
        memcpy(&buf[len],StuHis.StuPara.ip,4);
        len+=4;
        memcpy(&buf[len],StuHis.StuPara.gateway,4);
        len+=4;
        memcpy(&buf[len],StuHis.StuPara.netmask,4);
        len+=4;
        memcpy(&buf[len],StuHis.StuPara.dns1,4);
        len+=4;
        memcpy(&buf[len],StuHis.StuPara.dns2,4);
        len+=4;
        memcpy(&buf[len],StuHis.StuPara.mac,6);
        len+=6;
        send_data(MCU_NETWORK_PARAMETERS,buf,len);
        break;
    case SET_FACTORY://恢复出厂设置
        buf[0] = 0;
        send_ack(MCU_ACK,SET_FACTORY,buf,1);
        ResetHis();
        AnsyHis();
        delay_ms(200);
        WriteFlgBootSys();
        break;
    case UPFILE_START://升级开始
        file_cmd = UPFILE_START;
        //0x85+file_size(4byte)+crc(2byte)
        file_size = pdatas[0];
        file_size = file_size<<8;
        file_size |= pdatas[1];
        file_size = file_size<<8;
        file_size |= pdatas[2];
        file_size = file_size<<8;
        file_size |= pdatas[3];
        file_Offset = 0;
        file_crc = pdatas[4];
        file_crc= file_crc<<8;
        file_crc |= pdatas[5];
        buf[0] = 0;
        send_ack(MCU_ACK,UPFILE_START,buf,1);
        delay_ms(500);
        AnsyHis();
        delay_ms(100);
        WriteUpflg(NULL,0);
        WriteFlgBootSys();

        break;
    case UPFILE_DATA://升级数据
        //0x86+index(2byte)+data(16byte)
        file_cmd = UPFILE_DATA;
        memcpy(&iap_buf[0],pdatas,2+UPGRADE_PACKET_SIZE);
        iap_len = UPGRADE_PACKET_SIZE;
        file_cmd = UPFILE_DATA;
        break;
    case UPFILE_PAR://升级参数
        file_size = pdatas[0];
        file_size = file_size<<8;
        file_size |= pdatas[1];
        file_size = file_size<<8;
        file_size |= pdatas[2];
        file_size = file_size<<8;
        file_size |= pdatas[3];
        file_Offset = 0;
        file_crc = pdatas[4];
        file_crc= file_crc<<8;
        file_crc |= pdatas[5];
        iap_flag = true;
        file_cmd = UPFILE_PAR;
        break;
    case INFORMATION_PAR://开发板信息

        len=sprintf((char*)buf,"softversion:%d.%d.%d",version[0],version[1],version[2]);
        send_data(MCU_INFORMATION_PAR,buf,len);
        break;


    }



}


//===================================================================
//





/*
���ط���ack 0x31

*/
#define TMP_BUF_LEN  30
void send_ack2pigs(uint16_t cmd,uint16_t answer_cmd,uint8_t code,uint8_t key)
{

    uint8_t outbuf[TMP_BUF_LEN];
    uint16_t len;
    uint8_t sendbuf[TMP_BUF_LEN],sendlen=0;


    sendbuf[sendlen++]=key;
    sendbuf[sendlen++]=answer_cmd;
    sendbuf[sendlen++]=code;


    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}
/*
���ط��ͽ���ָ��0x32
���ط���ι��ָ��0x33

*/
void send_food2pigs(uint16_t cmd,uint16_t foods,uint8_t key)//
{

    uint8_t outbuf[TMP_BUF_LEN];
    uint16_t len;
    uint8_t sendbuf[TMP_BUF_LEN],sendlen=0;


    sendbuf[sendlen++]=key;


    sendbuf[sendlen++]=foods>>8;
    sendbuf[sendlen++]=foods;
//  memcpy(&sendbuf[sendlen],pdata,data_len);
//  sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}

/*
���ط��Ϳ��ص�ָ��0x34
���ط�����Ļ��ʾָ��0x35
���ط�����Ļ��ʾָ��0x3a
*/
void send_led_lcd2pigs(uint16_t cmd,uint8_t ledlcd,uint8_t key)//
{

    uint8_t outbuf[TMP_BUF_LEN];
    uint16_t len;
    uint8_t sendbuf[TMP_BUF_LEN],sendlen=0;


    sendbuf[sendlen++]=key;


    sendbuf[sendlen++]=ledlcd;
//  memcpy(&sendbuf[sendlen],pdata,data_len);
//  sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}

/*
���ط��ͳ��ش���������ָ��0x36
���ط���������ι��������״ָ̬��0x37
.���ط���������ι���豸��Ϣָ��0x39




*/

void send_cmd2pigs(uint16_t cmd,uint8_t key)
{

    uint8_t outbuf[TMP_BUF_LEN];
    uint16_t len;
    uint32_t tick_time;
    uint8_t sendbuf[TMP_BUF_LEN],sendlen=0;

    sendbuf[sendlen++]=key;
    if(cmd==0x39)
    {
        memcpy(&sendbuf[sendlen],&StuHis.StuPara.mac[0],6);
        sendlen+=6;
//        memset(StuHis.StuPara.gatewayid,0,MAX_GATWAY_ID);
        memcpy(&sendbuf[sendlen],StuHis.StuPara.gatewayid,strlen((char*)StuHis.StuPara.gatewayid));
        sendlen+=strlen((char*)StuHis.StuPara.gatewayid);
        
        
        tick_time=get_tick_from_rtc(0,0);
       
     
        
        
        
        sendbuf[sendlen++]=0;
            sendbuf[sendlen++]=tick_time>>24;
         sendbuf[sendlen++]=tick_time>>16;
         sendbuf[sendlen++]=tick_time>>8;
         sendbuf[sendlen++]=tick_time;
    }

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}

/*
���ط���������ι��ʵ�ʵĽ�ι����ָ��0x38
0x00:������ι��ʵ�ʽ���������
0x01:������ι��ʵ��ι������

*/
void send_cmd_read_foods2pigs(uint16_t cmd,uint8_t key,uint8_t feed_out)
{

    uint8_t outbuf[TMP_BUF_LEN];
    uint16_t len;
    uint8_t sendbuf[TMP_BUF_LEN],sendlen=0;

    sendbuf[sendlen++]=key;
    sendbuf[sendlen++]=feed_out;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}


