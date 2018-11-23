/********************************************************************************/
/*@file            :       console.h		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Thursday, November 22, 2018		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2018 Dereck. All rights reserved.	*/
/********************************************************************************/

#include <stdarg.h>
#include <stdint.h>
#include "console.h"
#include "uart.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif


//#define USE_LIST
static uint8_t log_lv = 1;
#define LOGI(fmt,args...) do{if (log_lv) printf(fmt,##args);}while(0)

extern unsigned cmdList$$Base;
extern unsigned cmdList$$Length;
#define LIST_START() ((cmd_list_t *)&cmdList$$Base)
#define LIST_NUM()	((unsigned)(&cmdList$$Length)/sizeof(cmd_list_t))

#define list_for_each(list) for ((list)=LIST_START; \
	(unsigned)list < + (unsigned)&cmdList$$Base +(unsigned)(&cmdList$$Length);list++)
/*********************************************************************************/

#define console_printf printf

static char stdout_off = 0;

//xmodem, ′??ú??á?1?ó?buffer
char xbuff[1128] = {0}; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */

void stdout_open(void)
{
	stdout_off = 0;
}


static void stdout_close(void)
{
	stdout_off = 1;
}


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
void _sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
    while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
    USART1->DR = (u8) ch;
    return ch;
}

#else

int fputc(int ch, FILE*f)
{
	unsigned char tx_data = (unsigned char)ch;
	if (stdout_off)
		return -1;
	if (tx_data == '\n')
	{
		tx_data = '\r';
		while(drv_uart_send(OFO_UART_DEBUG, (const unsigned char * )&tx_data, 1) != 1);
		tx_data = '\n';
	}
	while(drv_uart_send(OFO_UART_DEBUG, (const unsigned char * )&tx_data, 1) != 1);
	return ch;
}

int fgetc(FILE *f) 
{
	unsigned char ch;
	if (drv_uart_recv(OFO_UART_DEBUG, &ch, 1) == 1)
		return ch;
	return -1;
}

#endif


	

/******************************** For shell ***********************************************/
#define next_line() console_printf("\r\n") 

static void print_help(int argc, char **argv)
{
	int i;
	if (argc != 1)
		return;
	console_printf("Usage Cmd --help\r\n");
	const cmd_list_t *plist = LIST_START();
	for(i = 0; i < LIST_NUM(); i++, plist++){
		console_printf("%s -- help\r\n",plist->cmd);
	}
}

static void send_cr(int argc, char **argv)
{
	if (argc == 1)
		console_printf("\033[2J\033[1H");
}

static void do_reset(int argc, char **argv)
{
	if (argc == 1)
		NVIC_SystemReset(); 
}

static void do_all_module_handle(int argc, char **argv)
{
	cmd_list_t *list;
	unsigned i;
	if (0 == strcmp(argv[1], "off") || 0 == strcmp(argv[1], "on"))
	{
		if (!strcmp(argv[1], "off"))
			log_lv = 0;
		else
			log_lv = 1;
		for(list = LIST_START(), i = 0; i < LIST_NUM(); i++, list++)
		{
			if ((do_all_module_handle != list->func) && list->func)
				list->func(2, argv);
		}
	}
}

/********************************** Task ************************************************/
static void exec_cmd(char *const cmd)
{
	const cmd_list_t *list;
	int index;
	char *argv[10];
	int i =0,j=0,flag=0;
//	next_line();
	if(*cmd < ' ' || *cmd > '~') return;
	
	argv[j++] = cmd;
	for(i=0;cmd[i];i++,flag = 0){
		if(cmd[i] == ' ' || cmd[i] == '\t'){
			cmd[i++]=0;
			flag = 1;
		}	
		if(flag && cmd[i] && cmd[i] != ' ' && cmd[i] != '\t') {
			if(j + 1 <ARRAY_SIZE(argv)){ 
				argv[j++]=cmd+i;
			}else{
				break;
			}
		}	
	}
	argv[j] = NULL;
	for(list = LIST_START(), index = 0; index < LIST_NUM(); index++, list++)
	{
		if(list->func && 0 == strcasecmp(argv[0],list->cmd)){
			list->func(j, (char **)argv);
			break;
		}
	}
	if(index == LIST_NUM()){
		console_printf(" \"%s\" not support now",argv[0]);
	}
	next_line();
}

//#define TEST_UART 
static int do_shell_loop(void)
{
	static unsigned short i=0;
	int ch;
	/*get one byte ,it may block */
	ch = fgetc(stdin);
	if (ch < 0)  {
		return -1 ;
	}
#ifdef TEST_UART
	fputc(ch, stdout);
#else	
	if(ch == '\r' || ch == '\n'){
//		if(i !=0)
			xbuff[i]=0;
		console_printf("\n");
		exec_cmd(xbuff);
		console_printf("# ");
		i=0;
	}else{
		if (ch == '\b') {
			if(i > 0){
				xbuff[--i] = 0;
				console_printf("\b \b");
			} 			
		} else if (i + 1 < sizeof(xbuff)){
			xbuff[i++]=ch;
			console_printf("%c", ch);
		}	
	}
#endif
	return 0;
}

static osThreadId shell_task_id;

static void shell_task(const void *para)
{
	for (;;)
	{
		drv_uart_wait_rx(OFO_UART_DEBUG, 0xffffffff);
		do_shell_loop();

		if (shell_task_id == NULL)
			break;
	}
}

static void check_user_cmd(void)
{
	const char *cmd1, *cmd2;
	cmd_list_t *list = LIST_START();
	int i,j, num, list_num = LIST_NUM();
	for (i = 0; i < list_num; i++)
	{
		cmd1 = list[i].cmd;
		num = 0;
		for (j = i + 1; j < list_num; j++)
		{
			cmd2 = list[j].cmd;
			if (0 == strcasecmp(cmd1, cmd2))
			{
				num++;
			}
		}
		if (num)
		{
			printf("######## user has registered %d same command: \"%s\" ############\n", num +1, cmd1);
		}
	}
}


void start_shell_task(void)
{
	ASSERT(shell_task_id == NULL ,0);
	check_user_cmd();
	if (shell_task_id)
		return;
	osThreadDef(shell_task, osPriorityNormal, 0, 2 * 1024);
	shell_task_id = osThreadCreate(osThread(shell_task), 0);
	if (shell_task_id == NULL)
	{
		ASSERT(0 ,0);
	}
	printf("YOU CAN input 'std [close/open]' to enter console control\n");
	printf("There are %d items\n",LIST_NUM());
	return;
}

void _outbyte(int c)
{
	unsigned char tx_data = (unsigned char)c;
	while(1 != drv_uart_send(OFO_UART_DEBUG, &tx_data, 1));
}

int _inbyte(unsigned short timeout) // msec timeout
{
    unsigned char ch;
		
	if (drv_uart_wait_rx(OFO_UART_DEBUG, timeout) >= 0 && drv_uart_recv(OFO_UART_DEBUG, &ch, 1) == 1) {
			return ch;
	}
	return -2;
}

static void xmodem_recv(int argc, char **argv)
{
	char fullName[13] = "0:/";

	if ((argc < 2) || NULL == argv[1])
	{
		return;
	}
	
	if (0 != strcmp(argv[0], "xmodem"))
	{
		return;
	}

	if (strlen(argv[1]) > 12)
	{
		console_printf("file name too long:%d\r\n", strlen(argv[1]));
		return;
	}

	strcat(fullName, argv[1]);
	console_printf("xmodem -> [%s]\r\n", fullName);

	stdout_close();
	xmodemReceive(fullName);

	//xmodem ?áê???1?ó??o′?????￡?±ü?a2Dá?êy?Yó°?ì?üá?DD′|àí
	memset(xbuff, 0, sizeof(xbuff));
	
	stdout_open();
	
}

unsigned char cmd_return_log_lv = 1;
static void cmd_return_control(int argc, char **argv)
{
	if (0 == strcmp(argv[1], "off"))
		cmd_return_log_lv = 0;
	else if (0 == strcmp(argv[1], "on"))
		cmd_return_log_lv = 1;
}


void debug_data(const char *name, const uint8_t *data, int len)
{
	int i;
	LOGI("[%s] (%d)", name, len);
	for (i = 0; i < len; i++)
	{
		if ((i&0x1f) == 0)
			LOGI("\n");
		LOGI("%02X ", data[i]);
	}
	LOGI("\n");
}




DECLAREE_CMD_FUNC("help", print_help);
DECLAREE_CMD_FUNC("clear", send_cr);
DECLAREE_CMD_FUNC("reboot", do_reset);
DECLAREE_CMD_FUNC("all", do_all_module_handle);
DECLAREE_CMD_FUNC("xmodem", xmodem_recv);
DECLAREE_CMD_FUNC("template", cmd_return_control);





