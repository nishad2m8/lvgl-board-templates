/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void board_init(void);
bool board_ui_lock(uint32_t timeout_ms);
void board_ui_unlock(void);

#ifdef __cplusplus
}
#endif
