/**
 * @file    main.c
 * @brief   主程序入口 - LoRa/NB-IoT农业监测系统
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */

#include "bh1750_demo.h"
#include "sht30_demo.h"
#include "lora_demo.h"
#include "stm32f10x.h"
#include "buzzer.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "sys.h"

/**
 * @brief   主函数 - 系统初始化并启动演示程序
 * @param   无
 * @retval  int - 程序退出码（嵌入式系统中通常不返回）
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	/* 中断优先级分组（抢占2位，子优先级2位） */
	sys_stm32_clock_init(9);						/* 初始化系统时钟为72MHz（HSE=8MHz，倍频9倍） */
	systick_init();									/* 初始化延时函数（参数为系统时钟频率，单位MHz） */
	oled_init();									/* 对OLED显示器进行初始化 */
	led_init();
	buzzer_init();
	key_init();
	buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000); /* 系统初始化成功提示 */
    lora_demo_run();
	// bh1750_demo_run();
	// sht30_demo_run();
}
