#include "oled_default_font.h"
#include "oled.h"

OLED_TypeDef oled;

//
// @简介：OLED初始化
//
// @参数：oled - OLED显示器的句柄
//
// @返回值：0 - 成功，
//         -1 - 数据发送失败
//         -2 - 缓冲区分配失败
//
int oled_hw_init(OLED_TypeDef *oled, OLED_InitTypeDef *oled_init_struct)
{
    oled->i2c_write_cb = oled_init_struct->i2c_write_cb;

    // 给缓冲区分配空间
    oled->pBuffer = (uint8_t *)malloc(OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t) + 1);

    if (oled->pBuffer == 0) {
        return -2;
    }

    oled->pBuffer++;

    // 给缓冲区所有字节赋初值0
    memset(oled->pBuffer, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES * sizeof(uint8_t));

    uint8_t arg;

    if (oled_send_command(oled, 0xae, 0, 0)) return -1; /* display off*/
    arg = 0x80;
    if (oled_send_command(oled, 0xd5, &arg, 1)) return -1; /* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
    arg = 0x3f;
    if (oled_send_command(oled, 0xa8, &arg, 1)) return -1; /* multiplex ratio */
    arg = 0x00;
    if (oled_send_command(oled, 0xd3, &arg, 1)) return -1; /* vertical shift */

    if (oled_send_command(oled, 0x40, 0, 0)) return -1; /* set display start line to 0 */
    arg = 0x14;
    if (oled_send_command(oled, 0x8d, &arg, 1)) return -1; /* [2] charge pump setting (p62): 0x014 enable, 0x010 disable, SSD1306 only, should be removed for SH1106 */
    arg = 0x00;
    if (oled_send_command(oled, 0x20, &arg, 1)) return -1; /* horizontal addressing mode */

    if (oled_send_command(oled, 0xa1, 0, 0)) return -1; /* segment remap a0/a1 */
    if (oled_send_command(oled, 0xc8, 0, 0)) return -1; /* c0: scan dir normal, c8: reverse */

    arg = 0x12;
    if (oled_send_command(oled, 0xda, &arg, 1)) return -1; /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
    arg = 0xcf;
    if (oled_send_command(oled, 0x81, &arg, 1)) return -1; /* [2] set contrast control */
    arg = 0xf1;
    if (oled_send_command(oled, 0xd9, &arg, 1)) return -1; /* [2] pre-charge period 0x022/f1*/
    arg = 0x20;
    if (oled_send_command(oled, 0xdb, &arg, 1)) return -1; /* vcomh deselect level */

    // if vcomh is 0, then this will give the biggest range for contrast control issue #98
    // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs, #116
    if (oled_send_command(oled, 0x2e, 0, 0)) return -1; /* Deactivate scroll */
    if (oled_send_command(oled, 0xa4, 0, 0)) return -1; /* output ram to display */
    if (oled_send_command(oled, 0xa6, 0, 0)) return -1; /* none inverted normal display mode */

    if (oled_send_command(oled, 0xaf, 0, 0)) return -1; /* display on*/

    oled->PenWidth = 1;                  // 默认线宽1
    oled->PenColor = PEN_COLOR_WHITE;    // 默认白色画笔
    oled->Brush = BRUSH_TRANSPARENT;     // 默认白色画刷

    oled->CursorX = 0;
    oled->CursorY = 0;                   // 光标默认在屏幕的左上角

    oled->RefreshProgress = 0;

    oled->TextRegionX = 0;               // 默认关闭文本框功能
    oled->TextRegionY = 0;
    oled->TextRegionWidth = 0;
    oled->TextRegionHeight = 0;

    oled->Font = &default_font;          // 使用默认字体，8*8点阵字体

    return 0;
}

void oled_init(void)
{
    OLED_InitTypeDef oled_struct;
    si2c_init();
    oled_struct.i2c_write_cb = si2c_write_bytes;      // 回调函数
    oled_hw_init(&oled, &oled_struct);
}

void oled_show(OLED_TypeDef *oled, int16_t x, int16_t y, uint32_t xms, const char *format, ...)
{
    oled_set_cursor(oled, x, y);
    char format_buffer[64];
    va_list argptr;
    __va_start(argptr, format);
    vsprintf(format_buffer, format, argptr);
    __va_end(argptr);
    oled_draw_string(oled, format_buffer);
    oled_send_buffer(oled);
    delay_ms(xms);
}

//
// @简介：设置剪切区域
// @参数：oled - OLED显示器的句柄
// @参数：x - 剪切区域左上角的横坐标
// @参数：y - 剪切区域左上角的纵坐标
// @参数：width - 剪切区域宽度
// @参数：height - 剪切区域高度
//
void oled_start_clip_region(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    oled->ClipRegionX = x;
    oled->ClipRegionY = y;
    oled->ClipRegionWidth = width;
    oled->ClipRegionHeight = height;
}

//
// @简介：关闭剪切区域
// @参数：oled - OLED显示器的句柄
//
void oled_stop_clip_region(OLED_TypeDef *oled)
{
    oled->ClipRegionX = 0;
    oled->ClipRegionY = 0;
    oled->ClipRegionWidth = 0;
    oled->ClipRegionHeight = 0;
}

//
// @简介：获取屏幕的宽度
// @返回值：屏幕宽度，单位：像素
//
uint16_t oled_get_screen_width(OLED_TypeDef *oled)
{
    return OLED_SCREEN_COLS;
}

//
// @简介：获取屏幕高度
// @返回值：屏幕高度，单位：像素
//
uint16_t oled_get_screen_height(OLED_TypeDef *oled)
{
    return OLED_SCREEN_ROWS;
}

//
// @简介：清空缓冲区内容
// @参数：oled - 显示器的句柄
//
void oled_clear(OLED_TypeDef *oled)
{
    memset(oled->pBuffer, 0, OLED_SCREEN_COLS * OLED_SCREEN_PAGES);
}

//
// @简介：设置字体
// @参数：oled - 显示器的句柄
// @参数：font - 字体，如果要使用默认字体，可填写&default_font
//
void oled_set_font(OLED_TypeDef *oled, const Font_TypeDef *font)
{
    oled->Font = font;
}

//
// @简介：设置画笔的颜色和宽度
// @参数：oled - 显示器的句柄
// @参数：pen_color - 画笔颜色
//             PEN_COLOR_TRANSPARENT - 透明
//             PEN_COLOR_WHITE - 白色
//             PEN_COLOR_BLACK - 黑色
// @参数：width - 画笔宽度
//
void oled_set_pen(OLED_TypeDef *oled, uint8_t pen_color, uint8_t width)
{
    oled->PenColor = pen_color;
    oled->PenWidth = width;
}

//
// @简介：设置画刷
// @参数：oled - 显示器的句柄
// @参数：brush_color - 画刷颜色
//             BRUSH_TRANSPARENT - 透明
//             BRUSH_WHITE - 白色
//             BRUSH_BLACK - 黑色
//
void oled_set_brush(OLED_TypeDef *oled, uint8_t brush_color)
{
    oled->Brush = brush_color;
}

//
// @简介：将光标设置到坐标点（x，y）处
// @参数：oled - 显示器的句柄
// @参数：x - 光标横坐标
// @参数：y - 光标纵坐标
//
void oled_set_cursor(OLED_TypeDef *oled, int16_t x, int16_t y)
{
    oled->CursorX = x;
    oled->CursorY = y;
}

//
// @简介：将光标的横坐标设置到x处
// @参数：oled - 显示器的句柄
// @参数：x - 光标横坐标
//
void oled_set_cursor_x(OLED_TypeDef *oled, int16_t x)
{
    oled->CursorX = x;
}

//
// @简介：将光标的纵坐标设置到y处
// @参数：oled - 显示器的句柄
// @参数：y - 光标纵坐标
//
void oled_set_cursor_y(OLED_TypeDef *oled, int16_t y)
{
    oled->CursorY = y;
}

//
// @简介：移动光标
// @参数：oled - 显示器的句柄
// @参数：dx - 横向移动的距离
// @参数：dy - 纵向移动的距离
//
void oled_move_cursor(OLED_TypeDef *oled, int16_t dx, int16_t dy)
{
    oled->CursorX += dx;
    oled->CursorY += dy;
}

//
// @简介：横向移动光标
// @参数：oled - 显示器的句柄
// @参数：dx - 横向移动的距离
//
void oled_move_cursor_x(OLED_TypeDef *oled, int16_t dx)
{
    oled->CursorX += dx;
}

//
// @简介：纵向移动光标
// @参数：oled - 显示器的句柄
// @参数：dy - 纵向移动的距离
//
void oled_move_cursor_y(OLED_TypeDef *oled, int16_t dy)
{
    oled->CursorY += dy;
}

//
// 简介：获取光标的当前位置
// @参数：oled - 显示器的句柄
// @参数：p_x_out - 输出参数，用于接收光标的横坐标
// @参数：p_y_out - 输出参数，用于接收光标的纵坐标
//
void oled_get_cursor(OLED_TypeDef *oled, int16_t *p_x_out, int16_t *p_y_out)
{
    *p_x_out = oled->CursorX;
    *p_y_out = oled->CursorY;
}

//
// 简介：获取光标的横坐标
// @参数：oled - 显示器的句柄
// @返回值：光标的横坐标值
//
int16_t oled_get_cursor_x(OLED_TypeDef *oled)
{
    return oled->CursorX;
}

//
// 简介：获取光标的纵坐标
// @参数：oled - 显示器的句柄
// @返回值：光标的纵坐标值
//
int16_t oled_get_cursor_y(OLED_TypeDef *oled)
{
    return oled->CursorY;
}

#define min(x1, x2) ((x1) > (x2) ? (x2) : (x1))
#define max(x1, x2) ((x1) > (x2) ? (x1) : (x2))

static Rect get_overlapped_rect(Rect rect1, Rect rect2)
{
    Rect ret = {0, 0, 0, 0};

    int16_t xl = max(rect1.X, rect2.X);
    int16_t xr = min(rect1.X + rect1.Width, rect2.X + rect2.Width);

    int16_t yt = max(rect1.Y, rect2.Y);
    int16_t yb = min(rect1.Y + rect1.Height, rect2.Y + rect2.Height);

    if (xl < xr && yt < yb) {
        ret.X = xl;
        ret.Y = yt;
        ret.Width = xr - xl;
        ret.Height = yb - yt;
    }

    return ret;
}

static uint16_t get_glyph_width(OLED_TypeDef *oled, uint32_t unicode)
{
    if (oled->Font == NULL) return 0; // 未设置字体

    int16_t idx = unicode_to_glyph_idx(oled, unicode);

    if (idx < 0) return 0; // 未找到对应的字形

    return oled->Font->Glyphs[idx].Dwx0;
}

static void draw_charator(OLED_TypeDef *oled, uint32_t unicode)
{
    if (oled->Font == NULL) return; // 未设置字体

    int16_t idx = unicode_to_glyph_idx(oled, unicode);

    const Glyph_TypeDef *p_glyph = 0;

    if (idx >= 0) {
        p_glyph = &oled->Font->Glyphs[idx];
    }

    int16_t clip_region_x_cpy, clip_region_y_cpy, clip_region_width_cpy, clip_region_height_cpy;

    // 如果启用了文本框区域
    if (oled->TextRegionWidth != 0 && oled->TextRegionHeight != 0) {
        // 备份剪切区域
        clip_region_x_cpy = oled->ClipRegionX;
        clip_region_y_cpy = oled->ClipRegionY;
        clip_region_width_cpy = oled->ClipRegionWidth;
        clip_region_height_cpy = oled->ClipRegionHeight;

        // 将剪切区域设置到与文本框区域相交处

        if (oled->ClipRegionWidth != 0 && oled->ClipRegionHeight != 0) {
            Rect rect1 = {oled->TextRegionX, oled->TextRegionY, oled->TextRegionWidth, oled->TextRegionHeight};
            Rect rect2 = {oled->ClipRegionX, oled->ClipRegionY, oled->ClipRegionWidth, oled->ClipRegionHeight};
            Rect overlapped = get_overlapped_rect(rect1, rect2);

            oled->ClipRegionX = overlapped.X;
            oled->ClipRegionY = overlapped.Y;
            oled->ClipRegionWidth = overlapped.Width;
            oled->ClipRegionHeight = overlapped.Height;
        } else {
            oled->ClipRegionX = oled->TextRegionX;
            oled->ClipRegionY = oled->TextRegionY;
            oled->ClipRegionWidth = oled->TextRegionWidth;
            oled->ClipRegionHeight = oled->TextRegionHeight;
        }

        // 如果光标在文本框之外
        if (oled->CursorX < oled->TextRegionX || oled->CursorX >= oled->TextRegionX + oled->TextRegionWidth) {
            oled->CursorX = oled->TextRegionX; // 让光标回到起始点
        }

        if (p_glyph != 0) {
            // 如果下一个字符的宽度超过了文本框
            if (oled->CursorX + p_glyph->Dwx0 >= oled->TextRegionX + oled->TextRegionWidth) {
                oled->CursorX = oled->TextRegionX; // 让光标回到起始点
                oled->CursorY = oled->CursorY + oled->Font->FBBy + oled->Font->FBBYoff;
            }
        }

        // 如果为\r（回车）
        if (unicode == '\r') {
            oled->CursorX = oled->TextRegionX;
        } else if (unicode == '\n') {
            oled->CursorY = oled->CursorY + oled->Font->FBBy + oled->Font->FBBYoff;
        }
    }

    if (p_glyph != 0) {
        // 绘制背景
        fill_rect(oled, oled->CursorX, oled->CursorY - oled->Font->FBBYoff - oled->Font->FBBy, p_glyph->Dwx0, oled->Font->FBBy);

        // 绘制字形
        draw_bitmap_ex(oled, oled->CursorX + p_glyph->BBxoff0x, oled->CursorY - p_glyph->BByoff0y - p_glyph->BBh,
                       p_glyph->BBw, p_glyph->BBh, p_glyph->Bitmap);
    }

    // 如果启用了文本框区域
    if (oled->TextRegionWidth != 0 && oled->TextRegionHeight != 0) {
        // 恢复剪切区域
        oled->ClipRegionX = clip_region_x_cpy;
        oled->ClipRegionY = clip_region_y_cpy;
        oled->ClipRegionWidth = clip_region_width_cpy;
        oled->ClipRegionHeight = clip_region_height_cpy;
    }
    if (p_glyph != 0) {
        oled->CursorX += p_glyph->Dwx0;
    }
}

//
// 简介：从光标处开始绘制字符串
// @参数：oled - 显示器的句柄
// @参数：str - 要绘制的字符串
//
void oled_draw_string(OLED_TypeDef *oled, const char *str)
{
    // 注意这里使用的是UTF-8编码，因此先将其解析成Unicode
    uint16_t i;
    uint32_t unicode;
    uint8_t first, second, third, forth;

    i = 0;
    for (;;) {
        first = str[i++];
        if (first == '\0') break;

        if ((first & (1 << 7)) == 0) // 1字节
        {
            unicode = first;
            draw_charator(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5)) == (1 << 7 | 1 << 6)) // 2字
        {
            first = first & 0x1f;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            unicode = ((uint32_t)first << 6) | second;
            draw_charator(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) == (1 << 7 | 1 << 6 | 1 << 5)) // 3字节
        {
            first = first & 0x0f;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            third = str[i++];
            if (third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
            third = third & 0x3f;

            unicode = ((uint32_t)first << 12) | ((uint32_t)second << 6) | third;
            draw_charator(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3)) == (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) // 4字节
        {
            first = first & 0x07;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            third = str[i++];
            if (third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
            third = third & 0x3f;

            forth = str[i++];
            if (forth == '\0' || ((forth & (1 << 7 | 1 << 6)) != 0x80)) break;
            forth = forth & 0x3f;

            unicode = ((uint32_t)first << 18) | ((uint32_t)second << 12) | ((uint32_t)second << 6) | forth;

            draw_charator(oled, unicode);
        }
    }
}

//
// @简介：设置文本框区域，同时将光标移动到文本框的第一个字符处
// @参数：oled - 显示器的句柄
// @参数：x - 文本框左上角的横坐标
// @参数：y - 文本框左上角的纵坐标
// @参数：width - 文本框的宽度
// @参数：height - 文本框的高度
//
void oled_start_text_region(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    oled->TextRegionX = x;
    oled->TextRegionY = y;
    oled->TextRegionWidth = width;
    oled->TextRegionHeight = height;

    oled->CursorX = x;
    oled->CursorY = y + oled_get_font_height(oled);
}

//
// @简介：取消文本框
// @参数：oled - 显示器的句柄
//
void oled_stop_text_region(OLED_TypeDef *oled)
{
    oled->TextRegionX = 0;
    oled->TextRegionY = 0;
    oled->TextRegionWidth = 0;
    oled->TextRegionHeight = 0;
}

//
// 简介：绘制格式化字符串（最大64字节）
// @参数：oled - 显示器的句柄
// @参数：format - 格式
// @参数：... - 可变参数
//
void oled_printf(OLED_TypeDef *oled, const char *format, ...)
{
    char format_buffer[64];

    va_list argptr;
    __va_start(argptr, format);
    vsprintf(format_buffer, format, argptr);
    __va_end(argptr);
    oled_draw_string(oled, format_buffer);
}

//
// @简介：获取当前字体下字符串所占的宽度
// @参数：oled - 显示器的句柄
// @参数：str - 字符串
// @返回值：宽度，单位：像素
//
uint16_t oled_get_str_width(OLED_TypeDef *oled, const char *str)
{
    // 注意这里使用的是UTF-8编码，因此先将其解析成Unicode
    uint16_t i;
    uint32_t unicode;
    uint8_t first, second, third, forth;
    uint16_t ret = 0;

    i = 0;
    for (;;) {
        first = str[i++];
        if (first == '\0') break;

        if ((first & (1 << 7)) == 0) // 1字节
        {
            unicode = first;
            ret += get_glyph_width(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5)) == (1 << 7 | 1 << 6)) // 2字
        {
            first = first & 0x1f;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            unicode = ((uint32_t)first << 6) | second;
            ret += get_glyph_width(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) == (1 << 7 | 1 << 6 | 1 << 5)) // 3字节
        {
            first = first & 0x0f;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            third = str[i++];
            if (third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
            third = third & 0x3f;

            unicode = ((uint32_t)first << 12) | ((uint32_t)second << 6) | third;
            ret += get_glyph_width(oled, unicode);
        } else if ((first & (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3)) == (1 << 7 | 1 << 6 | 1 << 5 | 1 << 4)) // 4字节
        {
            first = first & 0x07;

            second = str[i++];
            if (second == '\0' || ((second & (1 << 7 | 1 << 6)) != 0x80)) break;
            second = second & 0x3f;

            third = str[i++];
            if (third == '\0' || ((third & (1 << 7 | 1 << 6)) != 0x80)) break;
            third = third & 0x3f;

            forth = str[i++];
            if (forth == '\0' || ((forth & (1 << 7 | 1 << 6)) != 0x80)) break;
            forth = forth & 0x3f;

            unicode = ((uint32_t)first << 18) | ((uint32_t)second << 12) | ((uint32_t)second << 6) | forth;

            ret += get_glyph_width(oled, unicode);
        }
    }

    return ret;
}

//
// @简介：获取当前字体的最大高度
// @参数：oled - 显示器的句柄
// @返回值：字体最大高度
//
uint16_t oled_get_font_height(OLED_TypeDef *oled)
{
    return oled->Font->FontSize;
}

#define swap(x, y) do { (x) = (x) + (y); (y) = (x) - (y); (x) = (x) - (y); } while (0)

//
// @简介：画点
// @参数：oled - OLED显示器的句柄
// @参数：x - 横坐标
// @参数：y - 纵坐标
//
void oled_draw_dot(OLED_TypeDef *oled)
{
    pen_dot(oled, oled->CursorX, oled->CursorY);
}

//
// @简介：以当前光标位置为起点绘制直线
// @参数：oled - OLED显示器的句柄
// @参数：x - 终止点横坐标
// @参数：y - 终止点纵坐标
//
void oled_draw_line(OLED_TypeDef *oled, int16_t x, int16_t y)
{
    int16_t x0, y0;
    int16_t X0 = oled->CursorX;
    int16_t Y0 = oled->CursorY;
    int16_t X1 = x;
    int16_t Y1 = y;

    if (oled->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图

    if (X0 != X1) {
        if (X0 > X1) {
            swap(X0, X1);
            swap(Y0, Y1);
        }
        for (x0 = X0; x0 < X1; x0++) {
            if (x0 < 0 || x0 >= OLED_SCREEN_COLS) continue;
            y0 = (int16_t)round(1.0 * (Y1 - Y0) * (x0 - X0) / (X1 - X0) + Y0);
            if (y0 < 0 || y0 >= OLED_SCREEN_ROWS) continue;

            pen_dot(oled, x0, y0);
        }
    }
    if (Y0 != Y1) {
        if (Y0 > Y1) {
            swap(X0, X1);
            swap(Y0, Y1);
        }
        for (y0 = Y0; y0 < Y1; y0++) {
            if (y0 < 0 || y0 >= OLED_SCREEN_ROWS) continue;
            x0 = (int16_t)round(1.0 * (y0 - Y0) * (X1 - X0) / (Y1 - Y0) + X0);
            if (x0 < 0 || x0 >= OLED_SCREEN_COLS) continue;

            pen_dot(oled, x0, y0);
        }
    }
}

//
// @简介：以当前光标位置为起点绘制直线，绘制完成后光标移动到线段的终点
// @参数：oled - OLED显示器的句柄
// @参数：x - 终止点横坐标
// @参数：y - 终止点纵坐标
//
void oled_line_to(OLED_TypeDef *oled, int16_t x, int16_t y)
{
    oled_draw_line(oled, x, y);
    oled->CursorX = x;
    oled->CursorY = y;
}

static void draw_circle_frame(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t radius)
{
    int16_t x0, y0, distance;

    if (oled->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图

    if (x - radius >= OLED_SCREEN_COLS || x + radius < 0) return; // 绘图区域超出缓冲区范围
    if (y - radius >= OLED_SCREEN_ROWS || y + radius < 0) return; // 绘图区域超出缓冲区范围

    for (x0 = x - radius; x0 <= x + radius; x0++) {
        if (x0 < 0 || x0 > OLED_SCREEN_COLS) continue; // x坐标超出缓冲区范围
        for (distance = 0; distance <= radius; distance++) {
            if ((x0 - x) * (x0 - x) + (distance + 1) * (distance + 1) > radius * radius) // x^2 + y ^ 2 > radius^2
            {
                if (y + distance < OLED_SCREEN_ROWS && x0 >= 0 && x0 < OLED_SCREEN_COLS) {
                    pen_dot(oled, x0, y + distance);
                }
                if (y - distance > 0 && x0 >= 0 && x0 < OLED_SCREEN_COLS) {
                    pen_dot(oled, x0, y - distance);
                }
                break;
            }
        }
    }

    for (y0 = y - radius; y0 <= y + radius; y0++) {
        if (y0 < 0 || y0 > OLED_SCREEN_ROWS) continue;
        for (distance = 0; distance <= radius; distance++) {
            if ((y0 - y) * (y0 - y) + (distance + 1) * (distance + 1) > radius * radius) {
                if (x + distance < OLED_SCREEN_COLS && y0 >= 0 && y0 < OLED_SCREEN_ROWS) {
                    pen_dot(oled, x + distance, y0);
                }
                if (x - distance > 0 && y0 >= 0 && y0 < OLED_SCREEN_ROWS) {
                    pen_dot(oled, x - distance, y0);
                }
                break;
            }
        }
    }
}

static void fill_circle(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t radius)
{
    int16_t x0, distance;

    if (oled->Brush == BRUSH_TRANSPARENT) return; // 透明画刷不需要绘图

    if (x - radius >= OLED_SCREEN_COLS || x + radius < 0) return;
    if (y - radius >= OLED_SCREEN_ROWS || y + radius < 0) return;

    for (x0 = x - radius; x0 <= x + radius; x0++) {
        if (x0 < 0 || x0 > OLED_SCREEN_COLS) continue;
        for (distance = 0; distance <= radius; distance++) {
            if ((x0 - x) * (x0 - x) + distance * distance <= radius * radius) {
                if (y + distance < OLED_SCREEN_ROWS) {
                    brush_dot(oled, x0, y + distance);
                }
                if (y - distance > 0) {
                    brush_dot(oled, x0, y - distance);
                }
            } else {
                break;
            }
        }
    }
}

//
// @简介：以光标为圆心绘制圆形
// @参数：oled - OLED显示器的句柄
// @参数：radius - 圆的半径
//
void oled_draw_circle(OLED_TypeDef *oled, uint16_t radius)
{
    int16_t x = oled->CursorX, y = oled->CursorY;

    if (oled->PenColor != PEN_COLOR_TRANSPARENT) {
        draw_circle_frame(oled, x, y, radius);
    }

    if (oled->Brush != BRUSH_TRANSPARENT) {
        fill_circle(oled, x, y, radius);
    }
}

static void draw_rect_frame(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    int16_t x0, y0;

    if (oled->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画笔不需要绘图

    // 绘制左侧边
    x0 = x;
    if (x0 >= 0 && x0 < OLED_SCREEN_COLS) {
        for (y0 = max(0, y); y0 < y + height; y0++) {
            pen_dot(oled, x0, y0);
        }
    }

    // 绘制右侧边
    x0 = x + width - 1;
    if (width > 0 && x0 >= 0 && x0 < OLED_SCREEN_COLS) {
        for (y0 = max(0, y); y0 < y + height; y0++) {
            pen_dot(oled, x0, y0);
        }
    }

    // 绘制上边
    y0 = y;
    if (y0 >= 0 && y0 < OLED_SCREEN_ROWS) {
        for (x0 = max(0, x); x0 < x + width; x0++) {
            pen_dot(oled, x0, y0);
        }
    }

    // 绘制下边
    y0 = y + height - 1;
    if (y0 >= 0 && y0 < OLED_SCREEN_ROWS) {
        for (x0 = max(0, x); x0 < x + width; x0++) {
            pen_dot(oled, x0, y0);
        }
    }
}

static void fill_rect(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    if (oled->Brush == BRUSH_TRANSPARENT) return; // 透明画刷不需要绘图

    int16_t x0, y0;

    for (x0 = x; x0 < x + width; x0++) {
        for (y0 = y; y0 < y + height; y0++) {
            brush_dot(oled, x0, y0);
        }
    }
}

//
// @简介：以光标位置为左上角绘制矩形
// @参数：oled - OLED显示器的句柄
// @参数：width - 矩形宽度
// @参数：height - 矩形高度
//
void oled_draw_rect(OLED_TypeDef *oled, uint16_t width, uint16_t height)
{
    if (oled->PenColor != PEN_COLOR_TRANSPARENT) {
        draw_rect_frame(oled, oled->CursorX, oled->CursorY, width, height);
    }
    if (oled->Brush != BRUSH_TRANSPARENT) {
        fill_rect(oled, oled->CursorX, oled->CursorY, width, height);
    }
}

//
// @简介：开始分段发送缓冲区内容到屏幕（每个8*8的区域为一个单元进行发送）
// @参数：oled - OLED显示器的句柄
// @返回值：0 - 启动成功
//
int oled_start_send_buffer(OLED_TypeDef *oled)
{
    uint8_t arg[2];

    oled->RefreshProgress = 0;

    // 设置寻址模式为横向寻址模式
    arg[0] = 0x00;
    if (oled_send_command(oled, 0x20, arg, 1) != 0) return -1;

    // 设置列范围
    arg[0] = 0x00;
    arg[1] = 0x7f;
    if (oled_send_command(oled, 0x21, arg, 2) != 0) return -1;

    // 设置页范围
    arg[0] = 0x00;
    arg[1] = 0x07;
    if (oled_send_command(oled, 0x22, arg, 2) != 0) return -1;

    return 0;
}

//
// @简介：分段发送缓冲区内容到屏幕（每个8*8的区域为一个单元进行发送）
// @参数：oled - OLED显示器的句柄
// @参数：p_more_out - 输出参数，用于接收是否有后续数据需要发送
//                   0    - 当前分段为最后一个分段，至此所有分段均已发送结束
//                   非零 - 当前分段不是最后一个分段
// @返回值：0 - 成功
//
int oled_end_send_buffer(OLED_TypeDef *oled, uint8_t *p_more_out)
{
    if (oled->RefreshProgress >= 127) {
        *p_more_out = 0;
    } else {
        *p_more_out = 1;
    }

    if (oled->RefreshProgress >= 128) {
        return -1;
    }

    // 更新显示数据
    oled_send_data(oled, &oled->pBuffer[oled->RefreshProgress * 8], 8);

    oled->RefreshProgress = (oled->RefreshProgress + 1) % 128;

    return 0;
}

//
// @简介：将缓冲区数据一次性发送到屏幕
// @参数：oled - OLED显示器的句柄
// @返回值：0 - 成功
//
int oled_send_buffer(OLED_TypeDef *oled)
{
    uint8_t arg[2];

    // 设置寻址模式为横向寻址模式
    arg[0] = 0x00;
    if (oled_send_command(oled, 0x20, arg, 1) != 0) return -1;

    // 设置列范围
    arg[0] = 0x00;
    arg[1] = 0x7f;
    if (oled_send_command(oled, 0x21, arg, 2)) return -1;

    // 设置页范围
    arg[0] = 0x00;
    arg[1] = 0x07;
    if (oled_send_command(oled, 0x22, arg, 2)) return -1;

    // 更新显示数据
    if (oled_send_data(oled, oled->pBuffer, OLED_SCREEN_COLS * OLED_SCREEN_PAGES) != 0) {
        return -1;
    }

    return 0;
}

static int oled_send_command(OLED_TypeDef *oled, const uint8_t cmd, const uint8_t *arg, uint16_t size)
{
    uint8_t buf[8];
    uint8_t i;

    buf[0] = SSD1306_CTRL_COMMAND_STREAM;
    buf[1] = cmd;

    for (i = 0; i < size; i++) {
        buf[i + 2] = arg[i];
    }

    if (oled->i2c_write_cb(OLED_SLAVE_ADDR, buf, i + 2) != 0) {
        return -1; // 数据发送失败
    }

    return 0;
}

static int oled_send_data(OLED_TypeDef *oled, uint8_t *p_data, uint16_t size)
{
    int ret = 0;

    uint8_t tmp = *(p_data - 1);
    *(p_data - 1) = SSD1306_CTRL_DATA_STREAM;

    if (oled->i2c_write_cb(OLED_SLAVE_ADDR, p_data - 1, size + 1) != 0) {
        ret = -1; // 数据发送失败
    }

    *(p_data - 1) = tmp;

    return ret;
}

//
// @简介：绘制位图
// @参数：oled - OLED显示器的句柄
// @参数：x - 位图左上角的横坐标
// @参数：y - 位图左上角的纵坐标
// @参数：width - 位图宽度
// @参数：height - 位图高度
// @参数：p_bitmap - 位图数据
//
static void draw_bitmap_ex(OLED_TypeDef *oled, int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *p_bitmap)
{
    int16_t x0, y0;

    if (oled->Brush == BRUSH_TRANSPARENT && oled->PenColor == PEN_COLOR_TRANSPARENT) return; // 透明画刷不做任何操作

    uint16_t pen_width_cpy = oled->PenWidth;

    oled->PenWidth = 1;

    uint16_t n_bytes_per_row = (uint16_t)ceil(width / 8.0);

    for (x0 = 0; x0 < width; x0++) {
        for (y0 = 0; y0 < height; y0++) {
            // b0 b1 .. b7 b0 b1
            if ((p_bitmap[x0 / 8 + y0 * n_bytes_per_row] & (0x80 >> (x0 % 8))) != 0) {
                pen_dot(oled, x + x0, y + y0);
            } else {
                brush_dot(oled, x + x0, y + y0);
            }
        }
    }

    oled->PenWidth = pen_width_cpy;
}

//
// @简介：以光标位置为左上角绘制位图
// @参数：x - 位图左上角横坐标
// @参数：y - 位图左上角纵坐标
// @参数：width - 位图宽度
// @参数：height - 位图高度
// @参数：p_bitmap - 位图数据，格式：每个字节表示横向的8个像素点，行末尾不足8个像素的用0补齐
//
void oled_draw_bitmap(OLED_TypeDef *oled, uint16_t width, uint16_t height, const uint8_t *p_bitmap)
{
    draw_bitmap_ex(oled, oled->CursorX, oled->CursorY, width, height, p_bitmap);
}

//
// @简介：根据字符的unicode码获取字形序号
// @参数：oled - OLED显示器句柄
// @参数：unicode - 字符的Unicode编码
// @返回值：如果查找成功则返回字符序号，否则返回-1
//
static int16_t unicode_to_glyph_idx(OLED_TypeDef *oled, uint32_t unicode)
{
    int16_t ret = -1;

    if (oled->Font == 0) {
        return -1;
    }

    uint32_t i;

    for (i = 0; i < oled->Font->nChars; i++) {
        if (oled->Font->Map[i] == unicode) {
            ret = i;
            break;
        }
    }

    return ret;
}

static void pen_dot(OLED_TypeDef *oled, int16_t x, int16_t y)
{
    // 判断是否是透明画笔
    if (oled->PenColor == PEN_COLOR_TRANSPARENT) {
        return; // 透明画笔不需要绘图
    }

    if (oled->PenWidth == 0) {
        return;
    }

    uint16_t border_left, border_right, border_top, border_bottom;

    if (oled->PenWidth % 2) {
        border_left = border_right = border_top = border_bottom = oled->PenWidth / 2;
    } else {
        border_left = border_right = border_top = border_bottom = oled->PenWidth / 2;
        border_left--;
        border_top--;
    }

    // 绘图
    // 备份画刷
    uint8_t brush_cpy = oled->Brush;

    oled->Brush = oled->PenColor;

    int16_t i, j;

    for (i = x - border_left; i <= x + border_right; i++) {
        for (j = y - border_top; j <= y + border_bottom; j++) {
            brush_dot(oled, i, j);
        }
    }

    // 还原画刷
    oled->Brush = brush_cpy;
}

static void brush_dot(OLED_TypeDef *oled, int16_t x, int16_t y)
{
    // 判断是否是透明画笔
    if (oled->Brush == BRUSH_TRANSPARENT) {
        return; // 透明画笔不需要绘图
    }

    // 判断绘图点是否在屏幕外部
    if (x < 0 || x >= OLED_SCREEN_COLS) {
        return;
    }

    if (y < 0 || y >= OLED_SCREEN_ROWS) {
        return;
    }

    // 判断是否在绘图区域的外部
    if (oled->ClipRegionWidth != 0 && oled->ClipRegionHeight != 0) {
        if (x < oled->ClipRegionX || x >= oled->ClipRegionX + oled->ClipRegionWidth) {
            return;
        }
        if (y < oled->ClipRegionY || y >= oled->ClipRegionY + oled->ClipRegionHeight) {
            return;
        }
    }

    // 绘图
    if (oled->Brush == BRUSH_WHITE) // 点亮
    {
        oled->pBuffer[x + y / 8 * OLED_SCREEN_COLS] |= 1 << (y % 8);
    } else // 熄灭
    {
        oled->pBuffer[x + y / 8 * OLED_SCREEN_COLS] &= ~(1 << (y % 8));
    }
}
