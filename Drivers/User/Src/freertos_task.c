#include "freertos_task.h"

TaskHandle_t led_Handle = NULL;//led任务句柄

static void led_task_f(void *args);//led任务函数

void rtos_start(void)
{
	BaseType_t state;
	xTaskCreate(led_task_f,"led_task",128,NULL,2,&led_Handle);//创建任务
	if(state == pdPASS)	
	{
		vTaskDelete( led_Handle );//如果创建任务失败就删除任务已节省空间
		printf("led任务创建失败>>\r\n");
	}
	else 
		printf("led任务创建成功>>\r\n");		
	vTaskStartScheduler();
}
static void led_task_f(void *pvParameters)
{
	while(1)
	{
		HAL_GPIO_TogglePin(user_led_GPIO_Port,user_led_Pin);
		vTaskDelay(500);	
	}
}


