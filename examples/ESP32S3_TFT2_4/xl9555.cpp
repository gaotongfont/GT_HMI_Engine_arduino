
/**
 ****************************************************************************************************
 * @file        xl9555.cpp
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

#include "xl9555.h"
#include <Wire.h>

/**
* @brief       初始化IO扩展芯片
* @param       无
* @retval      无
*/
void xl9555_init(void)
{
    pinMode(IIC_INT_PIN, INPUT_PULLUP);     /* 配置中断引脚 */

    Wire.begin(IIC_SDA, IIC_SCL, 400000);   /* 初始化IIC连接 */

    /* 上电先读取一次清除中断标志 */
    xl9555_read_port(0);
    xl9555_read_port(1);
}

/**
 * @brief      向XL9555相关寄存器写数据
 * @param      reg    : 寄存器地址
 * @param      data   : 写入到寄存器的数据
 * @retval     无
 */
void xl9555_write_reg(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(EXIO_ADDR);        /* 发送从机的7位器件地址到发送队列 */
    Wire.write(reg);                          /* 发送要写入从机寄存器的地址到发送队列 */
    Wire.write(data);                         /* 发送要写入从机寄存器的数据到发送队列 */
    Wire.endTransmission();                   /* IIC 发送 发送队列的数据(不带参数,表示发送stop信号,结束传输) */
}

/**
 * @brief       向XL9555相关寄存器读取数据
 * @param       reg    : 寄存器地址
 * @retval      寄存器的值 / 0xFF:未接收到数据
 */
uint8_t xl9555_read_reg(uint8_t reg)
{
    Wire.beginTransmission(EXIO_ADDR);        /* 发送从机的7位器件地址到发送队列 */
    Wire.write(reg);                          /* 发送要读取从机的寄存器地址到发送队列 */
    Wire.endTransmission(0);                  /* IIC 发送 发送队列的数据(传参为0,表示重新发送一个start信号,保持IIC总线有效连接) */

    Wire.requestFrom(EXIO_ADDR, 1);           /* 主机向从机发送数据请求,并获取到数据 */
    if (Wire.available() != 0)                /* 得到已经接收到的数据字节数 */
    {
        return Wire.read();                   /* 到数据缓冲区读取数据 */
    }

    return 0xFF;
}

/**
 * @brief       设置XL9555的P0或P1端口的输出状态
 * @param       portx : P0 / P1
 * @param       data  : IO的状态(对应8个IO)
 * @retval      无
 */
void xl9555_write_port(uint8_t portx, uint8_t data)
{
    xl9555_write_reg(portx ? XL9555_OUTPUT_PORT1_REG : XL9555_OUTPUT_PORT0_REG, data);
}

/**
 * @brief       读取XL9555的P0或P1端口的状态
 * @param       portx : P0 / P1
 * @retval      IO的状态(对应8个IO)
 */
uint8_t xl9555_read_port(uint8_t portx)
{
    return xl9555_read_reg(portx ? XL9555_INPUT_PORT1_REG : XL9555_INPUT_PORT0_REG);
}

/**
 * @brief       设置XL9555某个IO的模式(输出或输入)
 * @param       port_pin  : 要设置的IO编号,P0~7或P1~7
 * @param       mode      : IO_SET_OUTPUT / IO_SET_INPUT
 * @retval      无
 */
void xl9555_io_config(uint16_t port_pin, io_mode_t mode)
{
    uint8_t config_reg = 0;
    uint8_t config_value = 0;

    config_reg  = xl9555_read_reg(port_pin > XL_PORT0_ALL_PIN ? XL9555_CONFIG_PORT1_REG : XL9555_CONFIG_PORT0_REG);   /* 先读取设置Pin所在的寄存器情况 */

    if (mode == IO_SET_OUTPUT)    /* 根据 mode参数 设置输入输出情况，不能影响其他IO */
    {
        config_value = config_reg & (~(port_pin >> (port_pin > XL_PORT0_ALL_PIN ? 8 : 0)));   /* 得到某个IO设置为输出功能后的PORT值但不影响未设置的其他IO的状态 */
    }
    else
    {
        config_value = config_reg | (port_pin >> (port_pin > XL_PORT0_ALL_PIN ? 8 : 0));      /* 得到某个IO设置为输入功能的PORT值但不影响未设置的其他IO的状态 */
    }

    xl9555_write_reg(port_pin > XL_PORT0_ALL_PIN ? XL9555_CONFIG_PORT1_REG : XL9555_CONFIG_PORT0_REG, config_value);    /* 向配置寄存器设置IO输入输出状态 */
}

/**
 * @brief       设置XL9555配置为输出功能的IO的输出状态(高电平或低电平)
 * @param       port_pin  : 已经设置好输出功能的IO编号
 * @param       state      : IO_SET_LOW / IO_SET_HIGH
 * @retval      无
 */
void xl9555_pin_set(uint16_t port_pin, io_state_t state)
{
    uint8_t pin_reg = 0;
    uint8_t pin_value = 0;

    pin_reg = xl9555_read_reg(port_pin > XL_PORT0_ALL_PIN ? XL9555_OUTPUT_PORT1_REG : XL9555_OUTPUT_PORT0_REG);     /* 先读取设置Pin所在的寄存器情况 */

    if (state == IO_SET_HIGH)    /* 根据 state参数 设置IO的高低电平 */
    {
        pin_value = pin_reg | (port_pin >> (port_pin > XL_PORT0_ALL_PIN ? 8 : 0));          /* 得到某个IO设置为高电平后的PORT值但不影响未设置的其他IO的状态 */
    }
    else
    {
        pin_value = pin_reg & (~(port_pin >> (port_pin > XL_PORT0_ALL_PIN ? 8 : 0)));       /* 得到某个IO设置为低电平后的PORT值但不影响未设置的其他IO的状态 */
    }

    xl9555_write_reg(port_pin > XL_PORT0_ALL_PIN ? XL9555_OUTPUT_PORT1_REG : XL9555_OUTPUT_PORT0_REG, pin_value);   /* 向输出寄存器设置IO高低电平状态 */
}

/**
 * @brief       获取XL9555配置为输入功能的IO的状态(高电平或低电平)
 * @param       port_pin  : 已经设置好输入功能的IO编号
 * @retval      0低电平 / 1高电平
 */
uint8_t xl9555_get_pin(uint16_t port_pin)
{
    uint8_t pin_state = 0;
    uint8_t port_value = 0;

    port_value = xl9555_read_reg(port_pin > XL_PORT0_ALL_PIN ? XL9555_INPUT_PORT1_REG : XL9555_INPUT_PORT0_REG);  /* 读取pin所在port的状态：1没有按下，0按下 */
    pin_state = port_pin >> (port_pin > XL_PORT0_ALL_PIN ? 8 : 0);    /* 假如是PORT1的PIN需要先右移8位 */
    pin_state = pin_state & port_value;                               /* 得到需要查询位的状态 */

    return pin_state ? 1 : 0;
}

/**
 * @brief       按键扫描函数
 * @param       mode:0->不连续;1->连续
 * @retval      键值, 定义如下:
 *              KEY0_PRES, 1, KEY0按下
 *              KEY1_PRES, 2, KEY1按下
 *              KEY2_PRES, 3, KEY2按下
 *              KEY3_PRES, 4, KEY3按下
 */
uint8_t xl9555_key_scan(uint8_t mode)
{
    uint8_t keyval = 0;
    static uint8_t key_up = 1;                                          /* 按键按松开标志 */

    if (mode)
    {
        key_up = 1;                                                     /* 支持连按 */
    }

    if (key_up && (KEY0_STATE == 0 || KEY1_STATE == 0 || KEY2_STATE == 0  || KEY3_STATE == 0 )) /* 按键松开标志为1, 且有任意一个按键按下了 */
    {
        delay(10);                                                 /* 去抖动 */
        key_up = 0;

        if (KEY0_STATE == 0)
        {
            keyval = KEY0_PRES;
        }

        if (KEY1_STATE == 0)
        {
            keyval = KEY1_PRES;
        }

        if (KEY2_STATE == 0)
        {
            keyval = KEY2_PRES;
        }

        if (KEY3_STATE == 0)
        {
            keyval = KEY3_PRES;
        }
    }
    else if (KEY0_STATE == 1 && KEY1_STATE == 1 && KEY2_STATE == 1 && KEY3_STATE == 1)          /* 没有任何按键按下, 标记按键松开 */
    {
        key_up = 1;
    }

    return keyval;                                                      /* 返回键值 */
}