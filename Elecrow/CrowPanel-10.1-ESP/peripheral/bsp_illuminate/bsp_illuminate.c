/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include "bsp_illuminate.h"  // Include BSP display header (LCD and related configurations)
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/

esp_lcd_panel_handle_t panel_handle = NULL;          /* Type of LCD panel handle */
esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;        /* Type of LCD panel IO handle (consumed by lvgl_display) */
static esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL; /* Type of MIPI DSI bus handle */

/*————————————————————————————————————————Variable declaration end———————————————————————————————————————*/

/*—————————————————————————————————————————Functional function———————————————————————————————————————————*/

static esp_err_t blight_init(void)  // Initialize LCD backlight (PWM control)
{
    esp_err_t err = ESP_OK;  // Error status variable
    const gpio_config_t gpio_cofig = {  // GPIO configuration for backlight pin
        .pin_bit_mask = (1ULL << LCD_GPIO_BLIGHT),  // Select backlight GPIO pin
        .mode = GPIO_MODE_OUTPUT,                   // Configure as output
        .pull_up_en = false,                        // Disable pull-up
        .pull_down_en = false,                      // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE,             // Disable interrupt
    };
    err = gpio_config(&gpio_cofig);  // Apply GPIO config
    if (err != ESP_OK)
        return err;  // Return error if failed

    const ledc_timer_config_t timer_config = {  // LEDC timer configuration
        .clk_cfg = LEDC_USE_PLL_DIV_CLK,        // Use PLL clock
        .duty_resolution = LEDC_TIMER_11_BIT,   // 11-bit duty resolution
        .freq_hz = BLIGHT_PWM_Hz,               // Backlight PWM frequency
        .speed_mode = LEDC_LOW_SPEED_MODE,      // Low-speed mode
        .timer_num = LEDC_TIMER_0,              // Timer 0
    };

    const ledc_channel_config_t channel_config = {  // LEDC channel configuration
        .gpio_num = LCD_GPIO_BLIGHT,                // Backlight GPIO pin
        .speed_mode = LEDC_LOW_SPEED_MODE,          // Low-speed mode
        .channel = LEDC_CHANNEL_0,                  // Channel 0
        .intr_type = LEDC_INTR_DISABLE,             // Disable interrupt
        .timer_sel = LEDC_TIMER_0,                  // Use timer 0
        .duty = 0,                                  // Initial duty = 0
        .hpoint = 0,                                // Start point
    };
    err = ledc_timer_config(&timer_config);  // Configure LEDC timer
    if (err != ESP_OK)
        return err;
    err = ledc_channel_config(&channel_config);  // Configure LEDC channel
    if (err != ESP_OK)
        return err;     
    return err;  // Return success
}

/* brightness -  (0 - 100) */
esp_err_t set_lcd_blight(uint32_t brightness)  // Set LCD backlight brightness
{
    esp_err_t err = ESP_OK;
    if (brightness != 0)  // If brightness > 0
    {
        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, ((brightness * 18) + 200));  // Calculate PWM duty
        if (err != ESP_OK)
            return err;
        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);  // Apply duty update
        if (err != ESP_OK)
            return err;
    }
    else  // If brightness = 0, turn off
    {
        err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);  // Set duty = 0
        if (err != ESP_OK)
            return err;
        err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);  // Apply duty update
        if (err != ESP_OK)
            return err;
    }
    return err;  // Return status
}

static esp_err_t display_port_init(void)  // Initialize LCD port (MIPI DSI + panel config)
{
    esp_err_t err = ESP_OK;
    lcd_color_format_t dpi_pixel_format;  // Pixel format variable
    esp_lcd_dsi_bus_config_t bus_config = {  // MIPI DSI bus config
        .bus_id = 0,                        // Bus ID = 0
        .num_data_lanes = 2,                // 2 data lanes
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,  // Default PHY clock source
        .lane_bit_rate_mbps = 900,          // Lane bit rate = 900 Mbps
    };
    err = esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus);  // Create new DSI bus
    if (err != ESP_OK)
        return err;
    esp_lcd_dbi_io_config_t dbi_config = {  // DBI interface config
        .virtual_channel = 0,               // Virtual channel = 0
        .lcd_cmd_bits = 8,                  // 8-bit command
        .lcd_param_bits = 8,                // 8-bit parameter
    };
    err = esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io);  // Create new DBI IO
    if (err != ESP_OK)
        return err; 
    if (BITS_PER_PIXEL == 24)
        dpi_pixel_format = LCD_COLOR_FMT_RGB888;  // 24-bit RGB888
    else if (BITS_PER_PIXEL == 18)
        // IDF v6 lcd_color_format_t has no RGB666; fall back to RGB888 storage
        dpi_pixel_format = LCD_COLOR_FMT_RGB888;
    else if (BITS_PER_PIXEL == 16)
        dpi_pixel_format = LCD_COLOR_FMT_RGB565;  // 16-bit RGB565

    const esp_lcd_dpi_panel_config_t dpi_config = {  // DPI panel config
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,  // Default DPI clock source
        .dpi_clock_freq_mhz = 51,                     // DPI clock frequency = 51 MHz
        .virtual_channel = 0,                         // Virtual channel = 0
        .in_color_format = dpi_pixel_format,          // Frame buffer pixel format
        .out_color_format = dpi_pixel_format,         // Panel output pixel format
        .num_fbs = 1,                                 // Frame buffers = 1
        .video_timing = {                             // Video timing parameters
            .h_size = H_size,                         // Horizontal size
            .v_size = V_size,                         // Vertical size
            .hsync_back_porch = 160,                  // HSync back porch
            .hsync_pulse_width = 70,                  // HSync pulse width
            .hsync_front_porch = 160,                 // HSync front porch
            .vsync_back_porch = 23,                   // VSync back porch
            .vsync_pulse_width = 10,                  // VSync pulse width
            .vsync_front_porch = 12,                  // VSync front porch
        },
    };

    ek79007_vendor_config_t vendor_config = {  // Vendor-specific config
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,        // DSI bus handle
            .dpi_config = &dpi_config,      // DPI config reference
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {  // Panel device config
        .reset_gpio_num = -1,                // No reset GPIO
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,  // RGB order
        .bits_per_pixel = BITS_PER_PIXEL,    // Pixel depth
        .vendor_config = &vendor_config,     // Vendor config pointer
    };
    err = esp_lcd_new_panel_ek79007(mipi_dbi_io, &panel_config, &panel_handle);  // Create new EK79007 panel
    if (err != ESP_OK)
        return err;
    err = esp_lcd_panel_reset(panel_handle);  // Reset panel
    if (err != ESP_OK)
        return err;
    err = esp_lcd_panel_init(panel_handle);   // Initialize panel
    if (err != ESP_OK)
        return err;
    return err;  // Return success
}

esp_err_t display_init()  // Display hardware initialization (backlight + MIPI panel)
{
    esp_err_t err = blight_init();
    if (err != ESP_OK)
        return err;
    err = display_port_init();
    if (err != ESP_OK) {
        ILLUMINATE_ERROR("Display init fail");
        return err;
    }
    set_lcd_blight(0);
    return err;
}

/*———————————————————————————————————————Functional function end—————————————————————————————————————————*/
