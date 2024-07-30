/**
 ****************************************************************************************************
 * @file        uart.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       UART 驱动代码
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

#ifndef __UART_H
#define __UART_H

#include "Arduino.h"

/* 引脚定义 */
/* 串口0默认已经使用了固定IO(GPIO43为U0TXD,GPIO44为U0RXD)  
 * 以下两个宏为串口1或串口2使用到的IO口(例程未使用)
 */
#define TXD_PIN      19
#define RXD_PIN      20

/* 函数声明 */
void uart_init(uint8_t uartx, uint32_t baud);   /* uart初始化函数 */

#endif
