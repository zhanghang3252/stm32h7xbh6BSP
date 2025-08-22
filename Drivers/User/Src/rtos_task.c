#include "rtos_task.h"
Shell shell;//shell结构体
lv_ui guider_ui;//lvgl页面
LinkList *lvgl_link_list;//任务页面链表
#define Task_num 10//任务数量
BaseType_t task_Returned[Task_num];//任务状态
TaskHandle_t task_Handle[Task_num];//任务句柄

//函数声明
void get_thread_state(void);
void fatfs_init(void);
void sdram_init( void );
void led_TaskCode( void * pvParameters );
void send_usart_TaskCode( void * pvParameters );
void recv_usart_TaskCode( void * pvParameters );
void lvgl_TaskCode(void * pvParameters);
void encoder_TaskCode(void * pvParameters);
void lvgl_tick_TaskCode(void * pvParameters);
void fatfs_log(void);
//变量
#define	NumOf_Blocks	64
#define Test_BlockSize  ((BLOCKSIZE * NumOf_Blocks) >> 2)	 //定义数据大小,SD块大小为512字节，因为是32位的数组，所以这里除以4
#define	Test_Addr 	 	0x00
#define SDRAM_Size 		32*1024*1024  //32M字节

extern char *send_uart_buf;
unsigned char use_letter_flag=1;//是否使能letter_shell
unsigned char lvlg_indev_recv_flag=0;//串口输入标志
unsigned char recv_uart1_buf=0;
unsigned char recv_uart2_buf=0;


//信号量/互斥锁
SemaphoreHandle_t usart_Semaphore;//串口互斥锁
SemaphoreHandle_t lvgl_Semaphore;//lvgl互斥锁

void FreeRTOS_Start( void )
{
	sys_init();
	
	//创建互斥锁
	usart_Semaphore=xSemaphoreCreateMutex();
	lvgl_Semaphore=xSemaphoreCreateMutex();
	
	//创建线程
	task_Returned[0] = xTaskCreate(led_TaskCode,"led_Task",128,NULL,1,&task_Handle[0] );
	//task_Returned[1] = xTaskCreate(send_usart_TaskCode,"send_usart_Task",128,NULL,1,&task_Handle[1] );
	task_Returned[2] = xTaskCreate(recv_usart_TaskCode,"recv_usart_Task",128,NULL,1,&task_Handle[2] );
	task_Returned[3] = xTaskCreate(lvgl_tick_TaskCode,"lvgl_tick_Task",512,NULL,2,&task_Handle[3] );
	task_Returned[5] = xTaskCreate(lvgl_TaskCode,"lvgl_Task",2048,NULL,2,&task_Handle[5]);
	//task_Returned[6] = xTaskCreate	(encoder_TaskCode,"encoder_Task",256,NULL,1,&task_Handle[6] );

	vTaskStartScheduler();//开始调度
}

void sys_init(void)
{
	LED_Init();						// 初始化LED引脚
	USART1_Init();				// USART1初始化
	MX_USART2_UART_Init();//USART2初始化 
	MX_FMC_Init();				// SDRAM初始化
	MX_TIM4_Init();				// tim4旋转编码器接口测试
	
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	
	
	fatfs_init();
	sdram_init();
	
	HAL_UART_Receive_IT(&huart1, (uint8_t*)&recv_uart1_buf, 1);
	userShellInit(&shell);
}

//线程状态
void get_thread_state(void)
{
	char task_info[100];
	vTaskList(task_info);
	xSemaphoreTake(usart_Semaphore,portMAX_DELAY);
	printf("name\tstate\tpriority\tstack\tnum\t\r\n");
	printf("%s\r\n",task_info);
	xSemaphoreGive(usart_Semaphore);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), get_thread_state, get_thread_state, "打印任务状态");

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
		HAL_UART_Receive(&huart1,&recv_uart1_buf,1,0xFFFF);
		vTaskDelay(1);
	}
}

// 串口接收回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
			int res = taskENTER_CRITICAL_FROM_ISR();//进入临界区
			
			HAL_UART_Receive_IT(&huart1,&recv_uart1_buf, 1);//接收一个字节
			//printf("recv_uart1_buf%d\t%c\r\n",recv_uart1_buf,recv_uart1_buf);
			lvlg_indev_recv_flag = 1;//终端接收标志位
			if(use_letter_flag == 1)
			{
				shellHandler(&shell, recv_uart1_buf); //letter shell回调函数
				lvlg_indev_recv_flag = 0;//终端接收标志位
			}			
			if(recv_uart1_buf == 27 && use_letter_flag == 0)//ESC按键
			{
				use_letter_flag = 1;//启用shell	
				printf("use letter shell>>>\r\n");
			}			
			taskEXIT_CRITICAL_FROM_ISR(res);//退出临界区	
    }
		
	  if(huart->Instance == USART2)
    {
       HAL_UART_Receive_IT(&huart2,&recv_uart2_buf,1);
		}
}

void lvgl_tick_TaskCode(void * pvParameters)
{
	while(1)
	{
		xSemaphoreTake(lvgl_Semaphore,portMAX_DELAY);
		lv_task_handler();
		lv_timer_handler();
		xSemaphoreGive(lvgl_Semaphore);
		vTaskDelay(5);
	}
}

void lvgl_TaskCode(void * pvParameters){
	
	xSemaphoreTake(lvgl_Semaphore,portMAX_DELAY);
	setup_ui(&guider_ui);
	events_init(&guider_ui);
	lvgl_link_list = link_list_creat(5);//创建3个普通节点，头节点和尾节点，一共五个节点
	link_list_change(lvgl_link_list,0,guider_ui.timer_scr,guider_ui.timer_scr_del,setup_scr_timer_scr);//插入链表内容
	link_list_change(lvgl_link_list,1,guider_ui.led_scr,guider_ui.led_scr_del,setup_scr_led_scr);//插入链表内容
	link_list_change(lvgl_link_list,2,guider_ui.tz_scr,guider_ui.tz_scr_del,setup_scr_tz_scr);//插入链表内容
	link_list_change(lvgl_link_list,3,guider_ui.watch_scr,guider_ui.watch_scr_del,setup_scr_watch_scr);//插入链表内容
	link_list_change(lvgl_link_list,4,guider_ui.control_scr,guider_ui.control_scr_del,setup_scr_control_scr);//插入链表内容
	
	xSemaphoreGive(lvgl_Semaphore);
	while(1){
		xSemaphoreTake(lvgl_Semaphore,portMAX_DELAY);
//		lv_indev_wait_release(lv_indev_get_act());
//		ui_load_scr_animation(&guider_ui, &lvgl_link_list->next->lvgl_scr, lvgl_link_list->next->lvgl_scr_del, 
//		&lvgl_link_list->lvgl_scr_del,lvgl_link_list->next->setup_scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false, false);
//		lvgl_link_list=lvgl_link_list->next;//下一个节点
		
		xSemaphoreGive(lvgl_Semaphore);
		vTaskDelay(1000);
	}	
}

//旋转编码器任务
void encoder_TaskCode(void * pvParameters)
{
	unsigned int count=1,direction=0,temp=0;
	HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_ALL);//开启通道
	while(1)
	{
		count=__HAL_TIM_GET_COUNTER(&htim4);//获取计数值
		direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4);//获得旋转方向
		if(temp!=count)
		{
			printf("count :[%d],direction:[%d]",count,direction);
		}
		vTaskDelay(1);
	}
}


void fatfs_init(void)
{
	FATFS 	SD_FatFs; 		// 文件系统对象
	FRESULT MyFile_Res;    // 操作结果 
	uint16_t BufferSize = 0;
	FIL	MyFile;			// 文件对象
	UINT 	MyFile_Num;		//	数据长度
	BYTE 	MyFile_WriteBuffer[] = "hello,world";	//要写入的数据
	BYTE 	MyFile_ReadBuffer[512];	//要读出的数据	
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
		{
			printf("格式化失败，请检查或更换SD卡！retutn res[%d]\r\n",MyFile_Res);
		}
	}
	
	
	printf("-------------FatFs 文件创建和写入测试---------------\r\n");
	
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_CREATE_ALWAYS | FA_WRITE);	//打开文件，若不存在则创建该文件
	if(MyFile_Res == FR_OK)
	{
		printf("文件打开/创建成功，准备写入数据...\r\n");
		
		MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//向文件写入数据
		if (MyFile_Res == FR_OK)	
		{
			printf("写入成功，写入内容为：");
			printf("%s\r\n",MyFile_WriteBuffer);
		}
		else
		{
			printf("文件写入失败，请检查SD卡或重新格式化!retutn res[%d]\r\n",MyFile_Res);
			f_close(&MyFile);	  //关闭文件			
		}
		f_close(&MyFile);	  //关闭文件			
	}
	else
	{
		printf("无法打开/创建文件，请检查SD卡或重新格式化!retutn res[%d]\r\n",MyFile_Res);
		f_close(&MyFile);	  //关闭文件		
	}
	
	printf("-------------FatFs 文件读取测试---------------\r\n");	
	
	BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// 计算写入的数据长度
	MyFile_Res = f_open(&MyFile,"0:a.txt",FA_OPEN_EXISTING | FA_READ);	//打开文件，若不存在则创建该文件
	MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// 读取文件
	if(MyFile_Res == FR_OK)
	{
		printf("文件读取成功，正在校验数据...\r\n");
		
		for(int i=0;i<BufferSize;i++)
		{
			if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// 校验数据
			{
				printf("校验失败，请检查SD卡或重新格式化!retutn res[%d]\r\n",MyFile_Res);
				f_close(&MyFile);	  //关闭文件	
			}
		}
		printf("校验成功，读出的数据为：");
		printf("%s\r\n",MyFile_ReadBuffer);
	}	
	else
	{
		printf("无法读取文件，请检查SD卡或重新格式化!return res[%d]\r\n",MyFile_Res);
		f_close(&MyFile);	  //关闭文件		
	}	
	
	f_close(&MyFile);	  //关闭文件	
}

void sdram_init( void )
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

