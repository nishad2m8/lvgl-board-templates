// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2024 mzyy94

#include <driver/pcnt.h>
#include <driver/gpio.h>

class Encoder
{
private:
    gpio_num_t _pin_a;
    gpio_num_t _pin_b;

    pcnt_unit_t _pcnt_unit;

public:
    Encoder();
    ~Encoder();
    void setup(gpio_num_t pin_a = GPIO_NUM_40, gpio_num_t pin_b = GPIO_NUM_41);
    int getCount(bool clear = false);
    void reset();
};