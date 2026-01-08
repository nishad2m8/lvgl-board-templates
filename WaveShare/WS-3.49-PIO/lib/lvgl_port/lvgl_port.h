#ifndef LVGL_PORT_H
#define LVGL_PORT_H



#ifdef __cplusplus
extern "C" {
#endif


void lvgl_port_init(void);
void lvgl_port_run_with_gui(void (*init_fn)(void));


#ifdef __cplusplus
}
#endif



#endif









