#ifndef __FREEGUI_H
#define __FREEGUI_H
#include "main.h"
#include "lcd_spi_169.h"

#define 	GUI_WHITE       0xFFFFFF	 // 纯白色
#define 	GUI_BLACK       0x000000    // 纯黑色                     
#define 	GUI_BLUE        0x0000FF	 //	纯蓝色
#define 	GUI_GREEN       0x00FF00    //	纯绿色
#define 	GUI_RED         0xFF0000    //	纯红色
#define 	LGUI_CYAN        0x00FFFF    //	蓝绿色
#define 	LGUI_MAGENTA     0xFF00FF    //	紫红色
#define 	GUI_YELLOW      0xFFFF00    //	黄色
#define 	GUI_GREY        0x2C2C2C    //	灰色												
#define 	GUI_LIGHT_BLUE      0x8080FF    //	亮蓝色
#define 	GUI_LIGHT_GREEN     0x80FF80    //	亮绿色
#define 	GUI_LIGHT_RED       0xFF8080    //	亮红色
#define 	GUI_LIGHT_CYAN      0x80FFFF    //	亮蓝绿色
#define 	GUI_LIGHT_MAGENTA   0xFF80FF    //	亮紫红色
#define 	GUI_LIGHT_YELLOW    0xFFFF80    //	亮黄色
#define 	LIGHT_GREY      0xA3A3A3    //	亮灰色											
#define 	GUI_DARK_BLUE       0x000080    //	暗蓝色
#define 	GUI_DARK_GREEN      0x008000    //	暗绿色
#define 	GUI_DARK_RED        0x800000    //	暗红色
#define 	GUI_DARK_CYAN       0x008080    //	暗蓝绿色
#define 	GUI_DARK_MAGENTA    0x800080    //	暗紫红色
#define 	GUI_DARK_YELLOW     0x808000    //	暗黄色
#define 	GUI_DARK_GREY       0x404040    //	暗灰色

extern uint32_t active_window;
//控件结构体
typedef struct control{

}Free_control;

//窗口结构体
typedef struct window{
	uint32_t width;
	uint32_t height;
	uint32_t x;
	uint32_t y;
	uint32_t background_color;
	
}Free_window;

void FreeGUI_Handler(Free_window* Fgui);
void Free_draw_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t color);
#endif

