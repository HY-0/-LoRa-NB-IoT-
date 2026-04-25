/**
 * @file    main.c
 * @brief   主程序入口 - LoRa/NB-IoT农业监测系统
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 */
#include "transmission.h"
#include "acquisition.h"
#include "stm32f10x.h"
#include "display.h"
#include "buzzer.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "sys.h"

/**
 * @brief   主函数 - 系统初始化并启动数据采集与传输系统
 * @param   无
 * @retval  int - 程序退出码（嵌入式系统中通常不返回）
 */
int main(void)
{
	/* 系统初始化 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	/* 中断优先级分组（抢占2位，子优先级2位） */
	sys_stm32_clock_init(9);						/* 初始化系统时钟为72MHz（HSE=8MHz，倍频9倍） */
	systick_init();									/* 初始化延时函数（参数为系统时钟频率，单位MHz） */
	
	/* 外设初始化 */
	oled_init();									/* 对OLED显示器进行初始化 */
	led_init();
	buzzer_init();
	key_init();

    /*  */
    transmission_init();
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000); /* 系统初始化成功提示 */

    /* 接收端 */
    while (1)
    {
        transmission_receive();
    }

    /*  */
	acquisition_init();
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000); /* 系统初始化成功提示 */
	
    /* 发送端 */
	while(1)
	{
		sensor_data_t tx_data;        // 用于存放最新采集数据
        display_mode_t disp_mode;     // 显示模式
		uint8_t key = key_scan(0);
        
        // 1. 执行一次采集
        if (acquisition_poll() == ACQ_OK)
        {
            // 2. 读取采集结果
            acquisition_read(&tx_data);
            
            // 3. 获取当前传输模式，映射为显示模式
            // if (transport_get_type() == TRANSPORT_LORA)
            //     disp_mode = DISPLAY_MODE_LORA;
            // else
            //     disp_mode = DISPLAY_MODE_NBIOT;
            
            // 4. 显示数据
            display_sensor_data(&tx_data, disp_mode);
            
            // 5. 发送数据
            transmission_send(&tx_data);
        }
        else
        {
            // 采集失败，显示错误信息
            display_error("get err");
        }
    
        
        /* 接收端不灵敏的原因：将接收和发送隔离 */
        delay_ms(5000);  // 5秒采集间隔
	}
}
