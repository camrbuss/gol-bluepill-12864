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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void)
{
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

int main(void)
{
    clock_setup();
    gpio_setup();

    while (1)
    {
        gpio_toggle(GPIOC, GPIO13);
        for (uint32_t i = 0; i < 0xFFFFFF; i++)
            __asm__("nop");
    }

    return 0;
}