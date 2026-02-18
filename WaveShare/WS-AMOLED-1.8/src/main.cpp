#include <lvgl.h>
#include "HWCDC.h"
#include <SensorQMI8658.hpp>
#include <Wire.h>

#include "pin_config.h"
#include "display.h"
#include "ui.h"

HWCDC USBSerial;

// IMU for rotation detection
SensorQMI8658 qmi;
IMUdata acc;
float angleX = 1;
float angleY = 0;
bool rotation = false;

void setup() {
    USBSerial.begin(115200);

    // Initialize display and LVGL
    display_init();

    // Initialize touch
    touch_init();

    // Initialize IMU for rotation detection
    if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
        USBSerial.println("Failed to find QMI8658 - check your wiring!");
        while (1) {
            delay(1000);
        }
    }
    qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
    qmi.enableAccelerometer();

    // Initialize UI
    ui_init();

    USBSerial.println("Setup done");
}

void loop() {
    // Handle rotation based on IMU data
    if (qmi.getDataReady()) {
        if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
            angleX = acc.x;
            angleY = acc.y;
            if (angleX > 0.8 && !rotation) {
                lv_disp_set_rotation(NULL, LV_DISP_ROT_NONE);
                rotation = true;
            } else if (angleX < -0.8 && !rotation) {
                lv_disp_set_rotation(NULL, LV_DISP_ROT_180);
                rotation = true;
            } else if (angleY < -0.8 && !rotation) {
                lv_disp_set_rotation(NULL, LV_DISP_ROT_90);
                rotation = true;
            } else if (angleY > 0.8 && !rotation) {
                lv_disp_set_rotation(NULL, LV_DISP_ROT_270);
                rotation = true;
            }
            if ((angleX <= 0.8 && angleX >= -0.8) && (angleY <= 0.8 && angleY >= -0.8)) {
                rotation = false;
            }
        }
    }

    lv_timer_handler();
    delay(5);
}
