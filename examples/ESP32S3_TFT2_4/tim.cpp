/**
 ****************************************************************************************************
 * @file        tim.cpp
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       TIM 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20231201
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "tim.h"
#include "src/hal/gt_hal_tick.h"

hw_timer_t *timer = NULL;

/**
 * @brief       定时器TIMX定时中断初始化函数
 * @note
 *              定时器的时钟来自APB,而APB为80M
 *              所以定时器时钟 = (80/psc)Mhz, 单位时间为 1 / (80 / psc) = x us
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void timx_int_init(uint16_t arr, uint16_t psc)
{
    timer = timerBegin(TIMx_INT, psc, true);         /* 初始化定时器0 */

    timerAlarmWrite(timer, arr, true);               /* 设置中断时间 */

    timerAttachInterrupt(timer, &TIMx_ISR, true);    /* 配置定时器中断回调函数 */

    timerAlarmEnable(timer);                         /* 使能定时器中断 */
}

/**
 * @brief       定时器TIMX中断回调函数
 * @param       无
 * @retval      无
 */
void TIMx_ISR(void)
{
    gt_tick_inc(1);         /* 心跳函数 */
}

