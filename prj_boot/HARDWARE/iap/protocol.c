#include "protocol.h"

#include "in_flash_manage.h"
#include "stmflash.h"
#include "timer.h"
#include "delay.h"
#include "uart_packt.h"


#include "uart.h"
#ifdef EX_FLASH
#include "w25qxx.h"
#endif

bool iap_flag = false;
uint32_t file_size;
uint32_t file_Offset;
uint16_t file_crc;
uint8_t file_cmd;
uint8_t iap_buf[UPGRADE_PACKET_SIZE+16];
uint16_t iap_len = 0;
uint32_t g_ticktime_50ms = 0;

//const uint8_t version[8] __attribute__ ((at(APP_BVK_ADDR+1024)))= {0,0,0,'2','d','a','n'};


static bool IsRightBoard(char* boardbuf)
{
    bool OkFlg=true;
	
	const uint8_t version[12] ={1,1,1,'2','d','a','n',1,1,1};

    if(0!=memcmp((char*)&version[3],boardbuf+3,4))
        OkFlg=false;
    return OkFlg;
}
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
//	uint32_t size;
    uint16_t tmpcrc=0xffff;
#ifdef EX_FLASH
    uint16_t crc_tmp;

    uint32_t num,last,j,addr;

#endif
    if (file_size&0x80000000)
    {
#ifdef EX_FLASH
		size = file_size&0x7fffffff;
        last=0;
        num=size/len;
        if(size%len)
        {
            num++;
            last=size%len;
        }
        addr=EX_UI_ADDR;//EX_UI_ADDR;///EX_UI_ADDR;
        for(j=0; j<num; j++)
        {
            if(j==(num-1))
            {
                len=last;
            }
            W25QXX_Read(buf,addr,len);
            addr+=len;
            crc_tmp=CalcCrc16(tmpcrc,(const uint8_t *)buf,len);
            tmpcrc=((crc_tmp>>8)+(crc_tmp<<8));
        }
        crcout[0]=crc_tmp;
#endif
    } else
    {
        crcout[0]=CalcCrc16(tmpcrc,(const uint8_t *)APP_BVK_ADDR,file_size);

    }


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
    pdata=(uint8_t*)(	BOOTLOADER_PARA_ADDR);

    if((pdata[0]!=0xaa)||(pdata[1]!=0x55))
    {
        return false;
    }
    return true;
}
#include "iwdg.h"
void WriteFlgBootSys(bool disversion)
{
			IWDG_Init(6,1250/3); 
	while(2);
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}
uint8_t in_buf[300];
static void WriteBvkFlash(uint32_t* buf,uint32_t numOfDword)
{
//	uint32_t size;
//	size = file_size&0x7fffff;
    if (file_size&0x80000000)
    {
#ifdef EX_FLASH
        W25QXX_Write((uint8_t *)buf,EX_UI_ADDR+file_Offset,numOfDword*4);
		W25QXX_Read((uint8_t *)in_buf,EX_UI_ADDR+file_Offset,numOfDword*4);
		if (memcmp(in_buf,buf,numOfDword*4) != 0)
		{
			in_buf[0] = 0;
		}
#endif
    } else
    {
        STMFLASH_Write(APP_BVK_ADDR+file_Offset,(uint32_t*)buf,numOfDword);
    }
    file_Offset+=(numOfDword*4);
}
static bool BvkAppWriteApp(void)
{

    uint32_t readtimes,i;
    uint32_t *pdatasrc;
    uint16_t tmpcrc=0xffff,crcout;
	uint32_t size;
	size = file_size&0x7fffff;
	

    pdatasrc=(uint32_t *)APP_BVK_ADDR;

    STMFLASH_Erase(ADDR_FLASH_SECTOR_5);

    readtimes=size/4;
    if(size%4)
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
static bool EraseBvkFlash(void )
{
    if (file_size&0x80000000)
    {
#ifdef EX_FLASH
        W25QXX_BlockErase(W25X_Block1);
        W25QXX_BlockErase(W25X_Block2);
        W25QXX_BlockErase(W25X_Block3);
        W25QXX_BlockErase(W25X_Block4);
        W25QXX_BlockErase(W25X_Block5);
        W25QXX_BlockErase(W25X_Block6);
#endif

    } else
    {
        STMFLASH_Erase(ADDR_FLASH_SECTOR_6);
    }
    return true;
}
uint8_t get_key(void)
{
    return 255;
}
static void send_ack(uint16_t cmd,uint16_t answer_cmd,uint8_t *pdata,uint16_t data_len)
{

    uint8_t outbuf[100];
    uint16_t len;
    uint8_t sendbuf[100],sendlen=0;

    sendbuf[0] = get_key();
    sendlen++;
    //sendlen += 16;

    sendbuf[sendlen++]=answer_cmd;
    memcpy(&sendbuf[sendlen],pdata,data_len);
    sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}

static void send_data(uint16_t cmd ,uint8_t *pdata,uint16_t data_len)
{

    uint8_t outbuf[150];
    uint16_t len;
    uint8_t sendbuf[150],sendlen=0;

    sendbuf[0] = get_key();
    sendlen++;
    //sendlen += 16;


    memcpy(&sendbuf[sendlen],pdata,data_len);
    sendlen+=data_len;

    len=uart_package_data(cmd,sendbuf,sendlen,outbuf);

    uart_sent_bytes(outbuf,len);

}
static void RfRevFileSendResult(uint8_t result)
{

    uint8_t sendbuf[30],sendlen;
    uint8_t outbuf[10],len;

    sendlen = 0;

    sendbuf[0] = get_key();
    sendlen++;
    //sendlen += 16;

    sendbuf[sendlen++] = result;

    len=uart_package_data(MCU_UPFILE_RESULT,sendbuf,sendlen,outbuf);
    uart_sent_bytes(outbuf,len);
}
static void AskPacket(uint32_t packet)
{
    uint8_t sendbuf[30],sendlen;
    uint8_t outbuf[10],len;

    sendlen = 0;
    sendbuf[0] = get_key();
    sendlen++;
    //sendlen += 16;

    sendbuf[sendlen++] = packet>>8;
    sendbuf[sendlen++] = packet;

    len=uart_package_data(MCU_ASK_PACKET,sendbuf,sendlen,outbuf);
    uart_sent_bytes(outbuf,len);

}

static bool RfWritePacket(uint8_t*rev_buf,uint32_t maxbuf,uint32_t *rev_lenth,uint32_t *packet )
{
	uint32_t size;
    uint8_t write_all,ask_flg=0;
    write_all=false;
    memcpy(&rev_buf[rev_lenth[0]],&iap_buf[2],UPGRADE_PACKET_SIZE);
    rev_lenth[0]+=UPGRADE_PACKET_SIZE;
	
	size = file_size &0x7fffffff;
    if((file_Offset+rev_lenth[0])<size)
    {
        ask_flg=1;

    }
    if(rev_lenth[0]>=maxbuf)
    {
        WriteBvkFlash((uint32_t*)rev_buf,maxbuf/4);
        rev_lenth[0]-=maxbuf;
    }
    if((file_Offset+rev_lenth[0])>=size)
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
#define MAX_REV_BUF_LEN			256

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
    //if (StuHis.StuPara.iap_flag == false ) return;

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
            return;
        } else
        {
            break;
        }

    }
    iap_len = 0;
    rev_lenth = 0;
    packet = 0;
    AskPacket(packet);
    EraseBvkFlash();
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
                    if(( (file_size&0x80000000) == 0) && (packet==5) )
                    {
						if  (true!=IsRightBoard((char*)(APP_BVK_ADDR+1024)) ) 
						{
							RfRevFileSendResult(1);
							iap_flag = false;
							WriteFlgBootSys(true);
							return;
						}
                    }

                    if(true==(RfWritePacket(rev_buf,MAX_REV_BUF_LEN,&rev_lenth,&packet)))
                    {
                        crc_ok=GetFileCrc(file_crc,&crcout,rev_buf,MAX_REV_BUF_LEN);
						if (file_size&0x80000000)
						{
							if(true==crc_ok)
							{
								RfRevFileSendResult(0);
							}else
							{
								RfRevFileSendResult(1);
							}
							return;
						}
						
                        if(true==crc_ok)
                        {
                            RfRevFileSendResult(0);
							
							if (true == BvkAppWriteApp())
							{
								RfRevFileSendResult(0);
								
								CleanUpflg();
								WriteFlgBootSys(true);
							} else
							{
								RfRevFileSendResult(1);
								return;
							}
							
                

                        } else
                        {
                            RfRevFileSendResult(1);
//							StuHis.StuPara.iap_flag = false;
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
            if( (file_size&0x800000) && (true==crc_ok))
            {
                WriteFlgBootSys(true);
            }
//			StuHis.StuPara.iap_flag = false;
//            AnsyHis();
            return;
        }
    }


}

void task_protocol_dataout_fun(uint16_t cmd,uint8_t*pdata,uint16_t datalen)
{
    uint8_t *pdatas;
    uint8_t buf[100],len;
    uint8_t string_len;

	
    if (pdata[0] != get_key()) return;
	len = len;
	string_len = string_len;
    pdatas = &pdata[1];
    switch(cmd)
    {

    case SET_NETWORK_PARAMETERS://上位机器配置网络参数
        //0x82+ip(4bytes)+port(2byte)+dhcp(1byte)+本机IP+网关+子网掩码+DNS1+DNS2
        //memcpy(test_buf,pdatas,datalen);
		#ifndef BOOT_LOADER
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
		#endif
        break;
    case READ_NETWORK_PARAMETERS://读取网络参数
        //0x82+ip(4bytes)+port(2byte)+dhcp(1byte)+本机IP+网关+子网掩码+DNS1+DNS2
		#ifndef BOOT_LOADER
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
		#endif
        break;
    case SET_FACTORY://恢复出厂设置
		#ifndef BOOT_LOADER
        buf[0] = 0;
        send_ack(MCU_ACK,SET_FACTORY,buf,1);
        ResetHis();
        AnsyHis();
		#endif
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
		#ifndef BOOT_LOADER
		if (file_size&0x800000)
		{

		}else
		{
//		delay_ms(500);
//		AnsyHis();
//		delay_ms(100);
//		WriteUpflg(NULL,0);
		WriteFlgBootSys(true);
		}

		#endif
        break;
    case UPFILE_DATA://升级数据
        //0x86+index(2byte)+data(16byte)
	#ifdef BOOT_LOADER
        file_cmd = UPFILE_DATA;
        memcpy(&iap_buf[0],pdatas,2+UPGRADE_PACKET_SIZE);
        iap_len = UPGRADE_PACKET_SIZE;
        file_cmd = UPFILE_DATA;
	#endif
        break;
    case UPFILE_PAR://升级参数
			#ifdef BOOT_LOADER
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
	#endif
        break;
    case INFORMATION_PAR://开发板信息
	#ifndef BOOT_LOADER
        len = sizeof("V1.0.1");
        memcpy(buf,"V1.0.1",len);
        send_data(MCU_INFORMATION_PAR,buf,len);
	#endif
        break;


    }



}








