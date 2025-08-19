#include <stdint.h>
#include "stm32f4xx.h"   // CMSIS device header (selects stm32f411xe.h when STM32F411xE is defined)

static void delay(volatile uint32_t d) {
    while (d--) { __asm volatile ("nop"); }
}

int main(void) {
    /* 1) Enable clock for GPIOC (PC13 = on-board LED on many BlackPills) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    /* Dummy read to ensure clock is enabled (recommended) */
    volatile uint32_t tmp = RCC->AHB1ENR;
    (void)tmp;

    /* 2) Configure PC13 as General purpose output (MODER = 01) */
    /* Clear mode bits for pin 13 then set output (01) */
    GPIOC->MODER &= ~(0x3U << (13 * 2));
    GPIOC->MODER |=  (0x1U << (13 * 2));

    /* Optional: push-pull (OTYPER = 0), low speed, no pull-up/pull-down */
    GPIOC->OTYPER &= ~(1U << 13);
    GPIOC->OSPEEDR &= ~(0x3U << (13 * 2));
    GPIOC->PUPDR &= ~(0x3U << (13 * 2));

    /* Initialize LED state: off (remember many boards have LED tied to VCC, active low) */
    GPIOC->ODR |= (1U << 13); // write '1' → LED off if active-low

    while (1) {
        /* Toggle pin */
        GPIOC->ODR ^= (1U << 13);

        /* crude delay ~ adjustable */
        delay(2000000);
    }

    /* never reached */
    // return 0;
}
