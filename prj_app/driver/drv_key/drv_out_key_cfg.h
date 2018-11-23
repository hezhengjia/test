/********************************************************************************/
/*@file            :       drv_out_key_cfg.h		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Thursday, December 21, 2017		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2017 OFO. All rights reserved.	*/
/********************************************************************************/

#ifndef __DRV_OUT_KEY_CFG_H__
#define __DRV_OUT_KEY_CFG_H__

#define		REGISTER_GESTURE_KEY_SUM				1

typedef enum tagkey_value_t
{
	KEY_ONE,
	KEY_LOCK,
	KEY_LADDER,
	KEY_MAX
} key_value_t;


//按键对应的枚举		这个io的端口		io的pin脚		松开按键的电平			滤波时间：单位10ms
#define		KEY_MAP_INDEX()			{\
		{KEY_ONE,		GPIOE, 		GPIO_PIN_0, 	GPIO_PIN_SET,				5},\
		{KEY_LOCK,		GPIOB, 		GPIO_PIN_3, 	GPIO_PIN_SET,				5},\
		{KEY_LADDER,	GPIOD, 		GPIO_PIN_5, 	GPIO_PIN_SET,				50},\
}


#define	GESTURE_SHORT_PRESS_MIN_TIME				200
#define	GESTURE_SHORT_PRESS_MAX_TIME				2000
#define	GESTURE_LONG_PRESS_MIN_TIME					1000
#define	GESTURE_LONG_PRESS_MAX_TIME					0
#define	GESTURE_SHORTPRESS_RELEASE_TIME				200


#endif //__DRV_OUT_KEY_CFG_H__
