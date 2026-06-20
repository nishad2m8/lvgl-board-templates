// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2024 mzyy94

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/pcnt.h>
#include <driver/gpio.h>
#include "encoder.hpp"

#define PCNT_LOW_LIMIT -32768
#define PCNT_HIGH_LIMIT 32767

Encoder::Encoder() {
  _pcnt_unit = PCNT_UNIT_MAX;
}

Encoder::~Encoder() {
  pcnt_counter_pause(_pcnt_unit);
  pcnt_counter_clear(_pcnt_unit);
}

void Encoder::setup(gpio_num_t pin_a, gpio_num_t pin_b) {
  pcnt_config_t pcnt_config = {
    .pulse_gpio_num = pin_a,
    .ctrl_gpio_num = pin_b,
    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_REVERSE,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DEC,
    .counter_h_lim = PCNT_HIGH_LIMIT,
    .counter_l_lim = PCNT_LOW_LIMIT,
    .unit = _pcnt_unit,
    .channel = PCNT_CHANNEL_0,
  };
  pcnt_unit_config(&pcnt_config);

  pcnt_counter_pause(_pcnt_unit);
  pcnt_counter_clear(_pcnt_unit);
  pcnt_counter_resume(_pcnt_unit);
}

int Encoder::getCount(bool clear) {
  int16_t count = 0;
  pcnt_get_counter_value(_pcnt_unit, &count);
  if (clear) {
    pcnt_counter_clear(_pcnt_unit);
  }
  return count;
}

void Encoder::reset() {
  pcnt_counter_clear(_pcnt_unit);
}