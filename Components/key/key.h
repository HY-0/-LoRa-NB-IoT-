#ifndef __KEY_H
#define __KEY_H

#include "sys.h"
#include "delay.h"
#include "stm32f10x.h"

/******************************************************************************************/
/* 引脚 定义 */
#define KEY0_GPIO_PORT                  GPIOB
#define KEY0_GPIO_PIN                   GPIO_Pin_13
#define KEY0_GPIO_CLK_ENABLE()          do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)   /* PC口时钟使能 */

#define KEY1_GPIO_PORT                  GPIOB
#define KEY1_GPIO_PIN                   GPIO_Pin_14
#define KEY1_GPIO_CLK_ENABLE()          do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)   /* PA口时钟使能 */

/******************************************************************************************/
#define KEY0        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)     /* 读取KEY0引脚 */
#define KEY1        GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)     /* 读取KEY1引脚 */

#define KEY0_PRES    1              /* KEY0按下 */
#define KEY1_PRES    2              /* KEY1按下 */

/* ── 非阻塞按键状态机 ── */
typedef enum {
    KS_IDLE = 0,
    KS_PRESS_DEBOUNCE,
    KS_PRESSED,
    KS_RELEASE_DEBOUNCE
} key_state_t;

void key_init(void);                /* 按键初始化函数 */
uint8_t key_scan(uint8_t mode);     /* 按键扫描函数 */
uint8_t key_scan_noblock(uint32_t now_ms);

#endif


