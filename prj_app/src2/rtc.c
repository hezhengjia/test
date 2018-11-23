#include "rtc.h"
#include "misc.h"



uint8_t stm32f4_rtc_Init(void)
{
#define RTC_BVK_VALUE  0x9529
    RTC_InitTypeDef RTC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();

    if(RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_BVK_VALUE)   //????,??RTC????
    {

//    RTC_WriteProtectionCmd(DISABLE);

//    RTC_EnterInitMode();
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        RTC_InitStructure.RTC_AsynchPrediv = 0x7D-1;
        RTC_InitStructure.RTC_SynchPrediv = 0xFF-1;
        RTC_Init(&RTC_InitStructure);

        set_tick2rtc(1539300960);

//    RTC_ExitInitMode();
        RTC_WriteBackupRegister(RTC_BKP_DR0,RTC_BVK_VALUE);
//    RTC_WriteProtectionCmd(ENABLE);
//    RTC_WriteBackupRegister(RTC_BKP_DR0,RTC_BVK_VALUE);
    }
//  PWR_BackupAccessCmd(DISABLE);
    return 0;
}



int32_t get_tick_from_rtc( RTC_DateTypeDef *pdata,RTC_TimeTypeDef*ptime)
{
    struct tm stm;
    struct tm *pstm;
    pstm=&stm;
    int iY, iM, iD, iH, iMin, iS;

    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;

    /* Get the RTC current Time */
    RTC_GetTime(RTC_Format_BIN, &stimestructureget);
    /* Get the RTC current Date */
    RTC_GetDate(RTC_Format_BIN, &sdatestructureget);
    if(pdata)
    {
        pdata->RTC_Year = sdatestructureget.RTC_Year;
        pdata->RTC_Month = sdatestructureget.RTC_Month;
        pdata->RTC_Date = sdatestructureget.RTC_Date;
        pdata->RTC_WeekDay = sdatestructureget.RTC_WeekDay;

    }

    if(ptime)
    {
        ptime->RTC_Hours= stimestructureget.RTC_Hours;
        ptime->RTC_Minutes = stimestructureget.RTC_Minutes;
        ptime->RTC_Seconds = stimestructureget.RTC_Seconds;
    }

    memset(pstm,0,sizeof(struct tm));


    iY = sdatestructureget.RTC_Year;
    iM = sdatestructureget.RTC_Month;
    iD = sdatestructureget.RTC_Date;
    iH = stimestructureget.RTC_Hours;
    iMin = stimestructureget.RTC_Minutes;
    iS = stimestructureget.RTC_Seconds;


    pstm->tm_year=iY+100;
    pstm->tm_mon=iM-1;
    pstm->tm_mday=iD;
    pstm->tm_hour=iH;
    pstm->tm_min=iMin;
    pstm->tm_sec=iS;

    return mktime(pstm);
}
struct tm *pstm;
void set_tick2rtc(uint32_t unix_tick)
{
//    struct tm *localtime(const time_t *timer)


    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;


    pstm= localtime(&unix_tick);

    sdatestructureget.RTC_Year=pstm->tm_year-100;
    sdatestructureget.RTC_Month=pstm->tm_mon+1;
    sdatestructureget.RTC_Date=pstm->tm_mday;


    sdatestructureget.RTC_WeekDay=(pstm->tm_wday!=0)? pstm->tm_wday:7;   //

    stimestructureget.RTC_Hours=pstm->tm_hour;
    stimestructureget.RTC_Minutes=pstm->tm_min;
    stimestructureget.RTC_Seconds=pstm->tm_sec;
    stimestructureget.RTC_H12=RTC_H12_AM;


//     RTC_WriteProtectionCmd(DISABLE);
    RTC_SetTime(RTC_Format_BIN, &stimestructureget);
    RTC_SetDate(RTC_Format_BIN,&sdatestructureget);
//     RTC_WriteProtectionCmd(ENABLE);

}





#include "rtt_printf.h"

void printf_time(void)
{

    uint32_t ttt;
    RTC_DateTypeDef pd;
    RTC_TimeTypeDef pt;
    ttt=get_tick_from_rtc(&pd,&pt);
    MAIN_LOGRTT("sec:%d %d-%d-%d %d:%d:%d w:%d\r\n",ttt,pd.RTC_Year,pd.RTC_Month,pd.RTC_Date,pt.RTC_Hours,pt.RTC_Minutes,pt.RTC_Seconds,pd.RTC_WeekDay);


}



//struct tm stm111;
//uint32_t ttt;
//void test_rtc(void)
//{

//    RTC_DateTypeDef pd;
//    RTC_TimeTypeDef pt;
//    static uint8_t flg=0;

//    if(0==flg)
//    {
//        stm32f4_rtc_Init();
//        stm32f4_rtc_Init();
//        flg=1;
//        set_tick2rtc(1539300990);//18-10-11 16:36
//    }

//    ttt=get_tick_from_rtc(&pd,&pt);
//    
//    
//    

//    MAIN_LOGRTT("sec:%d %d-%d-%d %d:%d:%d w:%d\r\n",ttt,pd.RTC_Year,pd.RTC_Month,pd.RTC_Date,pt.RTC_Hours,pt.RTC_Minutes,pt.RTC_Seconds,pd.RTC_WeekDay);



//}


