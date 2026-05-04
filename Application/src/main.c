/**
 * @file    main.c
 * @brief   主程序入口 - LoRa/NB-IoT 农业监测系统
 * @author  Lora项目组
 * @date    2026-05-4
 * @version 5.0.0
 */
#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "buzzer.h"
#include "display.h"
#include "acquisition.h"
#include "transmission.h"

/* ──────── 设备选择（编译前修改此处） ──────── */
#define DEVICE_SENDER      // 发送端启用此行，注释下一行
// #define DEVICE_RECEIVER   // 接收端启用此行，注释上一行

/****************************************************************
 * 共用初始化函数（避免重复代码）
 ****************************************************************/
static void system_common_init(void)
{
    /* 系统初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    sys_stm32_clock_init(9);        /* 72MHz */
    systick_init();
    
    /* 外设初始化 */
    oled_init();
    led_init();
    buzzer_init();
    key_init();

    /* 首次显示，避免黑屏 */
    display_sensor_data(NULL, NULL, 0);
}
/****************************************************************
 * 发送端 main 函数
 ****************************************************************/
#ifdef DEVICE_SENDER
int main(void)
{
    system_common_init();

    /* 仅初始化 LoRa（发送端无 NB） */
    transmission_lora_init();
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT,
                      BUZZER_RHYTHM_SLOW, 1000);

    /* 初始化传感器 */
    acquisition_init();
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT,
                      BUZZER_RHYTHM_SLOW, 1000);

    sensor_data_t local_data = {0};
    comm_status_t comm = {0};
    uint8_t key;
    uint32_t last_collect = 0;
    uint32_t last_send = 0;
    const uint32_t collect_interval = 1000;  /* 1秒采集一次 */
    const uint32_t send_interval = 5000;     /* 5秒发送一次 */

    while (1)
    {
        uint32_t now = get_ms();
        key = key_scan_noblock(now);            /* 非阻塞按键 */

        /* ── 定时采集 ── */
        if (now - last_collect >= collect_interval) 
        {
            last_collect = now;
            if (acquisition_poll() == ACQ_OK) 
            {
                acquisition_read(&local_data);
                local_data.data_valid = 1;
            } 
            else 
            {
                local_data.data_valid = 0;
            }
        }

        /* ── 定时发送 ── */
        if (now - last_send >= send_interval) 
        {
            last_send = now;
            if (local_data.data_valid) 
            {
                uint8_t send_ret = transmission_send(&local_data);
                if (send_ret == TRANS_OK) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
                } 
                else if (send_ret == TRANS_TRUNCATED) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_ALARM, 200);
                }
            }
        }

        /* ── 显示本地数据 ── */
        comm = get_comm_status();
        display_sensor_data(&local_data, &comm, key);

        __WFI();   /* 休眠降功耗 */
    }
}
#endif /* DEVICE_SENDER */


/****************************************************************
 * 接收端 main 函数
 ****************************************************************/
#ifdef DEVICE_RECEIVER
int main(void)
{
    system_common_init();

    /* 仅初始化 通信模块（接收端无传感器） */
    transmission_lora_init();
    transmission_nbiot_init();
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT,
                      BUZZER_RHYTHM_SLOW, 1000);

    sensor_data_t display_data = {0};
    comm_status_t comm = {0};
    uint8_t key;
    uint8_t recv_ret;

    while (1)
    {
        uint32_t now = get_ms();
        key = key_scan_noblock(now);

        recv_ret = transmission_receive(&display_data);
        comm = get_comm_status();

        if (recv_ret == TRANS_RECV_OK) 
        {
            display_sensor_data(&display_data, &comm, key);
            buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT,
                              BUZZER_RHYTHM_FAST, 200);
        } 
        else if (recv_ret == TRANS_RECV_PARSE_ERROR) 
        {
            display_error("Parse Error");
        } 
        else 
        {
            /* 无数据或等待中，保持上次显示内容（display_sensor_data 内部会按需刷新） */
            display_sensor_data(NULL, &comm, key);
        }

        __WFI();
    }
}
#endif /* DEVICE_RECEIVER */



// int main(void)
// {
//     /* ── 系统基础初始化 ── */
//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//     sys_stm32_clock_init(9);        /* 72MHz */
//     systick_init();
    
//     /* ── 外设初始化 ── */
//     oled_init();
//     led_init();
//     buzzer_init();
//     key_init();
    
//     /* ── 通信模块初始化 ── */
//     transmission_init();
//     buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);
    
//     /* ── 传感器初始化 ── */
//     acquisition_init();
//     buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

//     /* ── 主循环变量 ── */
//     sensor_data_t local_data = {0};     /* 本地采集数据 */
//     sensor_data_t display_data = {0};   /* 最终送给显示的数据 */
//     comm_status_t comm = {0};
//     uint8_t key;
//     uint8_t recv_ret;
//     uint32_t last_collect = 0;          /* 上次采集时间戳 */
//     uint32_t last_send = 0;             /* 上次发送时间戳 */
//     const uint32_t collect_interval = 1000;  /* 1秒采集一次 */
//     const uint32_t send_interval = 5000;     /* 5秒发送一次 */

//     while (1)
//     {
//         key = key_scan(0);              /* 获取按键值（非阻塞） */
//         uint32_t now = get_ms();        /* 获取系统运行毫秒数 */

//         /* ── 定时采集本地传感器数据 ── */
//         // if (now - last_collect >= collect_interval) {
//         //     last_collect = now;
//             if (acquisition_poll() == ACQ_OK) {
//                 acquisition_read(&local_data);
//                 local_data.data_valid = 1;
//             } else {
//                 local_data.data_valid = 0;
//             }
//         // }

//         /* ── 定时发送（不阻塞） ── */
//         if (now - last_send >= send_interval) {
//             last_send = now;
//             if (local_data.data_valid) {
//                 transmission_send(&local_data);
//             }
//         }

//         // /* ── 持续接收远端数据（非阻塞） ── */
//         // recv_ret = transmission_receive(&display_data);
//         // if (recv_ret == TRANS_RECV_OK) {
//         //     /* 远端数据成功接收，使用远端数据刷新显示 */
//         //     comm = get_comm_status();
//         //     display_sensor_data(&display_data, &comm, key);
//         //     buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
//         // } else if (recv_ret == TRANS_RECV_PARSE_ERROR) {
//         //     display_error("Parse Error");
//         // }

//         /* ── 若无远端数据，则显示本地采集数据（保证屏幕常新） ── */
//         // if (recv_ret == TRANS_RECV_NO_DATA) {
//             comm = get_comm_status();
//             display_sensor_data(&local_data, &comm, key);
//         // }

//         delay_ms(5);   /* 控制主循环频率约 50Hz */
//     }
// }
