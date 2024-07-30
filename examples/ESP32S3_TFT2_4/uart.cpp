/**
 ****************************************************************************************************
 * @file        uart.cpp
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

#include "uart.h"

/**
* @brief       初始化UART
* @param       uartx：串口x
* @param       baud：波特率
* @retval      无
*/
void uart_init(uint8_t uartx, uint32_t baud) 
{
    if (uartx == 0)
    {
        Serial.begin(baud);                                   /* 串口0初始化 */
    }
    else if (uartx == 1)
    {
        Serial1.begin(baud, SERIAL_8N1, RXD_PIN, TXD_PIN);    /* 串口1初始化 */
    }
    else if (uartx == 2)
    {
        Serial2.begin(baud, SERIAL_8N1, RXD_PIN, TXD_PIN);    /* 串口2初始化 */
    }
}


