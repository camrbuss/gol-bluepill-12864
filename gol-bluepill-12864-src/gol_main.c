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
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_clock_enable(RCC_SPI1);

  // Enable AFIO for EXTI interrupts
  rcc_periph_clock_enable(RCC_AFIO);
}

// Using SPI1 Default Pins
// SCK - Master - Alternate function push-pull
// COPI - - Aliternate function push-pull
// PA5 SCK Alternate function 0
// PA7 COPI Alternate function 0

static void gpio_setup(void) {
  // Bluepill PC13 LED
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO13);
  // PB1 Button Switch, PB10 Enc A, PB11 Enc B, all input pull-up
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO1);
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO10);
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO11);
  gpio_set(GPIOB, GPIO1);
  gpio_set(GPIOB, GPIO10);
  gpio_set(GPIOB, GPIO11);

  // Enable PB1 External Interrupt
  nvic_enable_irq(NVIC_EXTI1_IRQ);
  exti_select_source(EXTI1, GPIOB);
  exti_set_trigger(EXTI1, EXTI_TRIGGER_FALLING);
  exti_enable_request(EXTI1);

  // Enable PB10 and PB11 External Interrupt
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);
  exti_select_source(EXTI10 | EXTI11, GPIOB);
  exti_set_trigger(EXTI10 | EXTI11, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI10 | EXTI11);

  // Set PA5 and PA7 as SCK and COPI respectively
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

static void timer_setup(void) {
  nvic_enable_irq(NVIC_TIM2_IRQ);
  rcc_periph_reset_pulse(RST_TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_disable_preload(TIM2);
  timer_continuous_mode(TIM2);
  // frequency = 72 MHz / 7200 = 10KHz
  timer_set_prescaler(TIM2, 7200);
  timer_set_period(TIM2, encoder_count);
  timer_enable_counter(TIM2);
  timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

void exti1_isr(void) {
  exti_reset_request(EXTI1);
  uint32_t flag = exti_get_flag_status(EXTI1);
  uint16_t pins_state = gpio_port_read(GPIOB);

  // If PB1 is 0, the button was pressed
  if (~pins_state & GPIO1) {
    button_pressed = true;
  }
}

void exti15_10_isr(void) {
  uint32_t flag10 = exti_get_flag_status(EXTI10);
  uint32_t flag11 = exti_get_flag_status(EXTI11);
  exti_reset_request(EXTI10 | EXTI11);
  uint16_t pins_state = gpio_port_read(GPIOB);
  bool pin10 = (pins_state & EXTI10) ? true : false;
  bool pin11 = (pins_state & EXTI11) ? true : false;

  // Pin 10 caused interrupt
  if (flag10) {
    if (pin10 != pin11) {
      encoder_count += ENCODER_INCREMENT_AMOUNT;
    } else {
      encoder_count -= ENCODER_INCREMENT_AMOUNT;
    }
  } else if (flag11) {
    if (pin10 == pin11) {
      encoder_count += ENCODER_INCREMENT_AMOUNT;
    } else {
      encoder_count -= ENCODER_INCREMENT_AMOUNT;
    }
  } else {
    // How did we end up here?
  }

  if (encoder_count > MAX_PLAY_RATE_MS * 10) {
    encoder_count = MAX_PLAY_RATE_MS * 10;
  }
  if (encoder_count < MIN_PLAY_RATE_MS * 10) {
    encoder_count = MIN_PLAY_RATE_MS * 10;
  }
}

void tim2_isr(void) {
  timer_clear_flag(TIM2, TIM_SR_CC1IF);
  play_game = true;
  timer_set_counter(TIM2, 0);
  timer_set_period(TIM2, encoder_count);
}

static void shift_randomn_state(uint8_t num) {
  for (uint8_t i = 0; i < num; i++) {
    // Update random_state with Marsaglia's Xorshift
    // https://en.wikipedia.org/wiki/Xorshift
    for (uint32_t j = 0; j < (SCREEN_HEIGHT * SCREEN_WIDTH / 32); j++) {
      uint32_t x = random_state[j];
      x ^= x << 13;
      x ^= x >> 7;
      x ^= x << 17;
      random_state[j] = x;
    }
  }
}

static void randomize_state(void) {
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
  timer_setup();

  shift_randomn_state(2);
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
    // Interrupt sets button_pressed
    if (button_pressed) {
      button_pressed = false;
      shift_randomn_state(timer_get_counter(TIM2) % 0xFF);
      randomize_state();
      memcpy(future_state, state, sizeof(state));
    }

    // TIM2 interrupt sets play game
    // Current implementation makes the edges dead
    if (play_game) {
      play_game = false;
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
  }

  return 0;
}