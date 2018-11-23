/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "stm32f4x7_eth.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* lwip includes */
#include "lwip/sys.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern xSemaphoreHandle s_xSemaphore;
extern xSemaphoreHandle ETH_link_xSemaphore;
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void);
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
/**************add by Hosea *************************/


/***************end add************/


#if 0
void NMI_Handler(void)
{
	ASSERT(0, 0);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
	printf("%s\r\n",__FUNCTION__);
	ASSERT(0, 0);
    while (1)
    {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    printf("%s\r\n",__FUNCTION__);
	ASSERT(0, 0);
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
	printf("%s\r\n",__FUNCTION__);
	ASSERT(0, 0);
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	printf("%s\r\n",__FUNCTION__);
	ASSERT(0, 0);
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

#else

void stackDump(uint32_t stack[])
{
    printf("r0=0x%x\n", stack[0]);
    printf("r1=0x%x\n", stack[1]);
    printf("r2=0x%x\n", stack[2]);
    printf("r3=0x%x\n", stack[3]);
    printf("r12=0x%x\n", stack[4]);
    printf("lr=0x%x\n", stack[5]);
    printf("pc=0x%x\n", stack[6]);
    printf("psr=0x%x\n", stack[7]);
}

void Handle_Mode_Handler(uint32_t stack[])
{
    stackDump(stack);
    while(1);
    //SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
}


__asm void NMI_Handler(void)
{
    MOVS    r0, #4  
    MOV     r1, lr
    TST     r0, r1          //; check LR bit 2                 
    BEQ     Stack_Use_MSP   //; stack use MSP
    MRS     r0, psp         //; stack use PSP, read PSP
    B       Get_LR_and_Branch
Stack_Use_MSP
    MRS     r0, msp         //; read MSP
Get_LR_and_Branch
    MOV     r1, lr          //; LR current value
    LDR     r2,=__cpp(Handle_Mode_Handler) //; branch to Hard_Fault_Handler 
    BX      r2
}

__asm void HardFault_Handler(void)
{
    MOVS    r0, #4  
    MOV     r1, lr
    TST     r0, r1          //; check LR bit 2                 
    BEQ     Stack_Use_MSP1   //; stack use MSP
    MRS     r0, psp         //; stack use PSP, read PSP
    B       Get_LR_and_Branch1
Stack_Use_MSP1
    MRS     r0, msp         //; read MSP
Get_LR_and_Branch1
    MOV     r1, lr          //; LR current value
    LDR     r2,=__cpp(Handle_Mode_Handler) //; branch to Hard_Fault_Handler 
    BX      r2
}

__asm void MemManage_Handler(void)
{
    MOVS    r0, #4  
    MOV     r1, lr
    TST     r0, r1          //; check LR bit 2                 
    BEQ     Stack_Use_MSP2   //; stack use MSP
    MRS     r0, psp         //; stack use PSP, read PSP
    B       Get_LR_and_Branch2
Stack_Use_MSP2
    MRS     r0, msp         //; read MSP
Get_LR_and_Branch2
    MOV     r1, lr          //; LR current value
    LDR     r2,=__cpp(Handle_Mode_Handler) //; branch to Hard_Fault_Handler 
    BX      r2
}

__asm void BusFault_Handler(void)
{
    MOVS    r0, #4  
    MOV     r1, lr
    TST     r0, r1          //; check LR bit 2                 
    BEQ     Stack_Use_MSP3   //; stack use MSP
    MRS     r0, psp         //; stack use PSP, read PSP
    B       Get_LR_and_Branch3
Stack_Use_MSP3
    MRS     r0, msp         //; read MSP
Get_LR_and_Branch3
    MOV     r1, lr          //; LR current value
    LDR     r2,=__cpp(Handle_Mode_Handler) //; branch to Hard_Fault_Handler 
    BX      r2
}

__asm void UsageFault_Handler(void)
{
    MOVS    r0, #4  
    MOV     r1, lr
    TST     r0, r1          //; check LR bit 2                 
    BEQ     Stack_Use_MSP4   //; stack use MSP
    MRS     r0, psp         //; stack use PSP, read PSP
    B       Get_LR_and_Branch4
Stack_Use_MSP4
    MRS     r0, msp         //; read MSP
Get_LR_and_Branch4
    MOV     r1, lr          //; LR current value
    LDR     r2,=__cpp(Handle_Mode_Handler) //; branch to Hard_Fault_Handler 
    BX      r2
}


#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    xPortSysTickHandler();
}

/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
    {
        /* Give the semaphore to wakeup LwIP task */
        //xSemaphoreGiveFromISR( ETH_link_xSemaphore, &xHigherPriorityTaskWoken );
    }
    /* Clear interrupt pending bit */
    EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);

    /* Switch tasks if necessary. */
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
}

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Frame received */
    if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET)
    {
        /* Give the semaphore to wakeup LwIP task */
        xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );
    }

    /* Clear the interrupt flags. */
    /* Clear the Eth DMA Rx IT pending bits */
    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);

    /* Switch tasks if necessary. */
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    }
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
