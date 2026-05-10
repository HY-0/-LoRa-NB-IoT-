#ifndef __OLED_H
#define __OLED_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "stm32f10x.h"
#include "oled_font.h"
#include "delay.h"
#include "si2c.h"

#define OLED_SLAVE_ADDR 0x78

#define OLED_SCREEN_COLS 128                     /* 屏幕列数（宽度） */
#define OLED_SCREEN_ROWS  64                     /* 屏幕行数（高度） */
#define OLED_SCREEN_PAGES 8                      /* 屏幕页数 */

#define SSD1306_CTRL_COMMAND           0x80      /* 单命令控制字节（Continuation bit=1, D/C=0） */
#define SSD1306_CTRL_COMMAND_STREAM    0x00      /* 命令流控制字节（Continuation bit=0, D/C=0） */
#define SSD1306_CTRL_DATA              0xc0      /* 单数据控制字节（Continuation bit=1, D/C=1） */
#define SSD1306_CTRL_DATA_STREAM       0x40      /* 数据流控制字节（Continuation bit=0, D/C=1） */

#define OLED_COLOR_TRANSPARENT 0x00              /* 透明颜色 */
#define OLED_COLOR_WHITE       0x01              /* 白色 */
#define OLED_COLOR_BLACK       0x02              /* 黑色 */

#define PEN_COLOR_TRANSPARENT OLED_COLOR_TRANSPARENT /* 透明画笔 */
#define PEN_COLOR_WHITE       OLED_COLOR_WHITE       /* 白色画笔 */
#define PEN_COLOR_BLACK       OLED_COLOR_BLACK       /* 黑色画笔 */

#define BRUSH_TRANSPARENT OLED_COLOR_TRANSPARENT     /* 透明画刷 */
#define BRUSH_WHITE       OLED_COLOR_WHITE           /* 白色画刷 */
#define BRUSH_BLACK       OLED_COLOR_BLACK           /* 黑色画刷 */

extern const Font_TypeDef default_font;         /* 默认字体 */
extern OLED_TypeDef oled;                       /* OLED设备实例 */

void oled_init(void);                           /* OLED显示器初始化函数 */
void oled_display_off(void);
void oled_display_on(void);
void oled_show(OLED_TypeDef *oled, int16_t x, int16_t y, uint32_t xms, const char *format, ...); /* 在指定位置显示格式化字符串 */

int oled_hw_init(OLED_TypeDef *oled, OLED_InitTypeDef *oled_init_struct);   /* OLED硬件初始化 */
void oled_clear(OLED_TypeDef *oled);                                        /* 清空屏幕 */
uint16_t oled_get_screen_width(OLED_TypeDef *oled);                         /* 获取屏幕宽度 */
uint16_t oled_get_screen_height(OLED_TypeDef *oled);                        /* 获取屏幕高度 */
int oled_send_buffer(OLED_TypeDef *oled);                                   /* 发送缓冲区的内容到屏幕 */
int oled_start_send_buffer(OLED_TypeDef *oled);                             /* 开始发送缓冲区的内容到屏幕（分段发送） */
int oled_end_send_buffer(OLED_TypeDef *oled, uint8_t *p_more_out);          /* 继续发送缓冲区的内容到屏幕（分段发送） */


void oled_set_cursor(OLED_TypeDef *oled, int16_t x, int16_t y);                 /* 设置光标位置 */
void oled_set_cursor_x(OLED_TypeDef *oled, int16_t x);                          /* 设置光标的X坐标 */
void oled_set_cursor_y(OLED_TypeDef *oled, int16_t y);                          /* 设置光标的Y坐标 */
void oled_move_cursor(OLED_TypeDef *oled, int16_t dx, int16_t dy);              /* 移动光标 */
void oled_move_cursor_x(OLED_TypeDef *oled, int16_t dx);                        /* 沿X轴方向移动光标 */
void oled_move_cursor_y(OLED_TypeDef *oled, int16_t dy);                        /* 沿Y轴方向移动光标 */
void oled_get_cursor(OLED_TypeDef *oled, int16_t *p_x_out, int16_t *p_y_out);   /* 获取光标当前位置 */
int16_t oled_get_cursor_x(OLED_TypeDef *oled);                                  /* 获取光标X坐标 */
int16_t oled_get_cursor_y(OLED_TypeDef *oled);                                  /* 获取光标Y坐标 */


void oled_set_pen(OLED_TypeDef *oled, uint8_t pen_color, uint8_t width);    /* 设置画笔 */
void oled_set_brush(OLED_TypeDef *oled, uint8_t brush_color);               /* 设置画刷 */

void oled_draw_dot(OLED_TypeDef *oled);                                     /* 画点 */
void oled_draw_line(OLED_TypeDef *oled, int16_t x, int16_t y);              /* 画线 */
void oled_line_to(OLED_TypeDef *oled, int16_t x, int16_t y);                /* 画线（终点后光标移动） */
void oled_draw_circle(OLED_TypeDef *oled, uint16_t radius);                 /* 画圆 */
void oled_draw_rect(OLED_TypeDef *oled, uint16_t width, uint16_t height);   /* 画矩形 */
void oled_draw_bitmap(OLED_TypeDef *oled, uint16_t width, uint16_t height, const uint8_t *p_bitmap); /* 画位图 */

void oled_draw_string(OLED_TypeDef *oled, const char *str);              /* 显示字符串 */
void oled_printf(OLED_TypeDef *oled, const char *format, ...);           /* 格式化打印字符串 */
void oled_start_text_region(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height); /* 开启文本区域 */
void oled_stop_text_region(OLED_TypeDef *oled);                          /* 停止文本区域 */
void oled_set_font(OLED_TypeDef *oled, const Font_TypeDef *font);        /* 设置字体 */
uint16_t oled_get_str_width(OLED_TypeDef *oled, const char *str);        /* 获取当前字体下的字符串宽度 */
uint16_t oled_get_font_height(OLED_TypeDef *oled);                       /* 获取字体高度 */

void oled_start_clip_region(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height); /* 设置剪切区域 */
void oled_stop_clip_region(OLED_TypeDef *oled);                          /* 停止剪切区域 */

// 内部静态函数声明（供内部调用，不在头文件导出，但为完整性保留）
static int oled_send_command(OLED_TypeDef *oled, const uint8_t cmd, const uint8_t *arg, uint16_t size);
static int oled_send_data(OLED_TypeDef *oled, uint8_t *p_data, uint16_t size);
static void draw_circle_frame(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t radius);
static void fill_circle(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t radius);
static void draw_rect_frame(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height);
static void fill_rect(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height);
static int16_t unicode_to_glyph_idx(OLED_TypeDef *oled, uint32_t unicode);
static void draw_charator(OLED_TypeDef *oled, uint32_t unicode);
static void brush_dot(OLED_TypeDef *oled, int16_t x, int16_t y);
static void pen_dot(OLED_TypeDef *oled, int16_t x, int16_t y);
static void draw_bitmap_ex(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *p_bitmap);
static uint16_t get_glyph_width(OLED_TypeDef *oled, uint32_t unicode);

#endif
