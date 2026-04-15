#include "sys.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_pwr.h"

/**
 * @brief       设置中断向量表偏移地址
 * @param       baseaddr: 基址
 * @param       offset: 偏移量(必须是0, 或者0X100的倍数)
 * @retval      无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       执行: WFI指令(执行完该指令进入低功耗状态, 等待中断唤醒)
 * @param       无
 * @retval      无
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       关闭所有中断(但是不包括fault和NMI中断)
 * @param       无
 * @retval      无
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       开启所有中断
 * @param       无
 * @retval      无
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       设置栈顶地址
 * @note        左侧的红X, 属于MDK误报, 实际是没问题的
 * @param       addr: 栈顶地址
 * @retval      无
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);  /* 设置栈顶地址 */
}

/**
 * @brief       进入待机模式
 * @param       无
 * @retval      无
 */
void sys_standby(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); /* 使能电源时钟 */
    PWR_EnterSTANDBYMode();                              /* 进入待机模式 */
}

/**
 * @brief       系统软复位
 * @param       无
 * @retval      无
 */
void sys_soft_reset(void)
{
    NVIC_SystemReset();
}

/**
 * @brief       系统时钟初始化函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~16
 *                中断向量表位置在启动时已经在SystemInit()中初始化
 * @retval      无
 */
void sys_stm32_clock_init(uint32_t plln)
{
    // ErrorStatus ret = SUCCESS;
    uint32_t PLLMulTable[] = {
        /* 索引0~1不用，从plln=2开始对应 */
        RCC_PLLMul_2,   RCC_PLLMul_3,   RCC_PLLMul_4,   RCC_PLLMul_5,
        RCC_PLLMul_6,   RCC_PLLMul_7,   RCC_PLLMul_8,   RCC_PLLMul_9,
        RCC_PLLMul_10,  RCC_PLLMul_11,  RCC_PLLMul_12,  RCC_PLLMul_13,
        RCC_PLLMul_14,  RCC_PLLMul_15,  RCC_PLLMul_16
    };

    /* 参数有效性检查 */
    if (plln < 2 || plln > 16)
    {
        while (1);  /* 倍频系数错误，死循环 */
    }

    /* 使能HSE */
    RCC_HSEConfig(RCC_HSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);  /* 等待HSE就绪 */

    /* 配置FLASH预取缓冲和等待周期 */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);  /* 72MHz需要2个等待周期 */

    /* 配置AHB、APB1、APB2预分频 */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);      /* HCLK = SYSCLK */
    RCC_PCLK1Config(RCC_HCLK_Div2);       /* PCLK1 = HCLK/2 */
    RCC_PCLK2Config(RCC_HCLK_Div1);       /* PCLK2 = HCLK */

    /* 配置PLL */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, PLLMulTable[plln - 2]); /* HSE不分频作为PLL输入 */
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  /* 等待PLL就绪 */

    /* 选择PLL作为系统时钟源 */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);  /* 0x08表示PLL作为系统时钟 */
}
