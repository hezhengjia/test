#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

//extern uint32_t g_upgrade_flag_50ms;
extern uint32_t g_ticktime_50ms;
void TIM3_Int_Init(u16 arr,u16 psc);
#endif
