#ifndef __RTOS_TASK_H
#define __RTOS_TASK_H
#include "stdio.h"
#include "string.h"

#include "main.h"
#include "led.h"
#include "usart.h"
#include "usart2.h"
#include "lcd_spi_169.h"
#include "sdmmc_sd.h"
#include "sdram.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "ff.h"
#include "ffconf.h"
#include "diskio.h"

void FreeRTOS_Start(void);
void sys_init(void);
#endif





