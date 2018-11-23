/********************************************************************************/
/*@file            :       id_handle_map.h		*/
/*@brief           :       Description of the brief functions of file		*/
/*@author          :       Zhengjia He		upload: $Author$	*/
/*@version         :       Ver0.01		upload: $Revision$	*/
/*@data            :       Friday, April 27, 2018		upload: $Date$	*/ 
/*@note            :       Copyright(c) 2017 OFO. All rights reserved.	*/
/********************************************************************************/

#ifndef __ID_HANDLE_MAP_H__
#define __ID_HANDLE_MAP_H__

#include <stdint.h> 
/****************************************INCLUDE*************************************************/
#ifndef NULL
#define NULL ((void *)0)
#endif
/****************************************** MACROS **********************************************/

#define	get_map_table_function(List, para)		\
			map_get_function(fst_map_##List, sizeof(fst_map_##List)/sizeof(fst_map_##List[0]), para)

typedef uint32_t (*map_table_t)(uint32_t para1, uint32_t para2, uint32_t para3);

typedef struct MSGMAP_ENTRY
{
	uint32_t para;
	map_table_t entry;
}MSGMAP_ENTRY, *prMSGMAP_ENTRY;

#define BEGIN_MESSAGE_MAP(List) \
	const MSGMAP_ENTRY fst_map_##List[] = \
{ \

#define END_MESSAGE_MAP()		\
	{0} \
}; \

/****************************************** TYPEDEFS *********************************************/


/****************************************** Constants  *******************************************/


/*************************************** GLOBAL VARIABLES ****************************************/


/****************************************** FUNCTIONS ********************************************/
map_table_t map_get_function(const MSGMAP_ENTRY *pr_map, uint32_t size, uint32_t para);

#endif //__ID_HANDLE_MAP_H__
