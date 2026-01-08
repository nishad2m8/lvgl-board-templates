#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_port.h"
#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "user_config.h"
#include "driver/spi_master.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_axs15231b.h"
#include "i2c_bsp.h"

static const char *TAG = "lvgl_port";
static SemaphoreHandle_t lvgl_mux = NULL;

static uint16_t *trans_buf_1 = NULL; 
uint8_t *lvgl_dest = NULL;                //旋转buffer
static SemaphoreHandle_t flush_done_semaphore;

#define LCD_BIT_PER_PIXEL 16
#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
#define BUFF_SIZE (EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * BYTES_PER_PIXEL)


static const axs15231b_lcd_init_cmd_t lcd_init_cmds[] = 
{
  {0x11, (uint8_t []){0x00}, 0, 100},
  {0x29, (uint8_t []){0x00}, 0, 100},
};

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
  BaseType_t TaskWoken;
  xSemaphoreGiveFromISR(flush_done_semaphore,&TaskWoken);
  return false;
}

static void example_increase_lvgl_tick(void *arg)
{
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void example_lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
  lv_draw_sw_rgb565_swap(color_p, lv_area_get_width(area) * lv_area_get_height(area));
#if (Rotated == USER_DISP_ROT_90)
  lv_display_rotation_t rotation = lv_display_get_rotation(disp);
  lv_area_t rotated_area;
  if(rotation != LV_DISPLAY_ROTATION_0)
  {
    lv_color_format_t cf = lv_display_get_color_format(disp);
    /*Calculate the position of the rotated area*/
    rotated_area = *area;
    lv_display_rotate_area(disp, &rotated_area);
    /*Calculate the source stride (bytes in a line) from the width of the area*/
    uint32_t src_stride = lv_draw_buf_width_to_stride(lv_area_get_width(area), cf);
    /*Calculate the stride of the destination (rotated) area too*/
    uint32_t dest_stride = lv_draw_buf_width_to_stride(lv_area_get_width(&rotated_area), cf);
    /*Have a buffer to store the rotated area and perform the rotation*/
    
    int32_t src_w = lv_area_get_width(area);
    int32_t src_h = lv_area_get_height(area);
    lv_draw_sw_rotate(color_p, lvgl_dest, src_w, src_h, src_stride, dest_stride, rotation, cf);
    /*Use the rotated area and rotated buffer from now on*/
    area = &rotated_area;
  }

  const int flush_coun = (LVGL_SPIRAM_BUFF_LEN / LVGL_DMA_BUFF_LEN);
  const int offgap = (EXAMPLE_LCD_V_RES / flush_coun);
  const int dmalen = (LVGL_DMA_BUFF_LEN / 2);
  int offsetx1 = 0;
  int offsety1 = 0;
  int offsetx2 = EXAMPLE_LCD_H_RES;
  int offsety2 = offgap;

  uint16_t *map = (uint16_t *)lvgl_dest;
  xSemaphoreGive(flush_done_semaphore);
  for(int i = 0; i<flush_coun; i++)
  {
    xSemaphoreTake(flush_done_semaphore,portMAX_DELAY);
    memcpy(trans_buf_1,map,LVGL_DMA_BUFF_LEN);
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2, offsety2, trans_buf_1);
    offsety1 += offgap;
    offsety2 += offgap;
    map += dmalen;
  }
  xSemaphoreTake(flush_done_semaphore,portMAX_DELAY);
  lv_disp_flush_ready(disp);
#else
  const int flush_coun = (LVGL_SPIRAM_BUFF_LEN / LVGL_DMA_BUFF_LEN);
  const int offgap = (EXAMPLE_LCD_V_RES / flush_coun);
  const int dmalen = (LVGL_DMA_BUFF_LEN / 2);
  int offsetx1 = 0;
  int offsety1 = 0;
  int offsetx2 = EXAMPLE_LCD_H_RES;
  int offsety2 = offgap;

  uint16_t *map = (uint16_t *)color_p;
  xSemaphoreGive(flush_done_semaphore);
  for(int i = 0; i<flush_coun; i++)
  {
    xSemaphoreTake(flush_done_semaphore,portMAX_DELAY);
    memcpy(trans_buf_1,map,LVGL_DMA_BUFF_LEN);
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2, offsety2, trans_buf_1);
    offsety1 += offgap;
    offsety2 += offgap;
    map += dmalen;
  }
  xSemaphoreTake(flush_done_semaphore,portMAX_DELAY);
  lv_disp_flush_ready(disp);
#endif
}
static void TouchInputReadCallback(lv_indev_t * indev, lv_indev_data_t *indevData)
{
  uint8_t read_touchpad_cmd[11] = {0xb5, 0xab, 0xa5, 0x5a, 0x0, 0x0, 0x0, 0x0e,0x0, 0x0, 0x0};
  uint8_t buff[32] = {0};
  ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_read_dev(disp_touch_dev_handle,read_touchpad_cmd,11,buff,32));
  uint16_t pointX;
  uint16_t pointY;
  pointX = (((uint16_t)buff[2] & 0x0f) << 8) | (uint16_t)buff[3];
  pointY = (((uint16_t)buff[4] & 0x0f) << 8) | (uint16_t)buff[5];
  ESP_LOGI("Touch","%d,%d",buff[0],buff[1]);
  if (buff[1]>0 && buff[1]<5)
  {
    indevData->state = LV_INDEV_STATE_PRESSED;
    if(pointX > EXAMPLE_LCD_V_RES) pointX = EXAMPLE_LCD_V_RES;
    if(pointY > EXAMPLE_LCD_H_RES) pointY = EXAMPLE_LCD_H_RES;
    indevData->point.x = pointY;
    indevData->point.y = (EXAMPLE_LCD_V_RES-pointX);
  }
  else 
  {
    indevData->state = LV_INDEV_STATE_RELEASED;
  }
}

static bool example_lvgl_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;       
}

static void example_lvgl_unlock(void)
{
  assert(lvgl_mux && "bsp_display_start must be called first");
  xSemaphoreGive(lvgl_mux);
}

void example_lvgl_port_task(void *arg)
{
  uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
  for(;;)
  {
    if (example_lvgl_lock(-1)) 
    {
      task_delay_ms = lv_timer_handler();
      //Release the mutex
      example_lvgl_unlock();
    }
    if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS)
    {
      task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS)
    {
      task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }
}

void lvgl_port_init(void)
{
  flush_done_semaphore = xSemaphoreCreateBinary();
  assert(flush_done_semaphore);
  ESP_LOGI(TAG, "Initialize LCD RESET GPIO");

  gpio_config_t gpio_conf = {};
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask = ((uint64_t)0X01<<EXAMPLE_PIN_NUM_LCD_RST);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));

  ESP_LOGI(TAG, "Initialize QSPI bus");
  spi_bus_config_t buscfg = {};
    buscfg.data0_io_num = EXAMPLE_PIN_NUM_LCD_DATA0;
    buscfg.data1_io_num = EXAMPLE_PIN_NUM_LCD_DATA1;
    buscfg.sclk_io_num = EXAMPLE_PIN_NUM_LCD_PCLK;
    buscfg.data2_io_num = EXAMPLE_PIN_NUM_LCD_DATA2;
    buscfg.data3_io_num = EXAMPLE_PIN_NUM_LCD_DATA3;
    buscfg.max_transfer_sz = LVGL_DMA_BUFF_LEN;
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

  ESP_LOGI(TAG, "Install panel IO");
	  esp_lcd_panel_io_handle_t panel_io = NULL;
    esp_lcd_panel_handle_t panel = NULL;
    
  esp_lcd_panel_io_spi_config_t io_config = {};
	io_config.cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS;                 
    io_config.dc_gpio_num = -1;          
    io_config.spi_mode = 3;              
    io_config.pclk_hz = 40 * 1000 * 1000;
    io_config.trans_queue_depth = 10;    
    io_config.on_color_trans_done = example_notify_lvgl_flush_ready; 
    //io_config.user_ctx = &disp_drv,         
    io_config.lcd_cmd_bits = 32;         
    io_config.lcd_param_bits = 8;        
    io_config.flags.quad_mode = true;                         
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &io_config, &panel_io));
    
  axs15231b_vendor_config_t vendor_config = {};
    vendor_config.flags.use_qspi_interface = 1;
    vendor_config.init_cmds = lcd_init_cmds;
    vendor_config.init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]);
    
  esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = -1;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = LCD_BIT_PER_PIXEL;
    panel_config.vendor_config = &vendor_config;

  ESP_LOGI(TAG, "Install panel driver");
  ESP_ERROR_CHECK(esp_lcd_new_panel_axs15231b(panel_io, &panel_config, &panel));
    
	ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_LCD_RST,1));
  vTaskDelay(pdMS_TO_TICKS(30));
  ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_LCD_RST,0));
  vTaskDelay(pdMS_TO_TICKS(250));
  ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_LCD_RST,1));
  vTaskDelay(pdMS_TO_TICKS(30));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel));

  lv_init();

  lv_display_t * disp = lv_display_create(EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);  /* 以水平和垂直分辨率（像素）进行基本初始化 */
  lv_display_set_flush_cb(disp, example_lvgl_flush_cb);                           /* 设置刷新回调函数以绘制到显示屏 */
  
  uint8_t *buffer_1 = NULL;
  uint8_t *buffer_2 = NULL;
  buffer_1 = (uint8_t *)heap_caps_malloc(BUFF_SIZE, MALLOC_CAP_SPIRAM);
  buffer_2 = (uint8_t *)heap_caps_malloc(BUFF_SIZE, MALLOC_CAP_SPIRAM);
  assert(buffer_1);
  assert(buffer_2);
	trans_buf_1 = (uint16_t *)heap_caps_malloc(LVGL_DMA_BUFF_LEN, MALLOC_CAP_DMA);
	assert(trans_buf_1);
  lv_display_set_buffers(disp, buffer_1, buffer_2, BUFF_SIZE, LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_user_data(disp, panel);
#if (Rotated == USER_DISP_ROT_90)
    lvgl_dest = (uint8_t *)heap_caps_malloc(BUFF_SIZE, MALLOC_CAP_SPIRAM); //旋转buf
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
#endif
  /*port indev*/
  lv_indev_t *touch_indev = NULL;
  touch_indev = lv_indev_create();
  lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch_indev, TouchInputReadCallback);

  ESP_LOGI(TAG, "Install LVGL tick timer");
  esp_timer_create_args_t lvgl_tick_timer_args = {};
    lvgl_tick_timer_args.callback = &example_increase_lvgl_tick;
    lvgl_tick_timer_args.name = "lvgl_tick";
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer,LVGL_TICK_PERIOD_MS * 1000));

  lvgl_mux = xSemaphoreCreateMutex();
  assert(lvgl_mux);
  xTaskCreatePinnedToCore(example_lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL,0);
}

void lvgl_port_run_with_gui(void (*init_fn)(void))
{
  if (init_fn == NULL)
  {
    ESP_LOGW(TAG, "lvgl_port_run_with_gui called with NULL init_fn");
    return;
  }

  if (example_lvgl_lock(-1))
  {
    init_fn();
    example_lvgl_unlock();
  }
  else
  {
    ESP_LOGW(TAG, "Failed to acquire LVGL mutex for GUI init");
  }
}
