#include "rtos_task.h"
//�߳�״̬
BaseType_t Returned[10];

//�߳̾��
TaskHandle_t ledHandle = NULL;
TaskHandle_t usartHandle = NULL;
TaskHandle_t lcdtHandle = NULL;
TaskHandle_t device_testingramtHandle = NULL;
//��������
void sd_test(void);
void sdram_test( void );
//�̺߳���
void led_TaskCode( void * pvParameters );
void usart_TaskCode( void * pvParameters );
void lcd_TaskCode( void * pvParameters );
void device_testing_TaskCode(void * pvParameters);

//����
#define	NumOf_Blocks	64
#define Test_BlockSize  ((BLOCKSIZE * NumOf_Blocks) >> 2)	 //�������ݴ�С,SD���СΪ512�ֽڣ���Ϊ��32λ�����飬�����������4
#define	Test_Addr 	 0x00
//#define SDRAM_Size 16*1024*1024  //16M�ֽ�
#define SDRAM_Size 32*1024*1024  //32M�ֽ�

uint32_t SD_Status ; 		 //SD������־λ
uint32_t SD_WriteBuffer[Test_BlockSize];	//	д��������
uint32_t SD_ReadBuffer[Test_BlockSize];	//	����������

void FreeRTOS_Start( void )
{
	sys_init();
	Returned[0] = xTaskCreate(led_TaskCode,"led_Task",64,NULL,1,&ledHandle );
	Returned[1] = xTaskCreate(usart_TaskCode,"usart_Task",128,NULL,2,&usartHandle );
	Returned[2] = xTaskCreate(lcd_TaskCode,"lcd_Task",128,NULL,3,&lcdtHandle );
	Returned[3] = xTaskCreate(device_testing_TaskCode,"device_testing",2048,NULL,4,&device_testingramtHandle );
	printf("thread state:\t");//��ӡ�߳�״̬
	for(int i=0;i<9;i++)
		printf("%ld\t",Returned[i]);//��ӡ�߳�״̬
	printf("\r\n");
	vTaskStartScheduler();
}

void sys_init(void)
{
	LED_Init();					// ��ʼ��LED����
	USART1_Init();				// USART1��ʼ��
	SPI_LCD_Init();     	 	// Һ�����Լ�SPI��ʼ�� 

	SD_Status = BSP_SD_Init(SD_Instance);	//SD����ʼ��
	if( SD_Status == BSP_ERROR_NONE )	//����Ƿ��ʼ���ɹ�
	{		
		printf("SD����ʼ���ɹ� \r\n");	
	}
	else
	printf("��ⲻ��SD����ERROR: %d\r\n",SD_Status);	
	MX_FMC_Init();				// SDRAM��ʼ��
}

void led_TaskCode( void * pvParameters )
{
		
    while(1)
    {
			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
			vTaskDelay(500);
    }
}

void usart_TaskCode( void * pvParameters )
{
	while(1)
	{
		vTaskDelay(1);
	}
}

void lcd_TaskCode( void * pvParameters )
{
	unsigned	int count=0;  
	while(1)
	{
		count++;
		LCD_DisplayString(20,1,"stm32h750xbh6>>");
		LCD_DisplayNumber(20,30,count,6);
		vTaskDelay(10);
	}
}
void device_testing_TaskCode(void * pvParameters)
{
	sd_test();
	sdram_test();
	while(1)
	{
		vTaskDelay(1);
	}
}
void sd_test(void)
{
		uint32_t i = 0;	
		uint32_t ExecutionTime_Begin;		// ��ʼʱ��
		uint32_t ExecutionTime_End;		// ����ʱ��
		uint32_t ExecutionTime;				// ִ��ʱ��	
		float    ExecutionSpeed;			// ִ���ٶ�
		printf ("SD������>>>\r\n");
		// ���� >>>>>>>    
		ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
		SD_Status = BSP_SD_Erase(SD_Instance,Test_Addr, NumOf_Blocks);
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
		ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms

		ExecutionTime = ExecutionTime_End - ExecutionTime_Begin; // �������ʱ�䣬��λms

		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("�����ɹ�, ��������ʱ��: %d ms\r\n",ExecutionTime);		
		}
		else
		{
			printf ("����ʧ��!!!!!  �������:%d\r\n",SD_Status);
			while (1);
		}		

		// д�� >>>>>>>    
		for(i=0;i<Test_BlockSize;i++)	//��Ҫд��SD��������д������
		{
			SD_WriteBuffer[i] = i;
		}

		ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms	
		SD_Status = BSP_SD_WriteBlocks(SD_Instance,SD_WriteBuffer, Test_Addr, NumOf_Blocks);	//��д��	
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
		ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms

		ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 		// �������ʱ�䣬��λms
		ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime /1024 ; // ����д���ٶȣ���λ MB/S	
		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("д��ɹ�,���ݴ�С��%d KB, ��ʱ: %d ms, д���ٶȣ�%.2f MB/s\r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
		}
		else
		{
			printf ("д�����!!!!!  �������:%d\r\n",SD_Status);
			while (1);
		}		


		// ��ȡ >>>>>>>    
		ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms		
		SD_Status = BSP_SD_ReadBlocks(SD_Instance,SD_ReadBuffer, Test_Addr, NumOf_Blocks);	//���ȡ
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
		ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms

		ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 						// �������ʱ�䣬��λms
		ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime / 1024 ; 	// �����ȡ�ٶȣ���λ MB/S 

		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("��ȡ�ɹ�,���ݴ�С��%d KB, ��ʱ: %d ms, ��ȡ�ٶȣ�%.2f MB/s \r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
		}
		else
		{
			printf ("��ȡ����!!!!!  �������:%d\r\n",SD_Status);
		while (1);
		}		

		// У�� >>>>>>>   
		for(i=0;i<Test_BlockSize;i++)	//��֤�����������Ƿ����д�������
		{
		if( SD_WriteBuffer[i] != SD_ReadBuffer[i] )	//������ݲ���ȣ��򷵻�0	
		{
			printf ("����У��ʧ��!!!!!\r\n");	
			while(1);
		}
		}		
		printf ("У��ͨ��!!!!!SD����������\r\n");		
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
				
	printf("��������У��>>>\r\n");
	
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

