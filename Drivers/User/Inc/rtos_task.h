#ifndef __RTOS_TASK_H
#define __RTOS_TASK_H
#include "stdio.h"
#include "string.h"
#include <math.h>
#define PI 3.14159265

#include "main.h"
#include "led.h"
#include "usart.h"
#include "usart2.h"
#include "lcd_spi_169.h"
#include "sdmmc_sd.h"
#include "sdram.h"
#include "tim5.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "ff.h"
#include "ffconf.h"
#include "diskio.h"

#include "lvgl.h" 
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "../generated/gui_guider.h"
#include "../generated/events_init.h"

#include "link_list.h"

#include "shell.h"
#include "shell_port.h"
extern unsigned char recv_uart_buf;

void FreeRTOS_Start(void);
void sys_init(void);
#endif





