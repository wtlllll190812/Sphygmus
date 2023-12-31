#ifndef _OLED_H_
#define _OLED_H_

#include "main.h"
#include "iic.h"
#include "font.h"

void OLED_Send_Command(u8 com);
void Oled_Write_Data(u8 data);
void Oled_Init(void);
void OLED_Clear(u8 clear_dat);
void Oled_Address(u8 row, u8 col);
void Oled_Display_Char(u8 page, u8 col, char ch);
void Oled_Display_String(u8 page, u8 col, char *str);
void Oled_Display_Pic(u8 high, u8 wight, u8 page, u8 col, u8 *str);

#endif
