/*
 * This file is part of gol-bluepill-12864.
 *
 * gol-bluepill-12864 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gol-bluepill-12864 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gol-bluepill-12864.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <libopencm3/stm32/spi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

typedef uint8_t arr_p[SCREEN_WIDTH][SCREEN_HEIGHT];

void us_delay(uint16_t nano_secs);

void send_command(uint8_t cmd);
void send_data(uint8_t dat);

void set_display_control(bool is_display_on, bool is_cursor_on,
                         bool is_blink_on);
void clear_display(void);
void cursor_return_home(void);

void init_display(bool use_graphics_mode);

void draw_entire_display(arr_p *pixels);
