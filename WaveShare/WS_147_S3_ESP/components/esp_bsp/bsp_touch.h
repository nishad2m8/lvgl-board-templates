#ifndef __BSP_TOUCH_H__
#define __BSP_TOUCH_H__

#include "driver/i2c_master.h"
#include "esp_lcd_touch_axs5106.h"


#define EXAMPLE_PIN_TP_INT GPIO_NUM_47
#define EXAMPLE_PIN_TP_RST GPIO_NUM_48


#ifdef __cplusplus
extern "C" {
#endif

void bsp_touch_init(esp_lcd_touch_handle_t *touch_handle, i2c_master_bus_handle_t bus_handle, uint16_t xmax, uint16_t ymax, uint16_t rotation);
#ifdef __cplusplus
}
#endif


#endif