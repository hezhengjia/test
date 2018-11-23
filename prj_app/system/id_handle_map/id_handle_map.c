/********************************************************************************/
/*@file            :       id_handle_map.c		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       i		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Friday, April 27, 2018		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2017 OFO. All rights reserved.	*/
/********************************************************************************/

/****************************************INCLUDE*************************************************/
#include	"id_handle_map.h"

/*******************************************************************************/


/****************************************** MACROS **********************************************/



/****************************************** TYPEDEFS *********************************************/


/****************************************** Constants  *******************************************/


/*************************************** GLOBAL VARIABLES ****************************************/


/****************************************** FUNCTIONS ********************************************/


/*****************************************************************
* Function Name : LogicListOperator                               
* Description   : Description of the brief functions of function  
* Date          : 2018/04/27                                      
* Parameter     :
* Return Code   :                                                 
* Author        : i                                              
*******************************************************************/

map_table_t map_get_function(const MSGMAP_ENTRY *pr_map, uint32_t size, uint32_t para)
{
	for (int loop=0; loop<size; loop++)
	{
		if (pr_map->para == para)
		{
			return pr_map->entry;
		}		
		pr_map++;
	}
	return NULL;
}

