/********************************************************************************/
/*@file            :       drv_key.h		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Monday, March 27, 2017		upload: $Date$	*/
/*@note            :       Copyright(c) 2014 PROTRULY. All rights reserved.	*/
/********************************************************************************/

#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__

/****************************************INCLUDE*************************************************/
#include "app.h"
#include "drv_out_key_cfg.h"


/****************************************** MACROS **********************************************/


/****************************************** TYPEDEFS *********************************************/
typedef enum tag_key_io_status_t
{
	KEY_IO_INVALID,
	KEY_IO_PRESS,
	KEY_IO_UP,
	KEY_IO_MAX
} key_io_status_t;


typedef enum
{
	KEY_Down = 0,
	KEY_ShortPress,
	KEY_DoubleTap,
	KEY_LongPress,
	KEY_GestureTotal,
}key_gesture_t;

/****************************************** Constants  *******************************************/


/*************************************** GLOBAL VARIABLES ****************************************/


/****************************************** FUNCTIONS ********************************************/
void ofo_gesture_init(void);
void ofo_gesture_exit(void);
void ofo_key_gesture_service(void);
void ofo_register_key(key_value_t key_index);

void drv_key_init(void);
void drv_key_exit(void);
key_io_status_t drv_debounce_key(key_value_t cur_key);

#endif //__DRV_KEY_H__
