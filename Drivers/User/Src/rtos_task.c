#include "rtos_task.h"
#define Task_num 10
//�߳�״̬
BaseType_t Returned[Task_num];

//�߳̾��
TaskHandle_t ledHandle = NULL;
TaskHandle_t send_usartHandle = NULL;
TaskHandle_t recv_usartHandle = NULL;
TaskHandle_t device_testingramtHandle = NULL;
TaskHandle_t bleHandle = NULL;
TaskHandle_t lvgl_Handle= NULL;
//��������
void get_thread_state(void);
void fatfs_test(void);
void sdram_test( void );
void led_TaskCode( void * pvParameters );
void send_usart_TaskCode( void * pvParameters );
void recv_usart_TaskCode( void * pvParameters );
void device_testing_TaskCode(void * pvParameters);
void lvgl_TaskCode(void * pvParameters);
//����
#define	NumOf_Blocks	64
#define Test_BlockSize  ((BLOCKSIZE * NumOf_Blocks) >> 2)	 //�������ݴ�С,SD���СΪ512�ֽڣ���Ϊ��32λ�����飬�����������4
#define	Test_Addr 	 	0x00
#define SDRAM_Size 		32*1024*1024  //32M�ֽ�


unsigned char recv_uart_buf=0;
unsigned char recv_uart2_buf=0;

FATFS 	SD_FatFs; 		// �ļ�ϵͳ����
FRESULT 	MyFile_Res;    // ������� 
uint16_t BufferSize = 0;	
FIL	MyFile;			// �ļ�����
UINT 	MyFile_Num;		//	���ݳ���
BYTE 	MyFile_WriteBuffer[] = "hello,world";	//Ҫд�������
BYTE 	MyFile_ReadBuffer[512];	//Ҫ����������

//�ź���/������
SemaphoreHandle_t usart_Semaphore;//�������ڻ�����
SemaphoreHandle_t lvgl_Semaphore;//�������ڻ�����

void FreeRTOS_Start( void )
{
	sys_init();
	//����������
	usart_Semaphore=xSemaphoreCreateMutex();
	lvgl_Semaphore=xSemaphoreCreateMutex();
	//�����߳�
	Returned[0] = xTaskCreate(led_TaskCode,"led_Task",128,NULL,1,&ledHandle );
	Returned[1] = xTaskCreate(send_usart_TaskCode,"send_usart_Task",128,NULL,1,&send_usartHandle );
	Returned[2] = xTaskCreate(recv_usart_TaskCode,"recv_usart_Task",128,NULL,1,&recv_usartHandle );
	Returned[4] = xTaskCreate(device_testing_TaskCode,"device_testing",2048,NULL,9,&device_testingramtHandle );
	Returned[6] = xTaskCreate	(lvgl_TaskCode,"lvgl_Task",2048,NULL,2,&lvgl_Handle );
	
	get_thread_state();
	
	vTaskStartScheduler();//��ʼ����
}

void sys_init(void)
{
	LED_Init();					// ��ʼ��LED����
	USART1_Init();				// USART1��ʼ��
	MX_USART2_UART_Init();//USART2��ʼ�� 
	MX_FMC_Init();				// SDRAM��ʼ��
	
	lv_init();
	lv_port_disp_init();
}

////�߳�״̬
void get_thread_state(void)
{
	printf("thread state:\t");//��ӡ�߳�״̬
	for(int i=0;i<Task_num;i++)
		printf("%ld\t",Returned[i]);//��ӡ�߳�״̬
	printf("\r\n");
}

void led_TaskCode( void * pvParameters )
{
    while(1)
    {
			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
			vTaskDelay(1000);
    }
}

void send_usart_TaskCode( void * pvParameters )
{
	while(1)
	{
		xSemaphoreTake(usart_Semaphore,portMAX_DELAY);
		
		printf("systick:[%d]\r\n",HAL_GetTick());
		
		xSemaphoreGive(usart_Semaphore);
		
		vTaskDelay(1000);
	}
}

void recv_usart_TaskCode( void * pvParameters )
{
	while(1)
	{
		HAL_UART_Receive(&huart1,&recv_uart_buf,1,0xFFFF);
		
		xSemaphoreTake(usart_Semaphore,portMAX_DELAY);
		
		printf("recv_uart_buf:[%d]\r\n",recv_uart_buf);
		printf("recv_uart_buf:[%c]\r\n",recv_uart_buf);
		
		xSemaphoreGive(usart_Semaphore);
		vTaskDelay(1);
	}
}

// ���ڽ��ջص�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1,&recv_uart_buf, 1);
    }
	  if(huart->Instance == USART2)
    {
       HAL_UART_Receive_IT(&huart2,&recv_uart2_buf,1);
		}
}

void device_testing_TaskCode(void * pvParameters)
{
	xSemaphoreTake(usart_Semaphore,portMAX_DELAY);
	fatfs_test();
	sdram_test();
	xSemaphoreGive(usart_Semaphore);
	while(1)
	{
		vTaskDelay(1000);
	}
}

void lvgl_TaskCode(void * pvParameters)
{
	xSemaphoreTake(lvgl_Semaphore,portMAX_DELAY);
	
//	lv_obj_t* btn = lv_btn_create(lv_scr_act()); // ����Button����
//	lv_obj_t* label = lv_label_create(btn); // ����Button���󴴽�Label����
//	lv_obj_set_size(btn, 100, 50); // ���ö����С,��Ⱥ͸߶�
//	lv_obj_set_pos(btn,20,20);
//	lv_label_set_text(label, "open"); // ������ʾ����
//	lv_obj_center(label); // ���������ʾ
//	lv_demo_widgets();
	lv_demo_benchmark();
	xSemaphoreGive(lvgl_Semaphore);
	while(1)
	{
		xSemaphoreTake(lvgl_Semaphore,portMAX_DELAY);
		lv_task_handler();
		lv_timer_handler();
		xSemaphoreGive(lvgl_Semaphore);
		vTaskDelay(5);
	}	
}

void fatfs_test(void)
{
	BYTE work[FF_MAX_SS]; 
	
	MyFile_Res = f_mount(&SD_FatFs,"0:",1);	//	����SD��
	
	if (MyFile_Res == FR_OK)	//�ж��Ƿ���سɹ�
	{
		printf("\r\nSD�ļ�ϵͳ���سɹ�\r\n");
	}
	else		
	{
		printf("SD����δ�����ļ�ϵͳ��������ʽ��\r\n");
		
		MyFile_Res = f_mkfs("0:",NULL,work,sizeof (work));
		
		if (MyFile_Res == FR_OK)		//�ж��Ƿ��ʽ���ɹ�
		{
			printf("SD����ʽ���ɹ���\r\n");
			MyFile_Res = f_mount(NULL,"0:",1);	//	����SD��
			MyFile_Res = f_mount(&SD_FatFs,"0:",1);	//	����SD��
			printf("mount retutn res[%d]\r\n",MyFile_Res);
		}
		else
		{
			printf("��ʽ��ʧ�ܣ���������SD����retutn res[%d]\r\n",MyFile_Res);
		}
	}
	
	
	printf("-------------FatFs �ļ�������д�����---------------\r\n");
	
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_CREATE_ALWAYS | FA_WRITE);	//���ļ������������򴴽����ļ�
	if(MyFile_Res == FR_OK)
	{
		printf("�ļ���/�����ɹ���׼��д������...\r\n");
		
		MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//���ļ�д������
		if (MyFile_Res == FR_OK)	
		{
			printf("д��ɹ���д������Ϊ��");
			printf("%s\r\n",MyFile_WriteBuffer);
		}
		else
		{
			printf("�ļ�д��ʧ�ܣ�����SD�������¸�ʽ��!retutn res[%d]\r\n",MyFile_Res);
			f_close(&MyFile);	  //�ر��ļ�			
		}
		f_close(&MyFile);	  //�ر��ļ�			
	}
	else
	{
		printf("�޷���/�����ļ�������SD�������¸�ʽ��!retutn res[%d]\r\n",MyFile_Res);
		f_close(&MyFile);	  //�ر��ļ�		
	}
	
	printf("-------------FatFs �ļ���ȡ����---------------\r\n");	
	
	BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// ����д������ݳ���
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_OPEN_EXISTING | FA_READ);	//���ļ������������򴴽����ļ�
	MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// ��ȡ�ļ�
	if(MyFile_Res == FR_OK)
	{
		printf("�ļ���ȡ�ɹ�������У������...\r\n");
		
		for(int i=0;i<BufferSize;i++)
		{
			if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// У������
			{
				printf("У��ʧ�ܣ�����SD�������¸�ʽ��!retutn res[%d]\r\n",MyFile_Res);
				f_close(&MyFile);	  //�ر��ļ�	
			}
		}
		printf("У��ɹ�������������Ϊ��");
		printf("%s\r\n",MyFile_ReadBuffer);
	}	
	else
	{
		printf("�޷���ȡ�ļ�������SD�������¸�ʽ��!return res[%d]\r\n",MyFile_Res);
		f_close(&MyFile);	  //�ر��ļ�		
	}	
	
	f_close(&MyFile);	  //�ر��ļ�	
}

void sdram_test( void )
{
	uint32_t i = 0;			// ��������
	uint16_t ReadData = 0; 	// ��ȡ��������
	uint8_t  ReadData_8b;

	uint32_t ExecutionTime_Begin;		// ��ʼʱ��
	uint32_t ExecutionTime_End;		// ����ʱ��
	uint32_t ExecutionTime;				// ִ��ʱ��	
	float    ExecutionSpeed;			// ִ���ٶ�
	printf ("SDRAM����>>>\r\n");	
	printf("�����ٶȲ���\r\n");

// д�� >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
	
	for (i = 0; i < SDRAM_Size/2; i++)
	{
 		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*i) = (uint16_t)i;		// д������
	}
	ExecutionTime_End		= HAL_GetTick();											// ��ȡ systick ��ǰʱ�䣬��λms
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 				// �������ʱ�䣬��λms
	ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ; 	// �����ٶȣ���λ MB/S	
	
	printf("��16λ���ݿ��д�����ݣ���С��%d MB����ʱ: %d ms, д���ٶȣ�%.2f MB/s\r\n",SDRAM_Size/1024/1024,ExecutionTime,ExecutionSpeed);

// ��ȡ	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 

	ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
	
	for(i = 0; i < SDRAM_Size/2;i++ )
	{
		ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // ��SDRAM��������	
	}
	ExecutionTime_End		= HAL_GetTick();											// ��ȡ systick ��ǰʱ�䣬��λms
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 				// �������ʱ�䣬��λms
	ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ; 	// �����ٶȣ���λ MB/S	
	
	printf("��ȡ������ϣ���С��%d MB����ʱ: %d ms, ��ȡ�ٶȣ�%.2f MB/s\r\n",SDRAM_Size/1024/1024,ExecutionTime,ExecutionSpeed);
	
// ����У�� >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   
				
	printf("��������У��\r\n");
	
	for(i = 0; i < SDRAM_Size/2;i++ )
	{
		ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // ��SDRAM��������	
		if( ReadData != (uint16_t)i )      //������ݣ�������ȣ���������,���ؼ��ʧ�ܽ����
		{
			printf("SDRAM����ʧ�ܣ���\r\n");
		}
	}
	
	printf("16λ���ݿ�ȶ�дͨ������8λ���ݿ��д������\r\n");
	for (i = 0; i < 255; i++)
	{
 		*(__IO uint8_t*) (SDRAM_BANK_ADDR + i) =  (uint8_t)i;
	}	
	printf("д����ϣ���ȡ���ݲ��Ƚ�...\r\n");
	for (i = 0; i < 255; i++)
	{
		ReadData_8b = *(__IO uint8_t*) (SDRAM_BANK_ADDR + i);
		if( ReadData_8b != (uint8_t)i )      //������ݣ�������ȣ���������,���ؼ��ʧ�ܽ����
		{
			printf("8λ���ݿ�ȶ�д����ʧ�ܣ���\r\n");
			printf("����NBL0��NBL1������\r\n");	
		}
	}		
	printf("8λ���ݿ�ȶ�дͨ��\r\n");
	printf("SDRAM��д����ͨ����ϵͳ����\r\n");
}

