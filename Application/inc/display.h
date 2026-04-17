#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "acquisition.h"  
#include "oled.h"

/* 传输模式类型 */
typedef enum {
    DISPLAY_MODE_LORA,
    DISPLAY_MODE_NBIOT
} display_mode_t;

void display_init(void);
void display_sensor_data(const sensor_data_t *data, display_mode_t mode);
void display_error(const char *msg);
void display_clear(void);

#endif
