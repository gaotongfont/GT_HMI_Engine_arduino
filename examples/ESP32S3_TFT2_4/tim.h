/**
 ****************************************************************************************************
 * @file        tim.h
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

#ifndef __TIM_H
#define __TIM_H

#include "Arduino.h"

/* 定时器中断定义 */
#define TIMx_INT        0
#define TIMx_ISR        tim0_ISR

/* 函数声明 */
void timx_int_init(uint16_t arr, uint16_t psc);     /* 定时器中断初始化函数 */
void TIMx_ISR(void);                                /* 定时器中断回调函数 */

#endif
