#ifndef NTP_CLIENT_H
#define NTP_CLIENT_H

#include "main.h"
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

/*Static NTP IP ADDRESS*/ //@sz
#define NTP_Server_ADDR0   133
#define NTP_Server_ADDR1   100
#define NTP_Server_ADDR2   11
#define NTP_Server_ADDR3   8

#define NTP_PKT_LEN        48   //48bytes
#define NTP_PORT        123

#define NTP_LI_NO_WARNING          (0x00<<6) //no warning
#define NTP_VERSION               (4/* NTP Version 4*/<<3) 
#define NTP_MODE_CLIENT            0x03
#define NTP_MODE_SERVER            0x04
#define NTP_MODE_BROADCAST         0x05
#define NTP_MODE_MASK              0x07
#define RECEIVE_TS_OFFSET          32 // Receive_Timestamp offset
#define TRANSMIT_TS_OFFSET         40 //Transmit_Timestamp offset

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800)
#define SEC_TIME_ZONE              + (8*60*60) //Beijing,GMT+8










#endif //#ifndef NTP_CLIENT_H