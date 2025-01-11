#include "rtos_task.h"
#define Task_num 10
Free_window main_win;
//线程状态
BaseType_t Returned[Task_num];

//线程句柄
TaskHandle_t ledHandle = NULL;
TaskHandle_t usartHandle = NULL;
TaskHandle_t recv_usartHandle = NULL;
TaskHandle_t lcdtHandle = NULL;
TaskHandle_t device_testingramtHandle = NULL;
TaskHandle_t fatfsHandle = NULL;
//函数定义
void sd_test(void);
void sdram_test( void );
//函数声明
void led_TaskCode( void * pvParameters );
void usart_TaskCode( void * pvParameters );
void recv_usart_TaskCode( void * pvParameters );
void lcd_TaskCode( void * pvParameters );
void device_testing_TaskCode(void * pvParameters);
void usart_timer_CallbackFunction( TimerHandle_t xTimer );
void fatfs_TaskCode(void * pvParameters);
void fatfs_test(void);
//变量
#define	NumOf_Blocks	64
#define Test_BlockSize  ((BLOCKSIZE * NumOf_Blocks) >> 2)	 //定义数据大小,SD块大小为512字节，因为是32位的数组，所以这里除以4
#define	Test_Addr 	 0x00
//#define SDRAM_Size 16*1024*1024  //16M字节
#define SDRAM_Size 32*1024*1024  //32M字节

uint32_t SD_Status ; 		 //SD卡检测标志位
uint32_t SD_WriteBuffer[Test_BlockSize];	//	写数据数组
uint32_t SD_ReadBuffer[Test_BlockSize];	//	读数据数组

unsigned char recv_uart_buf=0;

FATFS 	SD_FatFs; 		// 文件系统对象
FRESULT 	MyFile_Res;    // 操作结果 
char SDPath[4];			// SD卡逻辑驱动路径
BYTE workbuffer[512];
uint8_t i = 0;
uint16_t BufferSize = 0;	
FIL	MyFile;			// 文件对象
UINT 	MyFile_Num;		//	数据长度
BYTE 	MyFile_WriteBuffer[] = "abc";	//要写入的数据
BYTE 	MyFile_ReadBuffer[1024];	//要读出的数据
//信号量/互斥锁
SemaphoreHandle_t usart_Semaphore;//创建串口互斥锁
SemaphoreHandle_t lcd_Semaphore;//创建互斥锁

//定时器
TimerHandle_t usart_Timers;
void FreeRTOS_Start( void )
{
	sys_init();
	fatfs_test();
	//创建互斥锁
	usart_Semaphore=xSemaphoreCreateMutex();
	
//	//创建定时器
//	usart_Timers = xTimerCreate("usart_Timers",pdMS_TO_TICKS(100),pdTRUE,0,usart_timer_CallbackFunction);
	
	//创建线程
	Returned[0] = xTaskCreate(led_TaskCode,"led_Task",64,NULL,31,&ledHandle );
	Returned[1] = xTaskCreate(usart_TaskCode,"usart_Task",128,NULL,2,&usartHandle );
	Returned[2] = xTaskCreate(recv_usart_TaskCode,"recv_usart_Task",128,NULL,2,&usartHandle );
	Returned[3] = xTaskCreate(lcd_TaskCode,"lcd_Task",256,NULL,3,&lcdtHandle );
	Returned[4] = xTaskCreate(device_testing_TaskCode,"device_testing",2048,NULL,4,&device_testingramtHandle );
	Returned[5] = xTaskCreate(fatfs_TaskCode,"fatfs_Task_Task",512,NULL,30,&fatfsHandle );
	
	printf("thread state:\t");//打印线程状态
	for(int i=0;i<Task_num;i++)
		printf("%ld\t",Returned[i]);//打印线程状态
	printf("\r\n");
	vTaskStartScheduler();
}
////定时器回调函数
//void usart_timer_CallbackFunction( TimerHandle_t xTimer )
//{
//	printf("定时器时间到\r\n");
//}

void sys_init(void)
{
	LED_Init();					// 初始化LED引脚
	USART1_Init();				// USART1初始化
	SPI_LCD_Init();     	 	// 液晶屏以及SPI初始化 

	SD_Status = BSP_SD_Init(SD_Instance);	//SD卡初始化
	if( SD_Status == BSP_ERROR_NONE )	//检测是否初始化成功
	{		
		printf("SD卡初始化成功 \r\n");	
	}
	else
	printf("检测不到SD卡，ERROR: %d\r\n",SD_Status);	
	MX_FMC_Init();				// SDRAM初始化
}

void led_TaskCode( void * pvParameters )
{
    while(1)
    {
			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
			vTaskDelay(100);
    }
}

void usart_TaskCode( void * pvParameters )
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
		printf("recv_uart_buf:[%d]\r\n",recv_uart_buf);
		printf("recv_uart_buf:[%c]\r\n",recv_uart_buf);
		vTaskDelay(1);
	}
}
// 串口接收回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1,&recv_uart_buf, 1);
    }
}

void lcd_TaskCode( void * pvParameters )
{ 
	main_win.width=100;
	main_win.height=100;
	main_win.x=100;
	main_win.y=100;
	uint32_t count=0;
	while(1)
	{
		FreeGUI_Handler(&main_win);
		count++;
		LCD_DisplayNumber(20,1,count,6);
		vTaskDelay(1000);
	}
}

void device_testing_TaskCode(void * pvParameters)
{
	xSemaphoreTake(usart_Semaphore,portMAX_DELAY);
	sd_test();
	sdram_test();
	xSemaphoreGive(usart_Semaphore);
	while(1)
	{
		vTaskDelay(1);
	}
}

void fatfs_TaskCode(void * pvParameters)
{
	fatfs_test();
	while(1)
	{
		vTaskDelay(10);
	}
}

void fatfs_test(void)
{
	BYTE work[FF_MAX_SS]; 
	
	MyFile_Res = f_mount(&SD_FatFs,"0:",1);	//	挂载SD卡
	
	if (MyFile_Res == FR_OK)	//判断是否挂载成功
	{
		printf("\r\nSD文件系统挂载成功\r\n");
	}
	else		
	{
		printf("SD卡还未创建文件系统，即将格式化\r\n");
		
		MyFile_Res = f_mkfs("0:",NULL,work,sizeof (work));
		
		if (MyFile_Res == FR_OK)		//判断是否格式化成功
		{
			printf("SD卡格式化成功！\r\n");
			MyFile_Res = f_mount(NULL,"0:",1);	//	挂载SD卡
			MyFile_Res = f_mount(&SD_FatFs,"0:",1);	//	挂载SD卡
			printf("mount retutn res[%d]\r\n",MyFile_Res);
		}

		else
			printf("格式化失败，请检查或更换SD卡！\r\n");
	}
	
	
	printf("-------------FatFs 文件创建和写入测试---------------\r\n");
	
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_CREATE_ALWAYS | FA_WRITE);	//打开文件，若不存在则创建该文件
	if(MyFile_Res == FR_OK)
	{
		printf("文件打开/创建成功，准备写入数据...\r\n");
		
		MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//向文件写入数据
		if (MyFile_Res == FR_OK)	
		{
			printf("写入成功，写入内容为：\r\n");
			MyFile_Res=f_sync(&MyFile);
			printf("f_sync return (%d)\r\n",MyFile_Res);
			printf("%s\r\n",MyFile_WriteBuffer);
		}
		else
		{
			printf("文件写入失败，请检查SD卡或重新格式化!\r\n");
			f_close(&MyFile);	  //关闭文件			
		}
		f_close(&MyFile);	  //关闭文件			
	}
	else
	{
		printf("无法打开/创建文件，请检查SD卡或重新格式化!\r\n");
		f_close(&MyFile);	  //关闭文件		
	}
	
	printf("-------------FatFs 文件读取测试---------------\r\n");	
	
	BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// 计算写入的数据长度
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_OPEN_EXISTING | FA_READ);	//打开文件，若不存在则创建该文件
	printf("open return res %d\r\n",MyFile_Res);
	MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// 读取文件
	if(MyFile_Res == FR_OK)
	{
		printf("文件读取成功，正在校验数据...\r\n");
		
		for(i=0;i<BufferSize;i++)
		{
			if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// 校验数据
			{
				printf("校验失败，请检查SD卡或重新格式化!\r\n");
				f_close(&MyFile);	  //关闭文件	
			}
		}
		printf("校验成功，读出的数据为：\r\n");
		printf("%s\r\n",MyFile_ReadBuffer);
	}	
	else
	{
		printf("无法读取文件，请检查SD卡或重新格式化![%d]\r\n",MyFile_Res	);
		f_close(&MyFile);	  //关闭文件		
	}	
	
	f_close(&MyFile);	  //关闭文件	
}
void sd_test(void)
{
		uint32_t i = 0;	
		uint32_t ExecutionTime_Begin;		// 开始时间
		uint32_t ExecutionTime_End;		// 结束时间
		uint32_t ExecutionTime;				// 执行时间	
		float    ExecutionSpeed;			// 执行速度
		printf ("SD卡测试>>>\r\n");
		// 擦除 >>>>>>>    
		ExecutionTime_Begin 	= HAL_GetTick();	// 获取 systick 当前时间，单位ms
		SD_Status = BSP_SD_Erase(SD_Instance,Test_Addr, NumOf_Blocks);
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//等待通信结束	
		ExecutionTime_End		= HAL_GetTick();	// 获取 systick 当前时间，单位ms

		ExecutionTime = ExecutionTime_End - ExecutionTime_Begin; // 计算擦除时间，单位ms

		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("擦除成功, 擦除所需时间: %d ms\r\n",ExecutionTime);		
		}
		else
		{
			printf ("擦除失败!!!!!  错误代码:%d\r\n",SD_Status);
			while (1);
		}		

		// 写入 >>>>>>>    
		for(i=0;i<Test_BlockSize;i++)	//将要写入SD卡的数据写入数组
		{
			SD_WriteBuffer[i] = i;
		}

		ExecutionTime_Begin 	= HAL_GetTick();	// 获取 systick 当前时间，单位ms	
		SD_Status = BSP_SD_WriteBlocks(SD_Instance,SD_WriteBuffer, Test_Addr, NumOf_Blocks);	//块写入	
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//等待通信结束	
		ExecutionTime_End		= HAL_GetTick();	// 获取 systick 当前时间，单位ms

		ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 		// 计算擦除时间，单位ms
		ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime /1024 ; // 计算写入速度，单位 MB/S	
		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("写入成功,数据大小：%d KB, 耗时: %d ms, 写入速度：%.2f MB/s\r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
		}
		else
		{
			printf ("写入错误!!!!!  错误代码:%d\r\n",SD_Status);
			while (1);
		}		


		// 读取 >>>>>>>    
		ExecutionTime_Begin 	= HAL_GetTick();	// 获取 systick 当前时间，单位ms		
		SD_Status = BSP_SD_ReadBlocks(SD_Instance,SD_ReadBuffer, Test_Addr, NumOf_Blocks);	//块读取
		while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//等待通信结束	
		ExecutionTime_End		= HAL_GetTick();	// 获取 systick 当前时间，单位ms

		ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 						// 计算擦除时间，单位ms
		ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime / 1024 ; 	// 计算读取速度，单位 MB/S 

		if( SD_Status == BSP_ERROR_NONE )
		{
			printf ("读取成功,数据大小：%d KB, 耗时: %d ms, 读取速度：%.2f MB/s \r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
		}
		else
		{
			printf ("读取错误!!!!!  错误代码:%d\r\n",SD_Status);
		while (1);
		}		

		// 校验 >>>>>>>   
		for(i=0;i<Test_BlockSize;i++)	//验证读出的数据是否等于写入的数据
		{
		if( SD_WriteBuffer[i] != SD_ReadBuffer[i] )	//如果数据不相等，则返回0	
		{
			printf ("数据校验失败!!!!!\r\n");	
			while(1);
		}
		}		
		printf ("校验通过!!!!!SD卡测试正常\r\n");		
}

void sdram_test( void )
{
	uint32_t i = 0;			// 计数变量
	uint16_t ReadData = 0; 	// 读取到的数据
	uint8_t  ReadData_8b;

	uint32_t ExecutionTime_Begin;		// 开始时间
	uint32_t ExecutionTime_End;		// 结束时间
	uint32_t ExecutionTime;				// 执行时间	
	float    ExecutionSpeed;			// 执行速度
	printf ("SDRAM测试>>>\r\n");	
	printf("进行速度测试\r\n");

// 写入 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	ExecutionTime_Begin 	= HAL_GetTick();	// 获取 systick 当前时间，单位ms
	
	for (i = 0; i < SDRAM_Size/2; i++)
	{
 		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*i) = (uint16_t)i;		// 写入数据
	}
	ExecutionTime_End		= HAL_GetTick();											// 获取 systick 当前时间，单位ms
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 				// 计算擦除时间，单位ms
	ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ; 	// 计算速度，单位 MB/S	
	
	printf("以16位数据宽度写入数据，大小：%d MB，耗时: %d ms, 写入速度：%.2f MB/s\r\n",SDRAM_Size/1024/1024,ExecutionTime,ExecutionSpeed);

// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 

	ExecutionTime_Begin 	= HAL_GetTick();	// 获取 systick 当前时间，单位ms
	
	for(i = 0; i < SDRAM_Size/2;i++ )
	{
		ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // 从SDRAM读出数据	
	}
	ExecutionTime_End		= HAL_GetTick();											// 获取 systick 当前时间，单位ms
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 				// 计算擦除时间，单位ms
	ExecutionSpeed = (float)SDRAM_Size /1024/1024 /ExecutionTime*1000 ; 	// 计算速度，单位 MB/S	
	
	printf("读取数据完毕，大小：%d MB，耗时: %d ms, 读取速度：%.2f MB/s\r\n",SDRAM_Size/1024/1024,ExecutionTime,ExecutionSpeed);
	
// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   
				
	printf("进行数据校验\r\n");
	
	for(i = 0; i < SDRAM_Size/2;i++ )
	{
		ReadData = *(__IO uint16_t*)(SDRAM_BANK_ADDR + 2 * i );  // 从SDRAM读出数据	
		if( ReadData != (uint16_t)i )      //检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("SDRAM测试失败！！\r\n");
		}
	}
	
	printf("16位数据宽度读写通过，以8位数据宽度写入数据\r\n");
	for (i = 0; i < 255; i++)
	{
 		*(__IO uint8_t*) (SDRAM_BANK_ADDR + i) =  (uint8_t)i;
	}	
	printf("写入完毕，读取数据并比较...\r\n");
	for (i = 0; i < 255; i++)
	{
		ReadData_8b = *(__IO uint8_t*) (SDRAM_BANK_ADDR + i);
		if( ReadData_8b != (uint8_t)i )      //检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("8位数据宽度读写测试失败！！\r\n");
			printf("请检查NBL0和NBL1的连接\r\n");	
		}
	}		
	printf("8位数据宽度读写通过\r\n");
	printf("SDRAM读写测试通过，系统正常\r\n");
}

