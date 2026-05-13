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
// #define DEVICE_SENDER      // 发送端启用此行，注释下一行
#define DEVICE_RECEIVER   // 接收端启用此行，注释上一行

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
}

/****************************************************************
 * 发送端 main 函数
 ****************************************************************/
#ifdef DEVICE_SENDER
int main(void)
{
    system_common_init();

    /* ─── 初始化硬件与通信 ─── */
    transmission_lora_init();      /* 发送端仅初始化 LoRa 模块 */
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    acquisition_init();            /* 初始化传感器采集外设 */
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    /* ─── 运行状态变量 ─── */
    sensor_data_t local_data = {0};
    comm_status_t comm_status = {0};
    
    /* 调度器时间锚点 (单位: 毫秒) */
    uint32_t last_collect_ms = 0;
    uint32_t last_send_ms = 0;
    const uint32_t COLLECT_INTERVAL_MS = 1000;  /* 采集周期: 1秒 */
    const uint32_t SEND_INTERVAL_MS = 5000;     /* 发送周期: 5秒 */

    while (1)
    {
        /* 1. 获取系统运行时间 (用于非阻塞定时调度) */
        uint32_t sys_uptime_ms = get_ms();
        uint8_t key_val = key_scan(0); 

        /* 2. 定时采集传感器数据 */
        if (sys_uptime_ms - last_collect_ms >= COLLECT_INTERVAL_MS) 
        {
            last_collect_ms = sys_uptime_ms;
            if (acquisition_poll() == ACQ_OK) 
            {
                acquisition_read(&local_data);
                local_data.data_valid = 1;
                /* * 架构注意：发送端不具备联网能力，无需关心真实时间戳。
                 * 这里的 local_data.timestamp 保持默认的 0 即可。
                 * 真实时间会在数据抵达接收端网关时，由网关向基站索取并统一打标。
                 */
            } 
            else 
            {
                local_data.data_valid = 0;
            }
        }

        /* 3. 定时通过 LoRa 发射数据 */
        if (sys_uptime_ms - last_send_ms >= SEND_INTERVAL_MS) 
        {
            last_send_ms = sys_uptime_ms;
            
            if (local_data.data_valid) 
            {
                uint8_t lora_send_status = transmission_lora_send(&local_data); 
                
                if (lora_send_status == TRANS_OK) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
                } 
                else if (lora_send_status == TRANS_TRUNCATED) 
                {
                    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_ALARM, 200);
                }
            }
        }

        /* 4. 更新 OLED UI 显示 */
        // comm_status = get_comm_status();
        display_sensor_data(&local_data, &comm_status, key_val);

        /* 休眠等待硬件中断唤醒，极大降低传感器节点功耗 */
        __WFI();
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

    /* ─── 初始化通信模块 ─── */
    transmission_lora_init();      /* 恢复：接收端必须初始化 LoRa 才能监听节点数据 */
    transmission_nbiot_init();     /* 初始化 NB-IoT 并建立 MQTT 连接 */
    
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_SLOW, 1000);

    /* ─── 运行状态变量 ─── */
    sensor_data_t display_data = {0};
    comm_status_t comm_status = {0};

    while (1)
    {
        /* 1. 获取系统运行时间 (毫秒)，用于按键消抖等硬件级非阻塞调度 */
        uint32_t sys_uptime_ms = get_ms();
        uint8_t key_val = key_scan_noblock(sys_uptime_ms);

        /* 2. 获取底层通信状态 (内部已做10s非阻塞限流，每轮调用无性能负担) */
        comm_status = get_comm_status();

        /* 3. 轮询接收 LoRa 节点数据 */
        uint8_t lora_recv_status = transmission_receive(&display_data);

        if (lora_recv_status == TRANS_RECV_OK) 
        {
            // 收到新数据，刷新屏幕并鸣响蜂鸣器
            display_sensor_data(&display_data, &comm_status, key_val);
            buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
            
            /* 获取真实世界时间 (Unix时间戳)，打上时标用于云端数据库入库 */
            uint32_t unix_timestamp = 0;
            if (nbiot_get_timestamp(&unix_timestamp) == NBIOT_EOK) {
                display_data.timestamp = unix_timestamp;  
            }

            /* 将带时间戳的完整数据通过 NB-IoT 转发至云端 */
            uint8_t nbiot_send_status = transmission_nbiot_send(&display_data);
            if (nbiot_send_status != TRANS_OK) 
            {
                display_error("NB Send Fail");
            }
        } 
        else if (lora_recv_status == TRANS_RECV_PARSE_ERROR) 
        {
            display_error("Parse Error");
        } 
        else 
        {
            /* 4. 无新数据时，仅刷新 UI 的通信信号状态和按键响应 */
            display_sensor_data(&display_data, &comm_status, key_val);
        }

        /* 休眠等待中断，降低系统整体功耗 */
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
