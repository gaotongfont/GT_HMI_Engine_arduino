/**
 ****************************************************************************************************
 * @file        spilcd.h
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

#ifndef __SPILCD_H
#define __SPILCD_H

#include "Arduino.h"

/* LCD的宽和高定义 */
extern uint16_t spilcd_width;
extern uint16_t spilcd_height;
extern uint8_t  spilcd_dir;  

/* 定义LCD启用的字体 */
#define FONT_12             1
#define FONT_16             1
#define FONT_24             1
#define FONT_32             1

/* 默认启用12号字体 */
#if ((FONT_12 == 0) && (FONT_16 == 0) && (FONT_24 == 0) && (FONT_32 == 0))
#undef FONT_12
#define FONT_12 1
#endif

/* LCD显示字体枚举 */
typedef enum
{
#if (FONT_12 != 0)
  LCD_FONT_12,             /* 12号字体 */
#endif
#if (FONT_16 != 0)
  LCD_FONT_16,             /* 16号字体 */
#endif
#if (FONT_24 != 0)
  LCD_FONT_24,             /* 24号字体 */
#endif
#if (FONT_32 != 0)
  LCD_FONT_32,             /* 32号字体 */
#endif
} lcd_font_t;

/* LCD显示数字模式枚举 */
typedef enum
{
  NUM_SHOW_NOZERO = 0x00,  /* 数字高位0不显示 */
  NUM_SHOW_ZERO,           /* 数字高位0显示 */
} num_mode_t;

/* 引脚定义 */
#define SLCD_CS_PIN       21   
#define SLCD_SDA_PIN      11 
#define SLCD_SCK_PIN      12
#define SLCD_SDI_PIN      -1
#define SLCD_WR_PIN       40

/* 以下两个SPI_LCD需要用到的IO在xl9555.h中已经有定义
 * #define SLCD_PWR_PIN          
 * #define SLCD_RST_PIN  
 */

/* 宏函数 */
#define LCD_PWR(x)        xl9555_pin_set(SLCD_PWR, x ? IO_SET_HIGH : IO_SET_LOW)
#define LCD_RST(x)        xl9555_pin_set(SLCD_RST, x ? IO_SET_HIGH : IO_SET_LOW)
#define LCD_WR(x)         digitalWrite(SLCD_WR_PIN, x)
#define LCD_CS(x)         digitalWrite(SLCD_CS_PIN, x)

/******************************************************************************************/
/* LCD扫描方向和颜色 定义 */

/* 扫描方向定义 */
#define L2R_U2D         0           /* 从左到右,从上到下 */
#define L2R_D2U         1           /* 从左到右,从下到上 */
#define R2L_U2D         2           /* 从右到左,从上到下 */
#define R2L_D2U         3           /* 从右到左,从下到上 */

#define U2D_L2R         4           /* 从上到下,从左到右 */
#define U2D_R2L         5           /* 从上到下,从右到左 */
#define D2U_L2R         6           /* 从下到上,从左到右 */
#define D2U_R2L         7           /* 从下到上,从右到左 */

#define DFT_SCAN_DIR    L2R_U2D     /* 默认的扫描方向 */

/* 画笔颜色 */
#define WHITE               0xFFFF
#define BLACK               0x0000
#define BLUE                0x001F  
#define BRED                0XF81F
#define GRED                0XFFE0
#define GBLUE               0X07FF
#define RED                 0xF800
#define MAGENTA             0xF81F
#define GREEN               0x07E0
#define CYAN                0x7FFF
#define YELLOW              0xFFE0
#define BROWN               0XBC40      /* 棕色 */
#define BRRED               0XFC07      /* 棕红色 */
#define GRAY                0X8430      /* 灰色 */
#define DARKBLUE            0X01CF      /* 深蓝色 */
#define LIGHTBLUE           0X7D7C      /* 浅蓝色 */
#define GRAYBLUE            0X5458      /* 灰蓝色 */
/* 以上三色为PANEL的颜色  */
#define LIGHTGREEN          0X841F      /* 浅绿色 */
#define LGRAY               0XC618      /* 浅灰色(PANNEL),窗体背景色 */

#define LGRAYBLUE           0XA651      /* 浅灰蓝色(中间层颜色) */
#define LBBLUE              0X2B12      /* 浅棕蓝色(选择条目的反色) */

/* 函数声明 */
/* lcd驱动函数 */
void lcd_init(void);                                                                      /* lcd初始化函数 */
void lcd_set_address(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);                 /* 设置lcd行列地址 */
void lcd_clear(uint16_t color);                                                           /* lcd清屏函数 */
void lcd_display_on(void);                                                                /* 开启lcd背光 */
void lcd_display_off(void);                                                               /* 关闭lcd背光 */
void lcd_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);        /* lcd区域填充 */
void lcd_display_dir(uint8_t dir);
void lcd_scan_dir(uint8_t dir);

/* lcd功能函数 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);                                  /* lcd画点 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);       /* lcd画线段 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color);                    /* lcd画水平线 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);  /* lcd画矩形框 */
void lcd_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                     /* lcd画圆形框 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                     /* 填充实心圆 */

void lcd_show_char(uint16_t x, uint16_t y, char ch, lcd_font_t font, uint8_t mode, uint16_t color);                         /* lcd显示1个字符 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, lcd_font_t font, char *str,  uint16_t color); /* lcd显示字符串 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,  lcd_font_t font, num_mode_t mode, uint16_t color);   /* lcd显示数字，可控制显示高位0 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, lcd_font_t font, uint16_t color);                      /* lcd显示数字，不显示高位0 */
void lcd_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *pic);                                   /* lcd图片显示 */

#endif
