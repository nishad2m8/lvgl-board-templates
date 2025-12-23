#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

// Event groups for button events
extern EventGroupHandle_t boot_groups;
extern EventGroupHandle_t pwr_groups;

// Bit manipulation macros
#define set_bit_button(x) ((uint32_t)(0x01)<<(x))
#define get_bit_button(x,y) (((uint32_t)(x)>>(y)) & 0x01)
#define set_bit_all 0x00ffffff

// Initialize button system
void button_handler_init(void);

// Get button repeat count
uint8_t button_handler_get_pwr_repeat_count(void);
uint8_t button_handler_get_boot_repeat_count(void);

// Button callback function type for user code
typedef void (*button_event_callback_t)(uint8_t button_id, uint8_t event_type);

// Register callback for button events
void button_handler_register_callback(button_event_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_HANDLER_H
