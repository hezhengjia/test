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
/****************************************** MACROS **********************************************/
/****************************************** TYPEDEFS *********************************************/
typedef struct tag_key_map_t
{
	key_value_t 	key;
	GPIO_TypeDef 	*port;
	uint32_t 		pin;
	GPIO_PinState	status;
	uint32_t		time;
} key_map_t;

typedef struct tag_single_key_debounce_t
{
	key_io_status_t	value;
	signed char	hold_sum;
	signed char	hold_check;
} single_key_debounce_t;


/****************************************** Constants  *******************************************/


/*************************************** GLOBAL VARIABLES ****************************************/
const key_map_t fst_key_map[] = KEY_MAP_INDEX();
static single_key_debounce_t	fst_key_ctrl[KEY_MAX];


/****************************************** FUNCTIONS ********************************************/
static void drv_key_debounce(single_key_debounce_t *pst_debounce, int8_t status);

/*****************************************************************
* Function Name : drv_key_init
* Description   : Description of the brief functions of function
* Date          : 2017/03/27
* Parameter     :
* Return Code   :
* Author        : Zhengjia He
*******************************************************************/
void drv_key_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	uint32_t loop, loop2;
	
	memset(fst_key_ctrl, 0, sizeof(fst_key_ctrl));

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	if (KEY_MAX != DIM(fst_key_map))
	{
		ASSERT(0, 0);
	}
	for (loop=0; loop<KEY_MAX; loop++)
	{
		for (loop2=0; loop2<KEY_MAX; loop2++)
		{
			if (loop == fst_key_map[loop2].key)
			{
				if (fst_key_ctrl[loop].hold_sum == 0)
				{
					GPIO_InitStruct.Pin = fst_key_map[loop2].pin;
					HAL_GPIO_Init(fst_key_map[loop2].port, &GPIO_InitStruct);
					
					fst_key_ctrl[loop].hold_sum = fst_key_map[loop2].time;
				}
				else
				{
					ASSERT(0, 0);
				}
			}
		}
	}
}

void drv_key_exit(void)
{
	uint32_t loop;

	for (loop=0; loop<DIM(fst_key_map); loop++)
	{
		HAL_GPIO_DeInit(fst_key_map[loop].port, fst_key_map[loop].pin);
	}
	memset(&fst_key_ctrl, 0, sizeof(fst_key_ctrl));
}

//现在是一次性全部扫描。如果pin多的话，要改成分时扫描。因为在中断的时间不能太长。
void drv_key_scan(void)
{
	uint32_t loop;
	int8_t	status = 0;;

	for (loop=0; loop<DIM(fst_key_map); loop++)
	{
		if (HAL_GPIO_ReadPin(fst_key_map[loop].port, fst_key_map[loop].pin) == fst_key_map[loop].status)
		{
			status = 1;
		}
		else
		{
			status = 0;
		}
		drv_key_debounce(&fst_key_ctrl[fst_key_map[loop].key], status);
	}
}

static void drv_key_debounce(single_key_debounce_t *pst_debounce, int8_t status)
{
	key_io_status_t value = KEY_IO_UP;

	if (pst_debounce->hold_sum == 0)
	{
		return;
	}
	if (status == 0)
	{
		value = KEY_IO_PRESS;
	}
	if (status)
	{
		if (pst_debounce->hold_check < pst_debounce->hold_sum)
		{
			pst_debounce->hold_check++;
			if (pst_debounce->hold_check == pst_debounce->hold_sum)
			{
				if (pst_debounce->value != value)
				{
					pst_debounce->value = value;
				}
			}
		}
	}
	else
	{
		if (pst_debounce->hold_check > -pst_debounce->hold_sum)
		{
			pst_debounce->hold_check--;
			if (pst_debounce->hold_check == -pst_debounce->hold_sum)
			{
				if (pst_debounce->value != value)
				{
					pst_debounce->value = value;
				}
			}
		}
	}
}

key_io_status_t drv_debounce_key(key_value_t cur_key)
{
	if (cur_key >= KEY_MAX)
	{
		return KEY_IO_INVALID;
	}

	return fst_key_ctrl[cur_key].value;
}

#endif

