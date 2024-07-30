/**
 ****************************************************************************************************
 * @file        xl9555.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       xl9555 驱动代码
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

#ifndef __XL9555_H
#define __XL9555_H

#include "Arduino.h"

/* 引脚定义 */
#define IIC_SCL       42
#define IIC_SDA       41
#define IIC_INT_PIN   0     /* 需要用跳线帽进行连接 */

#define EXIO_ADDR     0x20  /* 7位器件地址 */

#define IIC_INT       digitalRead(IIC_INT_PIN)

/* IO扩展芯片XL9555的各个IO功能 */
#define KEY0                          XL_PIN_P17  /* 按键0引脚 P17 */
#define KEY1                          XL_PIN_P16  /* 按键1引脚 P16 */
#define KEY2                          XL_PIN_P15  /* 按键2引脚 P15 */
#define KEY3                          XL_PIN_P14  /* 按键3引脚 P14 */
#define SLCD_PWR                      XL_PIN_P13  /* SPI_LCD控制背光引脚 P13 */
#define SLCD_RST                      XL_PIN_P12  /* SPI_LCD复位引脚 P12 */
#define CT_RST                        XL_PIN_P11  /* 触摸屏中断引脚 P11 */
#define LCD_BL                        XL_PIN_P10  /* RGB屏背光控制引脚 P10 */
#define GBC_KEY                       XL_PIN_P07  /* ATK_MODULE接口KEY引脚 P07 */
#define GBC_LED                       XL_PIN_P06  /* ATK_MODULE接口LED引脚 P06 */
#define OV_RESET                      XL_PIN_P05  /* 摄像头复位引脚 P05 */
#define OV_PWDN                       XL_PIN_P04  /* 摄像头待机引脚 P04 */
#define BEEP                          XL_PIN_P03  /* 蜂鸣器控制引脚 P03 */
#define SPK_EN                        XL_PIN_P02  /* 功放使能引脚 P02 */
#define QMA_INT                       XL_PIN_P01  /* QMA6100P中断引脚 P01 */
#define AP_INT                        XL_PIN_P00  /* AP3216C中断引脚 P00 */

/* 器件寄存器 */
#define XL9555_INPUT_PORT0_REG        0           /* 输入寄存器：用于读取P0端口的输入值 */
#define XL9555_INPUT_PORT1_REG        1           /* 输入寄存器：用于读取P1端口的输入值 */
#define XL9555_OUTPUT_PORT0_REG       2           /* 输出寄存器 ：用于设置P0端口的输出值 */
#define XL9555_OUTPUT_PORT1_REG       3           /* 输出寄存器 ：用于设置P1端口的输出值 */
#define XL9555_INVERSION_PORT0_REG    4           /* 极性反转寄存器：用于当P0端口做为输入时，对输入的电平进行反转处理，即管脚为高电平时，设置这个寄存器中相应的位为1时，读取到的输入寄存器0，1的值就是低电平0 */
#define XL9555_INVERSION_PORT1_REG    5           /* 极性反转寄存器：用于当P1端口做为输入时，对输入的电平进行反转处理，即管脚为高电平时，设置这个寄存器中相应的位为1时，读取到的输入寄存器0，1的值就是低电平0 */
#define XL9555_CONFIG_PORT0_REG       6           /* 配置寄存器：用于配置P0端口的做为输入(1)或是输出(0) */
#define XL9555_CONFIG_PORT1_REG       7           /* 配置寄存器：用于配置P1端口的做为输入(1)或是输出(0) */

/* XL9555各个IO的功能 */
#define XL_PIN_P00       				      0x0001
#define XL_PIN_P01       				      0x0002
#define XL_PIN_P02       				      0x0004
#define XL_PIN_P03       				      0x0008
#define XL_PIN_P04       				      0x0010
#define XL_PIN_P05       				      0x0020
#define XL_PIN_P06       				      0x0040
#define XL_PIN_P07       				      0x0080
#define XL_PIN_P10      				      0x0100
#define XL_PIN_P11      				      0x0200
#define XL_PIN_P12      				      0x0400
#define XL_PIN_P13      				      0x0800
#define XL_PIN_P14      				      0x1000
#define XL_PIN_P15      				      0x2000
#define XL_PIN_P16      				      0x4000
#define XL_PIN_P17      				      0x8000

#define XL_PORT0_ALL_PIN              0x00FF
#define XL_PORT1_ALL_PIN              0xFF00

/* IO配置模式 */
typedef enum
{
  IO_SET_OUTPUT = 0x00,
  IO_SET_INPUT,
} io_mode_t;

/* IO配置输出高低电平 */
typedef enum
{
  IO_SET_LOW = 0x00,
  IO_SET_HIGH,
} io_state_t;

#define KEY0_STATE                        xl9555_get_pin(KEY0)        /* 读取KEY0引脚 */
#define KEY1_STATE                        xl9555_get_pin(KEY1)        /* 读取KEY1引脚 */
#define KEY2_STATE                        xl9555_get_pin(KEY2)        /* 读取KEY2引脚 */
#define KEY3_STATE                        xl9555_get_pin(KEY3)        /* 读取KEY3引脚 */

#define KEY0_PRES                   1                               /* KEY0按下 */
#define KEY1_PRES                   2                               /* KEY1按下 */
#define KEY2_PRES                   3                               /* KEY1按下 */
#define KEY3_PRES                   4                               /* KEY1按下 */

/* 函数声明 */
void xl9555_init(void);                                     /* 初始化IO扩展芯片 */
void xl9555_write_reg(uint8_t reg, uint8_t data);           /* 向XL9555相关寄存器写数据 */
uint8_t xl9555_read_reg(uint8_t reg);                       /* 向XL9555相关寄存器读取数据 */
void xl9555_write_port(uint8_t portx, uint8_t data);        /* 设置XL9555的P0或P1端口的输出状态 */
uint8_t xl9555_read_port(uint8_t portx);                    /* 读取XL9555的P0或P1端口的状态 */
void xl9555_io_config(uint16_t port_pin, io_mode_t mode);   /* 设置XL9555某个IO的模式(输出或输入) */
void xl9555_pin_set(uint16_t port_pin, io_state_t state);   /* 设置XL9555配置为输出功能的IO的输出状态(高电平或低电平) */
uint8_t xl9555_get_pin(uint16_t port_pin);                  /* 获取XL9555配置为输入功能的IO的状态(高电平或低电平) */
uint8_t xl9555_key_scan(uint8_t mode);                              /* 扫描按键值 */
#endif
