
/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include "tcpip.h"
#include <stdio.h>
#include "in_flash_manage.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DHCP_TRIES 4

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif xnetif; /* network interface structure */
extern __IO uint32_t  EthStatus;

__IO uint8_t DHCP_state;


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

//    uint8_t iptab[4] = {0};
//    uint8_t iptxt[20];

    /* Create tcp_ip stack thread */
    tcpip_init( NULL, NULL );

    /* IP address setting */
    if(StuHis.StuPara.dhcpstatus ==true)
    {
        ipaddr.addr = 0;
        netmask.addr = 0;
        gw.addr = 0;
    }
    else
    {
        IP4_ADDR(&ipaddr, StuHis.StuPara.ip[0],StuHis.StuPara.ip[1], StuHis.StuPara.ip[2], StuHis.StuPara.ip[3] );
        IP4_ADDR(&netmask, StuHis.StuPara.netmask[0], StuHis.StuPara.netmask[1], StuHis.StuPara.netmask[2], StuHis.StuPara.netmask[3]);
        IP4_ADDR(&gw,StuHis.StuPara.gateway[0],StuHis.StuPara.gateway[1], StuHis.StuPara.gateway[2], StuHis.StuPara.gateway[3]);

    }


    /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
    struct ip_addr *netmask, struct ip_addr *gw,
    void *state, err_t (* init)(struct netif *netif),
    err_t (* input)(struct pbuf *p, struct netif *netif))

    Adds your network interface to the netif_list. Allocate a struct
    netif and pass a pointer to this structure as the first argument.
    Give pointers to cleared ip_addr structures when using DHCP,
    or fill them with sane numbers otherwise. The state pointer may be NULL.

    The init function pointer must point to a initialization function for
    your ethernet netif interface. The following code illustrates it's use.*/
    netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

    /*  Registers the default network interface.*/
    netif_set_default(&xnetif);

    if (EthStatus == (ETH_INIT_FLAG | ETH_LINK_FLAG))
    {
        /* Set Ethernet link flag */
        xnetif.flags |= NETIF_FLAG_LINK_UP;

        /* When the netif is fully configured this function must be called.*/
        netif_set_up(&xnetif);
        if(StuHis.StuPara.dhcpstatus ==true)
        {
            DHCP_state = DHCP_START;
        }
    }
    else
    {
        /*  When the netif link is down this function must be called.*/
        netif_set_down(&xnetif);
        if(StuHis.StuPara.dhcpstatus ==true)
        {
            DHCP_state = DHCP_LINK_DOWN;
        }
        printf("Network Cable is not connect \r\n");
    }

    /* Set the link callback function, this function is called on change of link status*/
    netif_set_link_callback(&xnetif, ETH_link_callback);
}

uint8_t get_router_ip=0;
/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
#include "app.h"
void LwIP_DHCP_task(void * pvParameters)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    uint32_t IPaddress;
    uint8_t iptab[4] = {0};
//    uint8_t iptxt[20];

    for (;;)
    {
        switch (DHCP_state)
        {
            case DHCP_START:
            {
                if(StuHis.StuPara.dhcpstatus!=true)
                {

                    goto STATIC_IP_TAB;

                }

                dhcp_start(&xnetif);
                /* IP address should be set to 0
                   every time we want to assign a new DHCP address */
                IPaddress = 0;
                DHCP_state = DHCP_WAIT_ADDRESS;
                printf("dhcp start...\r\n");
            }
            break;

            case DHCP_WAIT_ADDRESS:
            {

                /* Read the new IP address */
                IPaddress = xnetif.ip_addr.addr;

                if (IPaddress!=0)
                {
                    DHCP_state = DHCP_ADDRESS_ASSIGNED;
                    /* Stop DHCP */
                    dhcp_stop(&xnetif);
                    get_router_ip=1;

                    iptab[0] = (uint8_t)(IPaddress >> 24);
                    iptab[1] = (uint8_t)(IPaddress >> 16);
                    iptab[2] = (uint8_t)(IPaddress >> 8);
                    iptab[3] = (uint8_t)(IPaddress);
                    printf("  dhcp IP  addr:%d.%d.%d.%d\r\n", iptab[3], iptab[2], iptab[1], iptab[0]);

                }
                else
                {
                    /* DHCP timeout */
                    if (xnetif.dhcp->tries > MAX_DHCP_TRIES)
                    {
                        /* Stop DHCP */
                        dhcp_stop(&xnetif);

                        DHCP_state = DHCP_TIMEOUT;

                    STATIC_IP_TAB:

                        /* Static address used */
                        IP4_ADDR(&ipaddr, StuHis.StuPara.ip[0],StuHis.StuPara.ip[1], StuHis.StuPara.ip[2], StuHis.StuPara.ip[3] );
                        IP4_ADDR(&netmask, StuHis.StuPara.netmask[0], StuHis.StuPara.netmask[1], StuHis.StuPara.netmask[2], StuHis.StuPara.netmask[3]);
                        IP4_ADDR(&gw,StuHis.StuPara.gateway[0],StuHis.StuPara.gateway[1], StuHis.StuPara.gateway[2], StuHis.StuPara.gateway[3]);
                        netif_set_addr(&xnetif, &ipaddr, &netmask, &gw);
                        get_router_ip=2;
                        printf("  Static IP address :%d.%d.%d.%d\r\n",StuHis.StuPara.ip[0],StuHis.StuPara.ip[1], StuHis.StuPara.ip[2], StuHis.StuPara.ip[3]);


                    }
                }
            }
            break;

            default:
            
                if(open_relay)
                {
                    delay_ms(5000);
                    RELAY_ON();
                    delay_ms(3000);

                    RELAY_OFF();

                    open_relay=0;

                }

                break;
        }

        /* wait 250 ms */
        vTaskDelay(250);
    }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
