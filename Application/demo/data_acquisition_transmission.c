/**
 * @file    data_acquisition_transmission.c
 * @brief   数据采集与传输集成模块 - LoRa/NB-IoT农业监测系统
 * @author  Lora项目组
 * @date    2024-01-15
 * @version 1.0.0
 * 
 * @details 本模块集成所有外设功能：
 *          1. BH1750光照传感器数据采集
 *          2. SHT30温湿度传感器数据采集  
 *          3. LoRa/NB-IoT无线数据传输
 *          4. OLED显示数据展示
 *          5. 系统状态指示（LED、蜂鸣器）
 */

#include "data_acquisition_transmission.h"
#include "bh1750.h"
#include "sht30.h"
#include "lora.h"
#include "oled.h"
#include "led.h"
#include "buzzer.h"
#include "key.h"
#include "sys.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

/* 全局变量定义 */
static struct {
    float temperature;     /* 温度 (°C) */
    float humidity;        /* 湿度 (%RH) */
    float light_intensity; /* 光照强度 (lux) */
    uint32_t timestamp;    /* 时间戳 */
    uint8_t data_valid;    /* 数据有效性标志 */
} sensor_data = {0};

static uint8_t transmission_mode = TRANSMISSION_MODE_LORA; /* 默认使用LoRa传输 */

/**
 * @brief       传感器数据采集
 * @param       无
 * @retval      DATA_ACQ_OK    : 数据采集成功
 *              DATA_ACQ_ERROR : 数据采集失败
 */
uint8_t data_acquisition(void)
{
    uint8_t ret;
    
    /* 采集BH1750光照数据 */
    ret = bh1750_read_lux(&sensor_data.light_intensity);
    if (ret != BH1750_EOK) {
        sensor_data.data_valid = 0;
        return DATA_ACQ_ERROR;
    }
    
    /* 采集SHT30温湿度数据 */
    ret = sht30_read_humiture(&sensor_data.temperature, &sensor_data.humidity);
    if (ret != SHT30_EOK) {
        sensor_data.data_valid = 0;
        return DATA_ACQ_ERROR;
    }
    
    /* 更新时间戳 */
    sensor_data.timestamp = get_ms();
    sensor_data.data_valid = 1;
    
    return DATA_ACQ_OK;
}

/**
 * @brief       数据显示到OLED
 * @param       无
 * @retval      无
 */
void data_display(void)
{
    char display_buf[32];
    
    /* 清屏并显示标题 */
    oled_clear(&oled);
    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);
    
    oled_show(&oled, 0, 10, 0, "农业监测系统");
    oled_show(&oled, 0, 25, 0, "=================");
    
    /* 显示温度 */
    sprintf(display_buf, "温度: %.1f C", sensor_data.temperature);
    oled_show(&oled, 0, 40, 0, display_buf);
    
    /* 显示湿度 */
    sprintf(display_buf, "湿度: %.1f %%", sensor_data.humidity);
    oled_show(&oled, 0, 55, 0, display_buf);
    
    /* 显示光照 */
    sprintf(display_buf, "光照: %.0f lux", sensor_data.light_intensity);
    oled_show(&oled, 0, 70, 0, display_buf);
    
    /* 显示传输模式 */
    if (transmission_mode == TRANSMISSION_MODE_LORA) {
        oled_show(&oled, 0, 85, 0, "模式: LoRa");
    } else {
        oled_show(&oled, 0, 85, 0, "模式: NB-IoT");
    }
}

/**
 * @brief       数据打包为JSON格式
 * @param       buf: 输出缓冲区
 * @param       size: 缓冲区大小
 * @retval      打包后的数据长度
 */
uint16_t data_pack_json(char *buf, uint16_t size)
{
    if (sensor_data.data_valid == 0) {
        return 0;
    }
    
    return snprintf(buf, size, 
        "{\"temp\":%.1f,\"humi\":%.1f,\"light\":%.0f,\"time\":%lu}",
        sensor_data.temperature,
        sensor_data.humidity,
        sensor_data.light_intensity,
        sensor_data.timestamp);
}

/**
 * @brief       数据传输
 * @param       无
 * @retval      DATA_TX_OK     : 数据传输成功
 *              DATA_TX_ERROR  : 数据传输失败
 *              DATA_TX_NO_DATA: 无有效数据可传输
 */
uint8_t data_transmission(void)
{
    char tx_buf[128];
    uint16_t data_len;
    
    if (sensor_data.data_valid == 0) {
        return DATA_TX_NO_DATA;
    }
    
    /* 打包数据为JSON格式 */
    data_len = data_pack_json(tx_buf, sizeof(tx_buf));
    if (data_len == 0) {
        return DATA_TX_ERROR;
    }
    
    /* 根据传输模式选择传输方式 */
    if (transmission_mode == TRANSMISSION_MODE_LORA) {
        /* 使用LoRa传输 - 直接通过UART发送数据 */
        lora_uart_printf("%s", tx_buf);
        
        /* 简单延迟等待发送完成 */
        delay_ms(100);
    } else {
        /* 使用NB-IoT传输（待实现） */
        // ret = nbiot_send_data((uint8_t *)tx_buf, data_len);
        // if (ret != NBIOT_EOK) {
        //     return DATA_TX_ERROR;
        // }
        return DATA_TX_ERROR; /* NB-IoT功能暂未实现 */
    }
    
    /* 传输成功提示 */
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 200);
    led0_toggle(); /* LED闪烁指示数据传输 */
    
    return DATA_TX_OK;
}

/**
 * @brief       设置传输模式
 * @param       mode: 传输模式
 *                 TRANSMISSION_MODE_LORA  - LoRa传输
 *                 TRANSMISSION_MODE_NBIOT - NB-IoT传输
 * @retval      DATA_MODE_OK   : 模式设置成功
 *              DATA_MODE_ERROR: 模式设置失败
 */
uint8_t set_transmission_mode(uint8_t mode)
{
    if (mode != TRANSMISSION_MODE_LORA && mode != TRANSMISSION_MODE_NBIOT) {
        return DATA_MODE_ERROR;
    }
    
    transmission_mode = mode;
    return DATA_MODE_OK;
}

/**
 * @brief       获取当前传输模式
 * @param       无
 * @retval      当前传输模式
 */
uint8_t get_transmission_mode(void)
{
    return transmission_mode;
}

/**
 * @brief       获取传感器数据
 * @param       data: 传感器数据结构体指针
 * @retval      DATA_GET_OK    : 获取成功
 *              DATA_GET_ERROR : 无有效数据
 */
uint8_t get_sensor_data(sensor_data_t *data)
{
    if (sensor_data.data_valid == 0) {
        return DATA_GET_ERROR;
    }
    
    memcpy(data, &sensor_data, sizeof(sensor_data_t));
    return DATA_GET_OK;
}

/**
 * @brief       数据采集与传输主任务
 * @param       无
 * @retval      无
 */
void data_acquisition_transmission_task(void)
{
    uint8_t acq_ret, tx_ret;
    
    /* 蜂鸣器提示任务开始 */
    buzzer_beep_scene(BUZZER_SCENE_OK, BUZZER_MODE_INTERMITTENT, BUZZER_RHYTHM_FAST, 300);
    
    while (1) {
        /* 数据采集 */
        acq_ret = data_acquisition();
        if (acq_ret == DATA_ACQ_OK) {
            /* 数据显示 */
            data_display();
            
            /* 数据传输 */
            tx_ret = data_transmission();
            if (tx_ret == DATA_TX_OK) {
                /* 传输成功，LED闪烁 */
                led0_toggle();
            }
        } else {
            /* 采集失败，错误提示 */
            oled_clear(&oled);
            oled_show(&oled, 30, 40, 0, "采集失败!");
            buzzer_beep_scene(BUZZER_SCENE_WARN, BUZZER_MODE_CONTINUOUS, BUZZER_RHYTHM_ALARM, 500);
        }
        
        /* 延时等待下一次采集（例如每5秒采集一次） */
        delay_ms(5000);
        
        /* 检查按键切换传输模式（使用KEY1按键） */
        if (key_scan(1) == 1) {  /* KEY1按下 */
            if (transmission_mode == TRANSMISSION_MODE_LORA) {
                set_transmission_mode(TRANSMISSION_MODE_NBIOT);
            } else {
                set_transmission_mode(TRANSMISSION_MODE_LORA);
            }
            buzzer_beep_scene(BUZZER_SCENE_CLICK, BUZZER_MODE_CONTINUOUS, BUZZER_RHYTHM_FAST, 200);
        }
    }
}
