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

#include "gol_main.h"

static void clock_setup(void) {
  rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOA);

  rcc_periph_clock_enable(RCC_SPI1);
}

// SCK - Master - Alternate function push-pull
// COPI - - Aliternate function push-pull
// PA5 SCK Alternate function 0
// PA7 COPI Alternate function 0

static void gpio_setup(void) {
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO13);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO5 | GPIO7);
}

static void spi_setup(void) {
  spi_reset(SPI1);
  spi_init_master(
      SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
      SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
  spi_enable_software_slave_management(SPI1);
  spi_set_nss_high(SPI1);
  spi_enable(SPI1);
}

static void randomize_state(void) {
  // Update random_state with Marsaglia's Xorshift
  // https://en.wikipedia.org/wiki/Xorshift
  for (uint32_t i = 0; i < (SCREEN_HEIGHT * SCREEN_WIDTH / 32); i++) {
    uint32_t x = random_state[i];
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    random_state[i] = x;
  }

  // Bring each bit of the random_state to the first bit of state
  for (uint8_t i = 0; i < SCREEN_WIDTH; i++) {
    state[i][0] = (random_state[i] >> 0) & 0x01;
    state[i][1] = (random_state[i] >> 1) & 0x01;
    state[i][2] = (random_state[i] >> 2) & 0x01;
    state[i][3] = (random_state[i] >> 3) & 0x01;
    state[i][4] = (random_state[i] >> 4) & 0x01;
    state[i][5] = (random_state[i] >> 5) & 0x01;
    state[i][6] = (random_state[i] >> 6) & 0x01;
    state[i][7] = (random_state[i] >> 7) & 0x01;
    state[i][8] = (random_state[i] >> 8) & 0x01;
    state[i][9] = (random_state[i] >> 9) & 0x01;
    state[i][10] = (random_state[i] >> 10) & 0x01;
    state[i][11] = (random_state[i] >> 11) & 0x01;
    state[i][12] = (random_state[i] >> 12) & 0x01;
    state[i][13] = (random_state[i] >> 13) & 0x01;
    state[i][14] = (random_state[i] >> 14) & 0x01;
    state[i][15] = (random_state[i] >> 15) & 0x01;
    state[i][16] = (random_state[i] >> 16) & 0x01;
    state[i][17] = (random_state[i] >> 17) & 0x01;
    state[i][18] = (random_state[i] >> 18) & 0x01;
    state[i][19] = (random_state[i] >> 19) & 0x01;
    state[i][20] = (random_state[i] >> 20) & 0x01;
    state[i][21] = (random_state[i] >> 21) & 0x01;
    state[i][22] = (random_state[i] >> 22) & 0x01;
    state[i][23] = (random_state[i] >> 23) & 0x01;
    state[i][24] = (random_state[i] >> 24) & 0x01;
    state[i][25] = (random_state[i] >> 25) & 0x01;
    state[i][26] = (random_state[i] >> 26) & 0x01;
    state[i][27] = (random_state[i] >> 27) & 0x01;
    state[i][28] = (random_state[i] >> 28) & 0x01;
    state[i][29] = (random_state[i] >> 29) & 0x01;
    state[i][30] = (random_state[i] >> 30) & 0x01;
    state[i][31] = (random_state[i] >> 31) & 0x01;
    state[i][32] = (random_state[i + 128] >> 0) & 0x01;
    state[i][33] = (random_state[i + 128] >> 1) & 0x01;
    state[i][34] = (random_state[i + 128] >> 2) & 0x01;
    state[i][35] = (random_state[i + 128] >> 3) & 0x01;
    state[i][36] = (random_state[i + 128] >> 4) & 0x01;
    state[i][37] = (random_state[i + 128] >> 5) & 0x01;
    state[i][38] = (random_state[i + 128] >> 6) & 0x01;
    state[i][39] = (random_state[i + 128] >> 7) & 0x01;
    state[i][40] = (random_state[i + 128] >> 8) & 0x01;
    state[i][41] = (random_state[i + 128] >> 9) & 0x01;
    state[i][42] = (random_state[i + 128] >> 10) & 0x01;
    state[i][43] = (random_state[i + 128] >> 11) & 0x01;
    state[i][44] = (random_state[i + 128] >> 12) & 0x01;
    state[i][45] = (random_state[i + 128] >> 13) & 0x01;
    state[i][46] = (random_state[i + 128] >> 14) & 0x01;
    state[i][47] = (random_state[i + 128] >> 15) & 0x01;
    state[i][48] = (random_state[i + 128] >> 16) & 0x01;
    state[i][49] = (random_state[i + 128] >> 17) & 0x01;
    state[i][50] = (random_state[i + 128] >> 18) & 0x01;
    state[i][51] = (random_state[i + 128] >> 19) & 0x01;
    state[i][52] = (random_state[i + 128] >> 20) & 0x01;
    state[i][53] = (random_state[i + 128] >> 21) & 0x01;
    state[i][54] = (random_state[i + 128] >> 22) & 0x01;
    state[i][55] = (random_state[i + 128] >> 23) & 0x01;
    state[i][56] = (random_state[i + 128] >> 24) & 0x01;
    state[i][57] = (random_state[i + 128] >> 25) & 0x01;
    state[i][58] = (random_state[i + 128] >> 26) & 0x01;
    state[i][59] = (random_state[i + 128] >> 27) & 0x01;
    state[i][60] = (random_state[i + 128] >> 28) & 0x01;
    state[i][61] = (random_state[i + 128] >> 29) & 0x01;
    state[i][62] = (random_state[i + 128] >> 30) & 0x01;
    state[i][63] = (random_state[i + 128] >> 31) & 0x01;
  }
}

int main(void) {
  clock_setup();
  gpio_setup();
  spi_setup();

  randomize_state();
  randomize_state();

  // Let display power on
  us_delay(0xFFFF);
  us_delay(0xFFFF);
  us_delay(0xFFFF);
  us_delay(0xFFFF);

  init_display(true);

  arr_p *screen = &state;
  memcpy(future_state, state, sizeof(state));
  arr_p *future_screen = &future_state;

  while (1) {
    // gpio_toggle(GPIOC, GPIO13);
    us_delay(0xFFFF);

    draw_entire_display(screen);

    memcpy(state, future_state, sizeof(state));

    for (uint8_t i = 0; i < SCREEN_WIDTH; i++) {
      for (uint8_t j = 0; j < SCREEN_HEIGHT; j++) {
        uint8_t sum = 0;
        if (i == 0) {
          sum += (*screen)[i + 1][j + 0] & 0x01;
          if (j == 0) {
            sum += (*screen)[i + 1][j + 1] & 0x01;
            sum += (*screen)[i + 0][j + 1] & 0x01;
          } else if (j == SCREEN_HEIGHT - 1) {
            sum += (*screen)[i + 1][j - 1] & 0x01;
            sum += (*screen)[i + 0][j - 1] & 0x01;
          } else {
            sum += (*screen)[i + 0][j - 1] & 0x01;
            sum += (*screen)[i + 1][j - 1] & 0x01;
            sum += (*screen)[i + 1][j + 1] & 0x01;
            sum += (*screen)[i + 0][j + 1] & 0x01;
          }
        } else if (i == SCREEN_WIDTH - 1) {
          sum += (*screen)[i - 1][j + 0] & 0x01;
          if (j == 0) {
            sum += (*screen)[i - 1][j + 1] & 0x01;
            sum += (*screen)[i + 0][j + 1] & 0x01;
          } else if (j == SCREEN_HEIGHT - 1) {
            sum += (*screen)[i - 1][j - 1] & 0x01;
            sum += (*screen)[i + 0][j - 1] & 0x01;
          } else {
            sum += (*screen)[i + 0][j - 1] & 0x01;
            sum += (*screen)[i - 1][j - 1] & 0x01;
            sum += (*screen)[i - 1][j + 1] & 0x01;
            sum += (*screen)[i + 0][j + 1] & 0x01;
          }
        } else if (j == 0) {
          sum += (*screen)[i - 1][j + 0] & 0x01;
          sum += (*screen)[i - 1][j + 1] & 0x01;
          sum += (*screen)[i + 0][j + 1] & 0x01;
          sum += (*screen)[i + 1][j + 1] & 0x01;
          sum += (*screen)[i + 1][j + 0] & 0x01;
        } else if (j == SCREEN_HEIGHT - 1) {
          sum += (*screen)[i - 1][j + 0] & 0x01;
          sum += (*screen)[i - 1][j - 1] & 0x01;
          sum += (*screen)[i + 0][j - 1] & 0x01;
          sum += (*screen)[i + 1][j - 1] & 0x01;
          sum += (*screen)[i + 1][j + 0] & 0x01;
        } else {
          sum += (*screen)[i + 1][j + 1] & 0x01;
          sum += (*screen)[i + 1][j + 0] & 0x01;
          sum += (*screen)[i + 1][j - 1] & 0x01;
          sum += (*screen)[i - 1][j + 1] & 0x01;
          sum += (*screen)[i - 1][j + 0] & 0x01;
          sum += (*screen)[i - 1][j - 1] & 0x01;
          sum += (*screen)[i + 0][j + 1] & 0x01;
          sum += (*screen)[i + 0][j - 1] & 0x01;
        }

        if ((*screen)[i][j] && sum == 2) {
          (*future_screen)[i][j] = 1;
        } else if (sum == 3) {
          (*future_screen)[i][j] = 1;
        } else {
          (*future_screen)[i][j] = 0;
        }
      }
    }
  }

  return 0;
}