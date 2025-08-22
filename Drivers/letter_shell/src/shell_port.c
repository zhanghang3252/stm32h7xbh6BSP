/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */
#include "shell_port.h"


char shellBuffer[512];
Log shell_log;
static SemaphoreHandle_t shellMutex;

/**
 * @brief 用户shell写
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 0x1FF);
    return len;
}


/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len)
{
	   if(HAL_UART_Receive(&huart1, (uint8_t *)data, 1, 0xFFFF) != HAL_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
	int res = taskENTER_CRITICAL_FROM_ISR();//进入临界区

	xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);

	taskEXIT_CRITICAL_FROM_ISR(res);//退出临界区
    
    return 0;
}

/**
 * @brief 用户shell解锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{
	xSemaphoreGiveRecursive(shellMutex);
	return 0;
}
void LogWrite(char *buffer, short len)
{
		HAL_UART_Transmit(&huart1, (uint8_t *)buffer, len, 0x1FF);
}
/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(Shell *shell)
{
	shellMutex = xSemaphoreCreateMutex();
	shell->write = userShellWrite;
	shell->read = userShellRead;
	//    shell->lock = userShellLock;
	//    shell->unlock = userShellUnlock;
	shellInit(shell, shellBuffer, 512);

	shell_log.write = LogWrite;
	shell_log.active = 1;
	shell_log.level = LOG_DEBUG;
	logRegister(&shell_log, shell);
}

