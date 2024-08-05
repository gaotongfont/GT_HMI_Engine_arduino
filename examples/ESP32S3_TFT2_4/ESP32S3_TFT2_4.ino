#include <Arduino.h>
#include "uart.h"
#include "xl9555.h"
#include "spilcd.h"
#include "tim.h"
#include "gt.h"

/**
 * @brief 刷屏函数
 *
 * @param drv   不需要配置
 * @param area  area->x绘制区域的起始x坐标
 *              area->y绘制区域的起始y坐标
 *              area->w绘制区域的宽度
 *              area->h绘制区域的高度
 * @param color 颜色数组
 */
void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color)
{
    uint32_t start_x = area->x, start_y = area->y;
    uint32_t end_x = start_x + area->w, end_y = start_y + area->h;
    //遍历绘制区域内的每个点
    for (uint32_t y = start_y; y < end_y; y++) {
        for (uint32_t x = start_x; x < end_x; x++) {
            //画点
            lcd_draw_point(x, y, color->full);
            color++;
        }
    }

}


/**
 * @brief 触摸函数
 *
 * @param indev_drv 不需要配置
 * @param data      data->state 触摸状态
 *                  data->point.x 触摸点x坐标
 *                  data->point.y 触摸点y坐标
 */
void read_cb(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data)
{
    // if (!tp_dev.scan(1)) {
    //     data->state = GT_INDEV_STATE_RELEASED;
    //     return;
    // }
    // data->point.x = tp_dev.x[0];
    // data->point.y = tp_dev.y[0];
    // data->state = GT_INDEV_STATE_PRESSED;
}

/**
 * @brief 按键函数
 *
 * @param indev_drv 不需要配置
 * @param data      data->state 按键状态
 *                  data->key   按键功能
 */
void read_cb_btn(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data)
{
    uint8_t status = 0;
    status = xl9555_key_scan(1);                    //获取按键扫描函数返回的值
    static int prev_state = 0;                      //用来记录按键上一次的状态
    switch (status) {
    case KEY0_PRES:                                 //按下KEY0
        data->state = GT_INDEV_STATE_PRESSED;       //state设置成按下状态
        data->key = GT_KEY_ENTER;                   //按键功能设置成ENTER，用来确定控件焦点，相当于触摸的点击作用
        break;
    case KEY1_PRES:
        data->state = GT_INDEV_STATE_PRESSED;
        data->key = GT_KEY_NEXT;                    //按键功能设置成NEXT，用来切换下一个控件
        break;
    case KEY2_PRES:
        data->state = GT_INDEV_STATE_PRESSED;
        data->key = GT_KEY_PREV;                    //按键功能设置成PREV，用来切换上一个控件
        break;
    default:                                        //没有按键按下的时候会进入default
        if (prev_state) {                           //如果按键上一个状态存在
            data->state = GT_INDEV_STATE_RELEASED;  //state设置成抬起状态
        }
        else {
            data->state = GT_INDEV_STATE_INVALID;   //state设置成无效状态
        }
        data->key = GT_KEY_NONE;
        prev_state = 0;
        break;
    }
    prev_state = status;
}

/**
 * @brief spi读写
 *
 * @param data_write    写入数据
 * @param len_write     写入数据长度
 * @param data_read     读取数据
 * @param len_read      读取数据长度
 * @return uint32_t
 */
uint32_t spi_wr(uint8_t * data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read)
{
    // uint32_t i;
    // digitalWrite(FLASH_CS, LOW);
    // for (i = 0; i < len_write; i++) {
    //     SPI.transfer(data_write[i]);
    // }
    // for (i = 0; i < len_read; i++) {
    //     data_read[i] = SPI.transfer(0xFF);
    // }
    // digitalWrite(FLASH_CS, HIGH);
    return 0;
}

static void btn_click_cb(gt_event_st * e)
{
    static int cnt_idx = 0;
    char buffer[100];

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "btn %d", ++cnt_idx);
    printf("%s\n", buffer);
    gt_btn_set_text(e->target, buffer);
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a button, and click it will printf touch count.
 */
void gt_examples_button(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * btn = gt_btn_create(screen);
    gt_obj_set_pos(btn, 0, 30);
    gt_obj_set_size(btn, 80, 40);
    gt_obj_add_event_cb(btn, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn1 = gt_btn_create(screen);
    gt_obj_set_pos(btn1, 100, 30);
    gt_obj_set_size(btn1, 80, 40);
    gt_obj_add_event_cb(btn1, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn2 = gt_btn_create(screen);
    gt_obj_set_pos(btn2, 200, 30);
    gt_obj_set_size(btn2, 80, 40);
    gt_obj_add_event_cb(btn2, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}

void setup() {
    uart_init(0, 115200);       /* 串口0初始化 */
    xl9555_init();              /* IO扩展芯片初始化 */
    lcd_init();                 /* LCD初始化 */
    timx_int_init(10, 8000);    /* 定时器初始化,定时时间为1ms */
    lcd_clear(WHITE);
    gt_init();                  /* gui初始化 */
    gt_examples_button();       /* 按键示例函数 */
}

void loop() {
    gt_task_handler();          /* gui的事务处理函数 */
    delay(1);
}
