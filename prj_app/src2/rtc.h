#ifndef __RTC_H
#define __RTC_H
#include "sys.h"
#include "stm32f4xx.h"
#include <time.h>
#include <string.h>



void set_tick2rtc(uint32_t unix_tick);
int32_t get_tick_from_rtc( RTC_DateTypeDef *pdata,RTC_TimeTypeDef*ptime);
uint8_t stm32f4_rtc_Init(void);
void printf_time(void);
//void test_rtc(void);
#endif

















