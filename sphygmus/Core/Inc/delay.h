#ifndef _DELAY_H
#define _DELAY_H

#include "stm32f10x.h"


void Sys_Delay_Init(void);
void delay_us(u32 us);
void delay_ms(u32 ms);

#endif
