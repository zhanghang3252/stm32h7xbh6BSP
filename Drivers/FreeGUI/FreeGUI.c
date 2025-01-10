#include "FreeGUI.h"
void FreeGUI_Handler(Free_window* Fgui)
{
	Free_draw_Fill(Fgui->x,Fgui->y,Fgui->width,Fgui->height,Fgui->background_color);
}

void Free_draw_point(uint16_t x,uint16_t y,uint32_t color)
{
	LCD_DrawPoint(x,y, color);
}

void Free_draw_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t color)
{
	LCD_SetColor(color);
	LCD_SetAddress(x,y,x+width,y+height);
	LCD_FillRect(x,y,width,height);
}

