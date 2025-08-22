#include "user_cmd.h"
extern unsigned char use_letter_flag;
void log_f(char *str)
{
	logPrintln("input:%s\r\n",str);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), log_f, log_f, "打印输出");

//系统复位
void reboot(void)
{
	NVIC_SystemReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), reboot, reboot, "系统复位");


void disable_letter_shell(void)
{
	use_letter_flag=0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), disable_letter_shell, disable_letter_shell, "禁止letter_shell");
