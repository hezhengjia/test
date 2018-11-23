/********************************************************************************/
/*@file		:	sys_assert.c													*/
/*@brief    	:debug  source file												*/
/*@author     :	Zhengjia He														*/
/*@version	:	Ver0.80															*/
/*@data    	:	2016-08-24														*/
/*@note		:	Copyright(c) 2016 PROTRULY. All rights reserved.				*/
/********************************************************************************/
#include "stdio.h"
#include "sys_assert.h"

static struct tag_error_information_t
{
	const char *error_last_file;			//record information after wakeup
	unsigned int his_error_totoal;	//counter from power on
	unsigned int error_counter;		//counter from every wakeup
	unsigned int error_last_line;	//record information after wakeup
} fst_error_infor = {0};

void error_information_initial(void)
{
	fst_error_infor.error_counter = 0U;
	fst_error_infor.error_last_line = 0U;
	fst_error_infor.error_last_file = (void *)0;
}

/************************************************************************
* Function Name : errorinfo
* Description	:get error id,error line, error file
* Date		: 2016/08/16
* Parameter	:	uError:	reference error.h
* 				uLine:	error line
* 				pcFile:	error File name
* Return Code	:void
* Author		:Zhengjia he
*************************************************************************/
void errorinfo(unsigned int uError, unsigned int type, unsigned int uLine, const char *pcFile)
{
	static int nest_flag;
	
	if (nest_flag == 1)
	{
		return;
	}
	nest_flag = 1;
	if (type == 0U)
	{
		printf("Error id:[%d],Line:[%d],File:[%s]\n", uError, uLine, pcFile);
	}
	else if (type == 1U)
	{
		//to flash
	}
	nest_flag = 0;
	fst_error_infor.his_error_totoal++;
	fst_error_infor.error_counter++;
	fst_error_infor.error_last_file = pcFile;
	fst_error_infor.error_last_line = uLine;

	//sys_cpu_wait_1ms(10000);
	//	while (1);//add for test
}





