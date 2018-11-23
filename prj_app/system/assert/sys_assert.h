/********************************************************************************/
/*@file		:	debug.h															*/
/*@brief    	:	debug header file											*/
/*@author     :	sanyro															*/
/*@version	:	Ver0.80															*/
/*@data    	:	2016-06-25														*/
/*@note		:	Copyright(c) 2016 PROTRULY. All rights reserved.				*/
/********************************************************************************/

#ifndef __SYS_ASSERT_H__
#define __SYS_ASSERT_H__


/**************ASSERT declaration*******************/
#define		DEBUG_ASSERT				1
#if	DEBUG_ASSERT

void error_information_initial(void);
void errorinfo(unsigned int uError, unsigned int type, unsigned int uLine, const char *pcFile);
#define ASSERT(arg, val)				((arg) ? (void)0 : errorinfo((unsigned int)val, 0U, (unsigned int)__LINE__, __FILE__))

#else

#define	error_information_initial()		((void)0)
#define ASSERT(ignore1, ignore2)  		((void)0)

#endif	//ASSERT
/**************stack check declaration*******************/


/**************snapshot check declaration*******************/


#endif /* __SYS_ASSERT_H__ */


