#include "freertos_task.h"

TaskHandle_t led_Handle = NULL;//led������

static void led_task_f(void *args);//led������

void rtos_start(void)
{
	BaseType_t state;
	xTaskCreate(led_task_f,"led_task",128,NULL,2,&led_Handle);//��������
	if(state == pdPASS)	
	{
		vTaskDelete( led_Handle );//�����������ʧ�ܾ�ɾ�������ѽ�ʡ�ռ�
		printf("led���񴴽�ʧ��>>\r\n");
	}
	else 
		printf("led���񴴽��ɹ�>>\r\n");		
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


