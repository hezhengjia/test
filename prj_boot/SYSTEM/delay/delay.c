#include "delay.h"
#include "sys.h"

 
 
void delay_us(u32 nus)
{    
   unsigned short i=0;  
   while(nus--)
   {
      i=10;  
      while(i--) ;    
   }
}

void delay_ms(u16 nms)
{
   unsigned short i=0;  
   while(nms--)
   {
      i=12000; 
      while(i--) ;    
   }
}



































