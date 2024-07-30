/**
 ****************************************************************************************************
 * @file        spilcd.cpp
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       SPILCD 驱动代码
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

#include "spilcd.h"
#include "font.h"
#include <SPI.h>
#include "xl9555.h"

#define SPI_LCD_TYPE    1           /* SPI接口屏幕类型（1：2.4寸SPILCD  0：1.3寸SPILCD） */

/* LCD的宽和高定义 */
#if SPI_LCD_TYPE                    /* 2.4寸SPI_LCD屏幕 */
uint16_t spilcd_width  = 240;       /* 屏幕的宽度 240(竖屏) */
uint16_t spilcd_height = 320;       /* 屏幕的宽度 320(竖屏) */
#else
uint16_t spilcd_width  = 240;       /* 屏幕的宽度 240(竖屏) */
uint16_t spilcd_height = 240;       /* 屏幕的宽度 240(竖屏) */
#endif                              /* 1.3寸SPI_LCD屏幕 */

uint8_t spilcd_dir = 1;             /* 默认横屏(1)、竖屏(0) */

#define USE_LCD_BUF    1            /* 默认使用lcd_buf,使用lcd_buf刷屏速度会大幅度提升,牺牲空间提高速度,内存不够时,置0即可 */
#if USE_LCD_BUF
    #if SPI_LCD_TYPE
    uint16_t lcd_buf[320 * 240];    /* 存放一帧图像数据 */
    #else
    uint16_t lcd_buf[240 * 240];    /* 存放一帧图像数据 */
    #endif
#endif

/* LCD的画笔颜色和背景色 */
uint32_t g_point_color = 0XF800;    /* 画笔颜色 */
uint32_t g_back_color  = 0XFFFF;    /* 背景色 */

static const int SPICLK = 80000000; /* SPI通信速率(屏幕显示异常,调低SPICLK) */

SPIClass* spi_lcd = NULL;           /* 定义一个未初始化指向SPI对象的指针 */

/**
 * @brief       往LCD写命令
 * @param       cmd:命令
 * @retval      无
 */
static void lcd_write_cmd(uint8_t cmd)
{
    LCD_WR(0);
    spi_lcd->transfer(cmd);
}

/**
 * @brief       往LCD写数据
 * @param       data:数据
 * @retval      无
 */
static void lcd_write_data(uint8_t data)
{
    LCD_WR(1);
    spi_lcd->transfer(data);
}

/**
 * @brief	      往LCD写指定数量的数据
 * @param       data:数据的起始地址
 * @param       size:发送数据大小
 * @return      无
 */
static void lcd_write_bytes(uint8_t *data, uint32_t size)
{
    LCD_WR(1);
    spi_lcd->transfer(data, size);
}

/**
 * @brief       往LCD写像素数据
 * @param       data:像素数据
 * @retval      无
 */
static void lcd_write_pixeldata(uint16_t data)
{
    LCD_WR(1);
    spi_lcd->transfer16(data);
}

/**
 * @brief       设置LCD行列地址
 * @param       xs: 列起始地址
 *              ys: 行起始地址
 *              xe: 列结束地址
 *              ye: 行结束地址
 * @retval      无
 */
void lcd_set_address(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    lcd_write_cmd(0x2A);
    lcd_write_data((uint8_t)(xs >> 8) & 0xFF);
    lcd_write_data((uint8_t)xs & 0xFF);
    lcd_write_data((uint8_t)(xe >> 8) & 0xFF);
    lcd_write_data((uint8_t)xe & 0xFF);
    lcd_write_cmd(0x2B);
    lcd_write_data((uint8_t)(ys >> 8) & 0xFF);
    lcd_write_data((uint8_t)ys & 0xFF);
    lcd_write_data((uint8_t)(ye >> 8) & 0xFF);
    lcd_write_data((uint8_t)ye & 0xFF);
    lcd_write_cmd(0x2C);
}

/**
 * @brief       设置LCD的自动扫描方向
 * @note        一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
 * @param       dir:0~7,代表8个方向(具体定义见lcd.h)
 * @retval      无
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    if (spilcd_dir == 1)  /* 横屏时，需改变扫描方向！ */
    {
        switch (dir)      /* 方向转换 */
        {
            case 0:
                dir = 5;
                break;

            case 1:
                dir = 7;
                break;

            case 2:
                dir = 4;
                break;

            case 3:
                dir = 6;
                break;

            case 4:
                dir = 1;
                break;

            case 5:
                dir = 0;
                break;

            case 6:
                dir = 3;
                break;

            case 7:
                dir = 2;
                break;
        }
    }

    /* 根据扫描方式 设置 0x36 寄存器 bit 5,6,7 位的值 */
    switch (dir)
    {
        case L2R_U2D:/* 从左到右,从上到下 */
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U:/* 从左到右,从下到上 */
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D:/* 从右到左,从上到下 */
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U:/* 从右到左,从下到上 */
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R:/* 从上到下,从左到右 */
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L:/* 从上到下,从右到左 */
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R:/* 从下到上,从左到右 */
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L:/* 从下到上,从右到左 */
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    dirreg = 0x36;

    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));

    LCD_CS(0);
    lcd_write_cmd(dirreg);
    lcd_write_data(regval);

    if (regval & 0x20)
    {
        if (spilcd_width < spilcd_height)   /* 交换X,Y */
        {
            temp = spilcd_width;
            spilcd_width = spilcd_height;
            spilcd_height = temp;
        }
    }
    else
    {
        if (spilcd_width > spilcd_height)   /* 交换X,Y */
        {
            temp = spilcd_width;
            spilcd_width = spilcd_height;
            spilcd_height = temp;
        }
    }

    lcd_set_address(0, 0, spilcd_width - 1, spilcd_height - 1);

    LCD_CS(1);
    spi_lcd->endTransaction();
}

/**
 * @brief       设置LCD显示方向
 * @param       dir:0,竖屏; 1,横屏
 * @retval      无
 */
void lcd_display_dir(uint8_t dir)
{
    spilcd_dir = dir;

    if (SPI_LCD_TYPE)   /* 2.4寸屏需要做处理，1.3寸屏不需要处理 */
    {
        if (dir == 0)   /* 竖屏 */
        {
            spilcd_width = 240;
            spilcd_height = 320;
        }
        else            /* 横屏 */
        {
            spilcd_width = 320;
            spilcd_height = 240;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR);
}

/**
 * @brief       清屏函数
 * @param       color: 要清屏的颜色
 * @retval      无
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = spilcd_width * spilcd_height;

    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));
    LCD_CS(0);

    lcd_set_address(0, 0, spilcd_width - 1 ,spilcd_height - 1);

    LCD_WR(1);

#if USE_LCD_BUF
    uint16_t color_tmp = ((color & 0x00FF) << 8) | ((color & 0xFF00) >> 8);   /* 需要转换一下颜色值 */
    for (uint32_t i = 0; i < spilcd_width * spilcd_height; i++)               /* 对lcd_buf填满数据 */
    {
      lcd_buf[i] = color_tmp;
    }
    spi_lcd->transfer(lcd_buf, spilcd_width * spilcd_height * 2);             /* 发送一帧图像数据(单位:字节) */
#else
    for (index = 0; index < totalpoint; index++)
    {
        spi_lcd->transfer16(color);
    }
#endif

    LCD_CS(1);
    spi_lcd->endTransaction();
}

/**
* @brief       初始化spilcd
* @param       无
* @retval      无
*/
void lcd_init(void)
{
    /* 初始化LCD屏需要用到的引脚 */
    xl9555_io_config(SLCD_PWR, IO_SET_OUTPUT);
    xl9555_io_config(SLCD_RST, IO_SET_OUTPUT);
    pinMode(SLCD_WR_PIN, OUTPUT);

    /* LCD屏需要用到的引脚默认状态为高电平 */
    xl9555_pin_set(SLCD_PWR, IO_SET_HIGH);
    xl9555_pin_set(SLCD_RST, IO_SET_HIGH);
    digitalWrite(SLCD_WR_PIN, HIGH);

    /* 对SPI进行配置 */
    spi_lcd = new SPIClass(HSPI);   /* 创建SPIClass实例时选择SPI总线(HSPI) */
    spi_lcd->begin(SLCD_SCK_PIN, SLCD_SDI_PIN, SLCD_SDA_PIN, SLCD_CS_PIN);  /* 设置SPI的通信线 */
    pinMode(SLCD_CS_PIN, OUTPUT);   /* 设置CS引脚为输出模式 */

    /* 硬件复位 */
    LCD_RST(1);
    delay(10);
    LCD_RST(0);
    delay(10);
    LCD_RST(1);
    delay(120);

    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));    /* 使用在SPISettings中自定义的配置进行SPI总线初始化 */

    LCD_CS(0);                  /* 拉低片选线,选中设备 */

    /* 对LCD的寄存器进行配置 */
#if SPI_LCD_TYPE                /* 对2.4寸LCD寄存器进行设置 */
    lcd_write_cmd(0x11);        /* Sleep Out */
    delay(120);                 /* wait for power stability */

    lcd_write_cmd(0x3A);        /* 65k mode */
    lcd_write_data(0x05);

    lcd_write_cmd(0xC5);        /* VCOM */
    lcd_write_data(0x1A);

    lcd_write_cmd(0x36);        /* 屏幕显示方向设置 */
    lcd_write_data(0x00);

    /*-------------ST7789V Frame rate setting-----------*/
    lcd_write_cmd(0xB2);        /* Porch Setting */
    lcd_write_data(0x05);
    lcd_write_data(0x05);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);

    lcd_write_cmd(0xB7);        /* Gate Control */
    lcd_write_data(0x05);       /* 12.2v   -10.43v */

    /*--------------ST7789V Power setting---------------*/
    lcd_write_cmd(0xBB);        /* VCOM */
    lcd_write_data(0x3F);

    lcd_write_cmd(0xC0);        /* Power control */
    lcd_write_data(0x2c);

    lcd_write_cmd(0xC2);		    /* VDV and VRH Command Enable */
    lcd_write_data(0x01);

    lcd_write_cmd(0xC3);        /* VRH Set */
    lcd_write_data(0x0F);       /* 4.3+( vcom+vcom offset+vdv) */

    lcd_write_cmd(0xC4);        /* VDV Set */
    lcd_write_data(0x20);       /* 0v */

    lcd_write_cmd(0xC6);        /* Frame Rate Control in Normal Mode */
    lcd_write_data(0X01);       /* 111Hz */

    lcd_write_cmd(0xD0);        /* Power Control 1 */
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);

    lcd_write_cmd(0xE8);        /* Power Control 1 */
    lcd_write_data(0x03);

    lcd_write_cmd(0xE9);        /* Equalize time control */
    lcd_write_data(0x09);
    lcd_write_data(0x09);
    lcd_write_data(0x08);

    /*---------------ST7789V gamma setting-------------*/
    lcd_write_cmd(0xE0);        /* Set Gamma */
    lcd_write_data(0xD0);
    lcd_write_data(0x05);
    lcd_write_data(0x09);
    lcd_write_data(0x09);
    lcd_write_data(0x08);
    lcd_write_data(0x14);
    lcd_write_data(0x28);
    lcd_write_data(0x33);
    lcd_write_data(0x3F);
    lcd_write_data(0x07);
    lcd_write_data(0x13);
    lcd_write_data(0x14);
    lcd_write_data(0x28);
    lcd_write_data(0x30);

    lcd_write_cmd(0XE1);        /* Set Gamma */
    lcd_write_data(0xD0);
    lcd_write_data(0x05);
    lcd_write_data(0x09);
    lcd_write_data(0x09);
    lcd_write_data(0x08);
    lcd_write_data(0x03);
    lcd_write_data(0x24);
    lcd_write_data(0x32);
    lcd_write_data(0x32);
    lcd_write_data(0x3B);
    lcd_write_data(0x14);
    lcd_write_data(0x13);
    lcd_write_data(0x28);
    lcd_write_data(0x2F);

    lcd_write_cmd(0x21);        /* 反显 */
    lcd_write_cmd(0x29);        /* 开启显示 */
#else                           /* 对1.3寸LCD寄存器进行设置 */
    lcd_write_cmd(0x11);        /* Sleep Out */
    delay(120);                 /* wait for power stability */

    lcd_write_cmd(0x36);        /* Memory Data Access Control */
    lcd_write_data(0x00);

    lcd_write_cmd(0x3A);        /* RGB 5-6-5-bit  */
    lcd_write_data(0x65);

    lcd_write_cmd(0xB2);        /* Porch Setting */
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);

    lcd_write_cmd(0xB7);        /*  Gate Control */
    lcd_write_data(0x72);

    lcd_write_cmd(0xBB);        /* VCOM Setting */
    lcd_write_data(0x3D);

    lcd_write_cmd(0xC0);        /* LCM Control */
    lcd_write_data(0x2C);

    lcd_write_cmd(0xC2);        /* VDV and VRH Command Enable */
    lcd_write_data(0x01);

    lcd_write_cmd(0xC3);        /* VRH Set */
    lcd_write_data(0x19);

    lcd_write_cmd(0xC4);        /* VDV Set */
    lcd_write_data(0x20);

    lcd_write_cmd(0xC6);        /* Frame Rate Control in Normal Mode */
    lcd_write_data(0x0F);

    lcd_write_cmd(0xD0);        /* Power Control 1 */
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);

    lcd_write_cmd(0xE0);        /* Positive Voltage Gamma Control */
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2B);
    lcd_write_data(0x3F);
    lcd_write_data(0x54);
    lcd_write_data(0x4C);
    lcd_write_data(0x18);
    lcd_write_data(0x0D);
    lcd_write_data(0x0B);
    lcd_write_data(0x1F);
    lcd_write_data(0x23);

    lcd_write_cmd(0xE1);        /* Negative Voltage Gamma Control */
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0C);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2C);
    lcd_write_data(0x3F);
    lcd_write_data(0x44);
    lcd_write_data(0x51);
    lcd_write_data(0x2F);
    lcd_write_data(0x1F);
    lcd_write_data(0x1F);
    lcd_write_data(0x20);
    lcd_write_data(0x23);

    lcd_write_cmd(0x21);        /* Display Inversion On */
    lcd_write_cmd(0x29);
#endif
    LCD_CS(1);                  /* 拉高片选线,取消选中 */
    spi_lcd->endTransaction();  /* 结束SPI传输 */

    lcd_display_dir(1);         /* 默认为横屏 */
    lcd_display_on();           /* 开启LCD背光 */
    lcd_clear(WHITE);           /* 清屏 */
}

/**
 * @brief       开启LCD背光
 * @param       无
 * @retval      无
 */
void lcd_display_on(void)
{
    LCD_PWR(1);
}

/**
 * @brief       关闭LCD背光
 * @param       无
 * @retval      无
 */
void lcd_display_off(void)
{
    LCD_PWR(0);
}

/**
 * @brief       LCD区域填充
 * @param       xs   : 区域起始X坐标
 *              ys   : 区域起始Y坐标
 *              xe   : 区域终止X坐标
 *              ye   : 区域终止Y坐标
 *              color: 区域填充颜色
 * @retval      无
 */
void lcd_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint32_t area_size;
    uint16_t buf_index;

    area_size = (xe - xs + 1) * (ye - ys + 1) * sizeof(uint16_t);   /* 计算区域的字节数 */

    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));
    LCD_CS(0);

    lcd_set_address(xs, ys, xe, ye);
    LCD_WR(1);

#if USE_LCD_BUF
    uint16_t color_tmp = ((color & 0x00FF) << 8) | ((color & 0xFF00) >> 8);
    for (uint32_t i = 0; i < area_size / sizeof(uint16_t); i++)
    {
        lcd_buf[i] = color_tmp;
    }
    spi_lcd->transfer(lcd_buf, area_size);
#else
    for (buf_index = 0; buf_index < area_size / sizeof(uint16_t); buf_index++)
    {
        spi_lcd->transfer16(color);
    }
#endif

    LCD_CS(1);
    spi_lcd->endTransaction();
}

/**
 * @brief       LCD画点
 * @param       x    : 待画点的X坐标
 *              y    : 待画点的Y坐标
 *              color: 待画点的颜色
 * @retval      无
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));
    LCD_CS(0);

    lcd_set_address(x, y, x, y);

    lcd_write_pixeldata(color);

    LCD_CS(1);
    spi_lcd->endTransaction();
}

/**
 * @brief       LCD画线段
 * @param       x1   : 待画线段端点1的X坐标
 *              y1   : 待画线段端点1的Y坐标
 *              x2   : 待画线段端点2的X坐标
 *              y2   : 待画线段端点2的Y坐标
 *              color: 待画线段的颜色
 * @retval      无
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t x_delta;
    uint16_t y_delta;
    int16_t x_sign;
    int16_t y_sign;
    int16_t error;
    int16_t error2;

    x_delta = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    y_delta = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    x_sign = (x1 < x2) ? 1 : -1;
    y_sign = (y1 < y2) ? 1 : -1;
    error = x_delta - y_delta;

    lcd_draw_point(x2, y2, color);

    while ((x1 != x2) || (y1 != y2))
    {
        lcd_draw_point(x1, y1, color);

        error2 = error << 1;
        if (error2 > -y_delta)
        {
            error -= y_delta;
            x1 += x_sign;
        }

        if (error2 < x_delta)
        {
            error += x_delta;
            y1 += y_sign;
        }
    }
}

/**
 * @brief       画水平线
 * @param       x,y   : 起点坐标
 * @param       len   : 线长度
 * @param       color : 矩形的颜色
 * @retval      无
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > spilcd_width) || (y > spilcd_height))
    {
        return;
    }

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief       LCD画矩形框
 * @param       x1   : 待画矩形框端点1的X坐标
 *              y1   : 待画矩形框端点1的Y坐标
 *              x2   : 待画矩形框端点2的X坐标
 *              y2   : 待画矩形框端点2的Y坐标
 *              color: 待画矩形框的颜色
 * @retval      无
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       LCD画圆形框
 * @param       x    : 待画圆形框原点的X坐标
 *              y    : 待画圆形框原点的Y坐标
 *              r    : 待画圆形框的半径
 *              color: 待画圆形框的颜色
 * @retval      无
 */
void lcd_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int32_t x_t;
    int32_t y_t;
    int32_t error;
    int32_t error2;

    x_t = -r;
    y_t = 0;
    error = 2 - 2 * r;

    do
    {
        lcd_draw_point(x - x_t, y + y_t, color);
        lcd_draw_point(x + x_t, y + y_t, color);
        lcd_draw_point(x + x_t, y - y_t, color);
        lcd_draw_point(x - x_t, y - y_t, color);

        error2 = error;
        if (error2 <= y_t)
        {
            y_t++;
            error = error + (y_t * 2 + 1);
            if ((-x_t == y_t) && (error2 <= x_t))
            {
                error2 = 0;
            }
        }

        if (error2 > x_t)
        {
            x_t++;
            error = error + (x_t * 2 + 1);
        }
    } while (x_t <= 0);
}

/**
 * @brief       填充实心圆
 * @param       x,y  : 圆中心坐标
 * @param       r    : 半径
 * @param       color: 圆的颜色
 * @retval      无
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
      if ((i * i + xr * xr) > sqmax)
      {
          /* draw lines from outside */
          if (xr > imax)
          {
            lcd_draw_hline (x - i + 1, y + xr, 2 * (i - 1), color);
            lcd_draw_hline (x - i + 1, y - xr, 2 * (i - 1), color);
          }

          xr--;
      }

      /* draw lines from inside (center) */
      lcd_draw_hline(x - xr, y + i, 2 * xr, color);
      lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}



/**
 * @brief       LCD显示1个字符
 * @param       x    : 待显示字符的X坐标
 *              y    : 待显示字符的Y坐标
 *              ch   : 待显示字符
 *              font : 待显示字符的字体
 *              mode : 叠加方式(1); 非叠加方式(0)
 *              color: 待显示字符的颜色
 * @retval      无
 */
void lcd_show_char(uint16_t x, uint16_t y, char ch, lcd_font_t font, uint8_t mode, uint16_t color)
{
    const uint8_t *ch_code;
    uint8_t ch_width;
    uint8_t ch_height;
    uint8_t ch_size;
    uint8_t ch_offset;
    uint8_t byte_index;
    uint8_t byte_code;
    uint8_t bit_index;
    uint8_t width_index = 0;
    uint8_t height_index = 0;

    ch_offset = ch - ' ';   /* 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库） */

    switch (font)   /* 获取字体的高度以及宽度 */
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_code = font_1206[ch_offset];
            ch_width = FONT_12_CHAR_WIDTH;
            ch_height = FONT_12_CHAR_HEIGHT;
            ch_size = FONT_12_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_code = font_1608[ch_offset];
            ch_width = FONT_16_CHAR_WIDTH;
            ch_height = FONT_16_CHAR_HEIGHT;
            ch_size = FONT_16_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_code = font_2412[ch_offset];
            ch_width = FONT_24_CHAR_WIDTH;
            ch_height = FONT_24_CHAR_HEIGHT;
            ch_size = FONT_24_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_code = font_3216[ch_offset];
            ch_width = FONT_32_CHAR_WIDTH;
            ch_height = FONT_32_CHAR_HEIGHT;
            ch_size = FONT_32_CHAR_SIZE;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    if ((x + ch_width > spilcd_width) || (y + ch_height > spilcd_height))
    {
        return;
    }

    for (byte_index = 0; byte_index < ch_size; byte_index++)
    {
        byte_code = ch_code[byte_index];                  /* 获取字符的点阵数据 */

        for (bit_index = 0; bit_index < 8; bit_index++)   /* 一个字节8个点 */
        {
            if ((byte_code & 0x80) != 0)                  /* 有效点,需要显示 */
            {
                lcd_draw_point(x + width_index, y + height_index, color);           /* 画点出来,要显示这个点 */
            }
            else if (mode == 0)
            {
                lcd_draw_point(x + width_index, y + height_index, g_back_color);    /* 画背景色,相当于这个点不显示(注意背景色由全局变量控制) */
            }

            height_index++;

            if (height_index == ch_height)    /* 显示完一列了? */
            {
                height_index = 0;             /* y坐标复位 */
                width_index++;                /* x坐标递增 */
                break;
            }

            byte_code <<= 1;                  /* 移位, 以便获取下一个位的状态 */
        }
    }
}

/**
 * @brief       LCD显示字符串
 * @note        会自动换行和换页
 * @param       x    : 待显示字符串的X坐标
 *              y    : 待显示字符串的Y坐标
 *              str  : 待显示字符串
 *              font : 待显示字符串的字体
 *              color: 待显示字符串的颜色
 * @retval      无
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, lcd_font_t font, char *str,  uint16_t color)
{
    uint8_t x0 = x;
    uint8_t ch_width;
    uint8_t ch_height;

    width += x;
    height += y;

    switch (font)   /* 获取字体的高度以及宽度 */
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_width = FONT_12_CHAR_WIDTH;
            ch_height = FONT_12_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_width = FONT_16_CHAR_WIDTH;
            ch_height = FONT_16_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_width = FONT_24_CHAR_WIDTH;
            ch_height = FONT_24_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_width = FONT_32_CHAR_WIDTH;
            ch_height = FONT_32_CHAR_HEIGHT;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    while ((*str >= ' ') && (*str <= '~'))   /* 判断是不是非法字符! */
    {
        if (x >= width)
        {
            x = x0;
            y += ch_height;
        }

        if (y >= height)
        {
            break;
        }

      lcd_show_char(x, y, *str, font, 0, color);

      x += ch_width;
      str++;
    }
}

/**
 * @brief       平方函数，x^y
 * @param       x: 底数
 *              y: 指数
 * @retval      x^y
 */
static uint32_t lcd_pow(uint8_t x, uint8_t y)
{
    uint8_t loop;
    uint32_t res = 1;

    for (loop = 0; loop < y; loop++)
    {
        res *= x;
    }

    return res;
}

/**
 * @brief       LCD显示数字，可控制显示高位0
 * @param       x    : 待显示数字的X坐标
 *              y    : 待显示数字的Y坐标
 *              num  : 待显示数字
 *              len  : 待显示数字的位数
 *              mode : NUM_SHOW_NOZERO: 数字高位0不显示
 *                     NUM_SHOW_ZERO  : 数字高位0显示
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,  lcd_font_t font, num_mode_t mode, uint16_t color)
{
    uint8_t ch_width;
    uint8_t len_index;
    uint8_t num_index;
    uint8_t first_nozero = 0;
    char pad;

    switch (font)
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_width = FONT_12_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_width = FONT_16_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_width = FONT_24_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_width = FONT_32_CHAR_WIDTH;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    switch (mode)
    {
        case NUM_SHOW_NOZERO:
        {
            pad = ' ';
            break;
        }
        case NUM_SHOW_ZERO:
        {
            pad = '0';
            break;
        }
        default:
        {
            return;
        }
    }

    for (len_index = 0; len_index < len; len_index++)                 /* 按总显示位数循环 */
    {
        num_index = (num / lcd_pow(10, len - len_index - 1)) % 10;    /* 获取对应位的数字 */
        if ((first_nozero == 0) && (len_index < (len - 1)))           /* 没有使能显示,且还有位要显示 */
        {
            if (num_index == 0)
            {
                lcd_show_char(x + ch_width * len_index, y, pad, font, mode & 0x01, color);   /* 高位需要填充0 */
                continue;
            }
            else
            {
                first_nozero = 1;   /* 使能显示 */
            }
        }

        lcd_show_char(x + ch_width * len_index, y, num_index + '0', font, mode & 0x01, color);
    }
}

/**
 * @brief       LCD显示数字，不显示高位0
 * @param       x    : 待显示数字的X坐标
 *              y    : 待显示数字的Y坐标
 *              num  : 待显示数字
 *              len  : 待显示数字的位数
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, lcd_font_t font, uint16_t color)
{
    lcd_show_xnum(x, y, num, len, font,  NUM_SHOW_NOZERO, color);
}

/**
 * @brief       LCD图片显示
 * @note        图片取模方式: 水平扫描、RGB565、高位在前
 * @param       x     : 待显示图片的X坐标
 *              y     : 待显示图片的Y坐标
 *              width : 待显示图片的宽度
 *              height: 待显示图片的高度
 *              pic   : 待显示图片数组首地址
 * @retval      无
 */
void lcd_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *pic)
{
    if ((x + width > spilcd_width) || (y + height > spilcd_height))
    {
        return;
    }

    spi_lcd->beginTransaction(SPISettings(SPICLK, MSBFIRST, SPI_MODE3));
    LCD_CS(0);

    lcd_set_address(x, y, x + width - 1, y + height - 1);

    lcd_write_bytes(pic, width * height * sizeof(uint16_t));

    LCD_CS(1);
    spi_lcd->endTransaction();
}

