#include "lora_demo.h"

/* LoRa模块配置参数定义 */
#define DEMO_ADDR       2                        /* 设备地址 */
#define DEMO_WLRATE     LORA_WLRATE_19K2         /* 空中速率 */
#define DEMO_CHANNEL    20                       /* 信道 */
#define DEMO_TPOWER     LORA_TPOWER_20DBM        /* 发射功率 */
#define DEMO_WORKMODE   LORA_WORKMODE_NORMAL     /* 工作模式 */
#define DEMO_TMODE      LORA_TMODE_TT            /* 发射模式 */
#define DEMO_WLTIME     LORA_WLTIME_1S           /* 休眠时间 */
#define DEMO_UARTRATE   LORA_UARTRATE_115200BPS  /* UART通讯波特率 */
#define DEMO_UARTPARI   LORA_UARTPARI_NONE       /* UART通讯校验位 */

void HCSR04_Init(void);
float HCSR04_demo(void);

/**
 * @brief       例程演示入口函数
 * @param       无
 * @retval      无
 */
void lora_demo_run(void)
{
    /* HCSR04_Init(); */

    oled_set_pen(&oled, PEN_COLOR_WHITE, 1);         /* 设置白画笔 */
    oled_set_brush(&oled, PEN_COLOR_TRANSPARENT);    /* 透明画刷 */

    oled_show(&oled, 0, 25,   0, "*******************");
    oled_show(&oled, 0, 40,   0, "<<<<STM32-LoRa>>>>");
    oled_show(&oled, 0, 55, 500, "*******************");
      
	uint8_t ret;
    uint8_t key;
    uint8_t *buf;

	/* 初始化ATK-MW1278D模块（波特率115200） */
    ret = lora_init(115200);
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "loRa err!");

        led0_toggle();
    }

    /* 进入配置模式，配置模块参数 */
    lora_enter_config();
    ret  = lora_addr_config(DEMO_ADDR);
    ret += lora_wlrate_channel_config(DEMO_WLRATE, DEMO_CHANNEL);
    ret += lora_tpower_config(DEMO_TPOWER);
    ret += lora_workmode_config(DEMO_WORKMODE);
    ret += lora_tmode_config(DEMO_TMODE);
    ret += lora_wltime_config(DEMO_WLTIME);
    ret += lora_uart_config(DEMO_UARTRATE, DEMO_UARTPARI);
    lora_exit_config();

    /* 检查配置是否全部成功 */
    if (ret != LORA_EOK)
    {
        oled_clear(&oled);
        oled_show(&oled, 30, 25, 500, "loRa err!");

        led0_toggle();
    }

    oled_clear(&oled);
    oled_show(&oled, 30, 40, 500, "loRa ok!");

	while(1)
	{
        oled_clear(&oled);
        oled_show(&oled, 25, 50, 0, "KEY0: Send");

        key = key_scan(0);                          /* 扫描按键（0：不连续扫描） */
        /* float distance = HCSR04_demo(); */          /* 开始测距 */

        if (key == KEY0_PRES)                       /* 如果KEY0按下 */
        {
            /* 检查模块是否空闲（AUX引脚为低电平表示空闲） */
            if (lora_free() != LORA_EBUSY)
            {
                /* 通过模块的UART发送数据 */
                lora_uart_printf("Hello LoRa!\r\n");

                oled_clear(&oled);
                oled_show(&oled, 25, 50, 50, "Send done!");
  
                delay_ms(100);
            }
        }

        /* 检查是否接收到一帧数据 */
        buf = lora_uart_rx_get_frame();
        if (buf != NULL)
        {
            /* 通过调试串口打印接收到的数据 */ 
            oled_clear(&oled);
            oled_show(&oled, 0, 25, 500, "%s", (char*)buf);
           
            /* 重新开始接收下一帧 */
            lora_uart_rx_restart();

            delay_ms(100);
        }

        delay_ms(10);   /* 简单防抖 */
	}
}

void HCSR04_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitConfig = {0};
    TIM_TimeBaseInitConfig.TIM_Prescaler = 71;
    TIM_TimeBaseInitConfig.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitConfig.TIM_Period = 50000;
    TIM_TimeBaseInitConfig.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitConfig);

    /* #2. 初始化输入捕获 */
    /* #2.1 初始化IO引脚 */
    /* 开启GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 配置IO引脚参数 */
    GPIO_InitTypeDef GPIO_InitConfig = {0};
    /* 初始化Trig引脚 PA0 */
    GPIO_InitConfig.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitConfig.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitConfig);
    /* 初始化Echo引脚 PA8 */
    GPIO_InitConfig.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;/* 芯片手册建议输入浮空，但我们发现 ECHO引脚空闲时为低电压，所以设置为IPD模式更哈，这样 ECHO引脚意外断开时就可以默认输出低电压 */
    GPIO_Init(GPIOA, &GPIO_InitConfig);

    TIM_ICInitTypeDef TIM_ICInitConfig = {0};
    TIM_ICInitConfig.TIM_Channel = TIM_Channel_1;
    TIM_ICInitConfig.TIM_ICFilter = 0;
    TIM_ICInitConfig.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitConfig.TIM_ICPrescaler = 1;
    TIM_ICInitConfig.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInit(TIM1, &TIM_ICInitConfig);

    TIM_ICInitConfig.TIM_Channel = TIM_Channel_2;
    TIM_ICInitConfig.TIM_ICFilter = 0;
    TIM_ICInitConfig.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitConfig.TIM_ICPrescaler = 1;
    TIM_ICInitConfig.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    TIM_ICInit(TIM1, &TIM_ICInitConfig);
}

float HCSR04_demo(void)
{
    /* #1. 对CNT清零 */
    TIM_SetCounter(TIM1, 0);

    /* #2. 清除CC1、CC2标志位 */
    TIM_ClearFlag(TIM1, TIM_FLAG_CC1);
    TIM_ClearFlag(TIM1, TIM_FLAG_CC2);

    /* #3. 开启定时器 */
    TIM_Cmd(TIM1, ENABLE);

    /* #4. 向 Trig引脚发送10us的脉冲 */
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
    delay_us(10);
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
    
    /* #5. 等待测量完成 */
    while(TIM_GetFlagStatus(TIM1, TIM_FLAG_CC1) == RESET);
    while(TIM_GetFlagStatus(TIM1, TIM_FLAG_CC2) == RESET);
    
    /* #6. 关闭定时器 */
    TIM_Cmd(TIM1, DISABLE);

    /* #7. 处理数据 */
    uint16_t ccr1 = TIM_GetCapture1(TIM1);
    uint16_t ccr2 = TIM_GetCapture2(TIM1);
    float distance = (ccr2 - ccr1) * 1.0e-6f * 3.4e4f / 2;

    return distance;
}
