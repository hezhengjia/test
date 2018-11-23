
/**********************************************************************************
 * FILE : console.h
 * Description:
 * Author: Kevin He
 * Created On: 2017-12-30 , At 14:20:43
 * Modifiled On : 
 * Version : 0.0.1
 * Information :
 **********************************************************************************/

#ifndef OFO_CONSOLE_H
#define OFO_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

struct __FILE 
{ ///// ADD THIS DEFINITION.
	int handle;
};

/************************ For Register console command ****************************/
typedef struct 
{
	char *cmd;
	void (*func)(int argc, char **argv);
}cmd_list_t;

#define DECLAREE_CMD_FUNC(cmd, func) \
	cmd_list_t list_##func  __attribute__((used, section("cmdList"))) \
	= {cmd, func}

/**********************************************************************************/

void start_shell_task(void);

void _outbyte(int c);
int _inbyte(unsigned short timeout);
void debug_data(const char *name, const unsigned char *data, int len);

extern char xbuff[1128];	
/********************************add for shell return******************************/
extern unsigned char cmd_return_log_lv;
#define CMD_RETURN(fmt,args...) do{if (cmd_return_log_lv) printf("\n[return:]" fmt, ##args);}while(0)
	
	
#ifdef __cplusplus
}
#endif

#endif
