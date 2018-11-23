/********************************************************************************/
/*@file            :       drv_key.c		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Monday, March 27, 2017		upload: $Date$	*/
/*@note            :       Copyright(c) 2014 PROTRULY. All rights reserved.	*/
/********************************************************************************/

/****************************************INCLUDE*************************************************/


#if OUT_DRIVER_KEY_MODULE_ENABLE
#include "drv_key.h"
#if REGISTER_GESTURE_KEY_SUM

extern void oPostEvent( uint32_t eventid, uint32_t instanceid, uint32_t eventpara1, uint32_t eventpara2 );
extern uint32_t   oGetCurrentInstanceID( void );

typedef struct tagGestureStruct
{
//	key_gesture_t Gesture;
	uint16_t		MinTime;
	uint16_t		MaxTime;
	uint32_t		Message;	//User message
} tpGestureProperty,*ptpGestureProperty;


const tpGestureProperty g_short_press = {GESTURE_SHORT_PRESS_MIN_TIME, GESTURE_SHORT_PRESS_MAX_TIME, KEY_ShortPress};
const tpGestureProperty g_long_press = {GESTURE_LONG_PRESS_MIN_TIME, GESTURE_LONG_PRESS_MAX_TIME, KEY_LongPress};

static uint32_t gShortPressNum = 0;
static BOOL bPressing = FALSE;
static BOOL bLongPressed = FALSE;
static BOOL bDoubleShort = FALSE;
static uint32_t gTPStartTime, gTPReleaseTime;
static uint32_t g_Regster_Instance = 0;

static uint32_t Msg_LongPress(void)
{
	uint32_t ms_tick;
	
	if (!bLongPressed)
	{
		ms_tick = HAL_GetTick();
		if( (ms_tick - gTPStartTime) < (g_long_press.MinTime))
			return 0;
	//	if(gGestureFlag&(1<<TP_LongPressRepeat) || !bLongPressed)
		{
	//		gTPStartTime = ms_tick - 100;
			bLongPressed=TRUE;
			return g_long_press.Message;
		}		
	}
	return 0;
}

//Input: Gesture--TP_SingleTapUp or TP_ShortPress
//static uint32_t Msg_TPUpProc(void)
//{
////	uint32_t i,tm_dt;

////	tm_dt = HAL_GetTick() - gTPStartTime;

////	if( tm_dt < (g_short_press.MinTime) || tm_dt >= (g_short_press.MaxTime))
////		return 0;
////	return g_short_press.Message;
//	return 0;
//}

void ofo_gesture_init(void)
{
	gShortPressNum = 0;
	bPressing = FALSE;
	bLongPressed = FALSE;
	bDoubleShort = FALSE;	
}

void ofo_gesture_exit(void)
{
	ofo_gesture_init();
}

void ofo_key_gesture_service(void)
{
	uint32_t Message = 0;
	key_io_status_t debounce_status;

	debounce_status = drv_debounce_key(KEY_ONE);
	if(debounce_status == KEY_IO_UP)
	{
		if(bPressing)
		{
			if (!bLongPressed)
			{
				bDoubleShort = TRUE;
				gTPReleaseTime = HAL_GetTick();
			}
			else
			{
				gShortPressNum = 0;
				gTPReleaseTime = HAL_GetTick();
				bDoubleShort = FALSE;
			}
		}
		else
		{
			if ((bDoubleShort) && ((HAL_GetTick() - gTPReleaseTime) >= GESTURE_SHORTPRESS_RELEASE_TIME))
			{
//				Message = Msg_TPUpProc();
//				if (Message) 
				{
					if (gShortPressNum == 1)
					{
						Message = KEY_ShortPress;
					}
					else
					{
						Message = KEY_DoubleTap;
					}
				}
				bDoubleShort = FALSE;
				gShortPressNum = 0;				
			}
		}
		bLongPressed = FALSE;
		bPressing = FALSE;
	}
	else if(debounce_status == KEY_IO_PRESS)
	{
		if(!bPressing)
		{
			gTPStartTime = HAL_GetTick();
			bDoubleShort = FALSE;
			bLongPressed = FALSE;
			gShortPressNum++;
//			Message = Msg_TPDown(TPCurPos,MSG_TP_DOWN);
		}
		else
		{
			if (!bLongPressed)
			{
				Message = Msg_LongPress();
			}
		}
		bPressing = TRUE;
	}
	if (Message)
	{
		if (g_Regster_Instance)
		{
			oPostEvent(EVID_GESTURE_KEY, g_Regster_Instance, KEY_ONE, Message);
		}
	}
//	return Message;
}

//EVID_GESTURE_KEY	Õâ¸ö°´¼ü£¬
void ofo_register_key(key_value_t key_index)
{
	g_Regster_Instance = oGetCurrentInstanceID();
}

#endif //REGISTER_GESTURE_KEY_SUM
#endif //OUT_DRIVER_KEY_MODULE_ENABLE
