#ifndef __FREEGUI_H
#define __FREEGUI_H
#include "main.h"
#include "lcd_spi_169.h"

#define 	GUI_WHITE       0xFFFFFF	 // ����ɫ
#define 	GUI_BLACK       0x000000    // ����ɫ                     
#define 	GUI_BLUE        0x0000FF	 //	����ɫ
#define 	GUI_GREEN       0x00FF00    //	����ɫ
#define 	GUI_RED         0xFF0000    //	����ɫ
#define 	LGUI_CYAN        0x00FFFF    //	����ɫ
#define 	LGUI_MAGENTA     0xFF00FF    //	�Ϻ�ɫ
#define 	GUI_YELLOW      0xFFFF00    //	��ɫ
#define 	GUI_GREY        0x2C2C2C    //	��ɫ												
#define 	GUI_LIGHT_BLUE      0x8080FF    //	����ɫ
#define 	GUI_LIGHT_GREEN     0x80FF80    //	����ɫ
#define 	GUI_LIGHT_RED       0xFF8080    //	����ɫ
#define 	GUI_LIGHT_CYAN      0x80FFFF    //	������ɫ
#define 	GUI_LIGHT_MAGENTA   0xFF80FF    //	���Ϻ�ɫ
#define 	GUI_LIGHT_YELLOW    0xFFFF80    //	����ɫ
#define 	LIGHT_GREY      0xA3A3A3    //	����ɫ											
#define 	GUI_DARK_BLUE       0x000080    //	����ɫ
#define 	GUI_DARK_GREEN      0x008000    //	����ɫ
#define 	GUI_DARK_RED        0x800000    //	����ɫ
#define 	GUI_DARK_CYAN       0x008080    //	������ɫ
#define 	GUI_DARK_MAGENTA    0x800080    //	���Ϻ�ɫ
#define 	GUI_DARK_YELLOW     0x808000    //	����ɫ
#define 	GUI_DARK_GREY       0x404040    //	����ɫ

extern uint32_t active_window;
//�ؼ��ṹ��
typedef struct control{

}Free_control;

//���ڽṹ��
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

