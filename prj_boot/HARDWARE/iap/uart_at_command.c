#include "uart_at_command.h"
#include "usart.h"






void send_at_back(char* datain,uint8_t len)
{
	uart_sent_bytes((uint8_t*)datain,len);
}
#define BUF_MAX_LEN 100
static char process_buf[BUF_MAX_LEN],process_len=0;
void process_at_command(uint8_t* pdatain,uint8_t d_length )
{

    int atlen;
//    bool from_uart;
//    double temp_f;
//    uint8_t temp_8;
//    uint16_t temp_16;
//    uint8_t namebuf[32];
	uint8_t i;
//    int itmp16;



    char *pdataend;
//    char *pdatastart;
    
    
    for(i=0; i<d_length; i++)
    {
        process_buf[process_len++]=pdatain[i];
        process_len%=BUF_MAX_LEN;
        if((process_buf[process_len-2]!=0x0d)||(process_buf[process_len-1]!=0x0a))  continue;

        pdataend=&process_buf[process_len];
		
        if((process_len>=4)&&(0==memcmp("AT\r\n",pdataend-4,4)))
		{
			send_at_back("OK+CONN\r\n",9);
		}else if( (process_len>=12)&&(0==memcmp("AT+FACTORY\r\n",pdataend-12,12)) )//16
		{
		    atlen=sprintf(process_buf,"OK+FACTORY\r\n");
            send_at_back(process_buf,atlen);
		}
		
//AT_END:
	memset(process_buf,0,BUF_MAX_LEN);
	atlen=0;
	}
}
void task_process_uart_data(void)
{
	
	uint8_t data;
    while ( true == queue_byte_out(&uart_queue,&data) )
    {
		process_at_command(&data,1);
    }
}






