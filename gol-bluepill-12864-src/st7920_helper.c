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

#include "st7920_helper.h"

// One NOP at about 72MHz is 0.0139 us
// Testing shows 7 multiplier is about right
void us_delay(uint16_t micro_secs) {
  for (uint32_t i = 0; i < (micro_secs * 7); i++)
    __asm__("nop");
}

void send_command(uint8_t cmd) {
  spi_send(SPI1, 0b11111000);        // Sync and RS=0
  spi_send(SPI1, cmd & 0xF0);        // Upper nibble
  spi_send(SPI1, (cmd << 4) & 0xF0); // Lower nibble
  us_delay(50);
}

void send_data(uint8_t dat) {
  spi_send(SPI1, 0b11111010);        // Sync and RS=1
  spi_send(SPI1, dat & 0xF0);        // Upper nibble
  spi_send(SPI1, (dat << 4) & 0xF0); // Lower nibble
  us_delay(50);
}

void set_display_control(bool is_display_on, bool is_cursor_on,
                         bool is_blink_on) {
  // Display ON/OFF, Curoser ON/OFF, Blink ON/OFF
  send_command(0b00001000 | (is_display_on << 2) | (is_cursor_on << 1) |
               (is_blink_on << 0));
  us_delay(110);
}

void clear_display(void) {
  send_command(0b00000001);
  us_delay(12000);
}

void cursor_return_home(void) {
  send_command(0b00000010);
  us_delay(1000);
}

void init_display(bool use_graphics_mode) {
  send_command(0x30); // 8 bit mode
  us_delay(110);
  set_display_control(false, false, false);
  clear_display();
  set_display_control(true, false, false);
  cursor_return_home();

  if (use_graphics_mode) {
    // 8-bit mode
    send_command(0b00110000);
    us_delay(1000);
    // Use extended instruction set
    send_command(0b00110100);
    us_delay(1000);
    // Enable Graphic Display On
    send_command(0b00110110);
    us_delay(1000);
    clear_display();
  }
}

void draw_entire_display(arr_p *pixels) {
  for (uint8_t y = 0; y < (SCREEN_HEIGHT / 2); y++) {
    for (uint8_t x = 0; x < (SCREEN_WIDTH / 8); x++) {
      // Vertical Address
      send_command(0b10000000 | (y & 0b00111111));
      // Horizontal Address
      send_command(0b10000000 | (x & 0b00001111));

      uint16_t sixteen_pixels = 0;
      for (uint8_t i = 0; i < 16; i++) {
        if (x < 8) {
          sixteen_pixels |= (((*pixels)[x * 16 + i][y] & 0x01) << (15 - i));

        } else {
          sixteen_pixels |=
              (((*pixels)[(x - 8) * 16 + i][y + 32] & 0x01) << (15 - i));
        }
      }

      send_data((sixteen_pixels >> 8) & 0xFF);
      send_data(sixteen_pixels & 0xFF);
    }
  }
}