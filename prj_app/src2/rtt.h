
#ifndef RTT_H__
#define RTT_H__

#include "SEGGER_RTT.h"

#define MAIN_RTT
#ifdef MAIN_RTT
#define MAIN_LOGRTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define MAIN_LOGRTT(...)
#endif


#define MOTOR_RTT
#ifdef MOTOR_RTT
#define MOTOR_LOGRTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define MOTOR_LOGRTT(...)
#endif





#define HX7XX_RTT


#ifdef HX7XX_RTT
#define HX7XX_LOGRTT(...) 	SEGGER_RTT_printf(0,__VA_ARGS__);//fprintf(stdout, ">>>>>" format "<<<<", ##__VA_ARGS__)  
#else
#define HX7XX_LOGRTT(...)
#endif


#endif




