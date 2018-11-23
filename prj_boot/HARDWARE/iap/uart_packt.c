#include "uart_packt.h"

static uint8_t cover_data(uint8_t indata,uint8_t*out_data)
{
    uint8_t  len=0;
    if(0xaa==indata)
    {
        out_data[len++]=0xab;
        out_data[len++]=0x01;

    } else if(0xab==indata)
    {
        out_data[len++]=0xab;
        out_data[len++]=0x02;
    } else
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
        } else
        {
            r_value= nowdata;//error=========================
        }
    }

    return r_value;
}
/*
** 解包数据
** datain,串口数据入口
** call_util_get_data接收到数据的回调
** return,true接收一帧数据
*/
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
        } else
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
                } else if(tmp16==0)
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
        } else
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
        } else
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


/*
** 函数说明：打包发送
** cmd:传入的命令
** p_in_datas,datalen 数据部分的指针和长度
** p_out_data:打包完全的数据
**  返回值为打包号数据的长度
*/
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








