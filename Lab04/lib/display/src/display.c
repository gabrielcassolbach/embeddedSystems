#include "stm32f4xx.h"
#include "display.h"

void display_send_char(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void display_send_string(const char *s) {
    while (*s) display_send_char(*s++);
}

void display_send_command(const char *cmd) {
    display_send_string(cmd);
    display_send_char(0xFF);
    display_send_char(0xFF);
    display_send_char(0xFF);
}

void display_init(void) {
    // 1. Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;    // GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;   // USART2 clock

    
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2)));   // AF mode
    GPIOA->AFR[0] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4)));
    GPIOA->AFR[0] |=  ((7U << (2 * 4)) | (7U << (3 * 4)));  // AF7 = USART2

    // 3. Configure USART2
    USART2->CR1 = 0;   // reset control register 1
    USART2->CR2 = 0;   // reset control register 2
    USART2->CR3 = 0;   // no flow control

    
     /* BRR = pclk / baud (formato mantissa.frac4), com arredondamento */
    uint32_t div16 = (SystemCoreClock + (115200/2u)) / 115200; // arredonda
    uint32_t mant  = div16 / 16u;
    uint32_t frac  = div16 % 16u;

    USART2->BRR = (mant << 4) | (frac & 0xFu);

    // 6. 1 stop bit (default)
    USART2->CR2 &= ~(USART_CR2_STOP);

    // 7. Enable USART, transmitter and receiver
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
}
