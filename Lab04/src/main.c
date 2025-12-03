#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "serial.h"
#include "st7789.h"
#include "board.h"
#include <stdint.h>
#include <stdio.h>
#include "game.h"
#include "display.h"

static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }
void delay_us(uint32_t us);

#define TRIG_PORT GPIOA
#define TRIG_PIN  2
#define ECHO_PORT GPIOA
#define ECHO_PIN  1


static inline uint32_t micros(void){
    return (uint32_t)((uint64_t)DWT->CYCCNT * 1000000ULL / SystemCoreClock);
}

static inline uint32_t pin_index(uint16_t pin_mask){
    return (uint32_t)__builtin_ctz((unsigned)pin_mask);
}

void delay_us_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;                                
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            
}

void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < cycles);
}


static void draw_time(int number, int c){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", number);
    st7789_draw_text_5x7(100, 100, buffer, c, 8, 0, 0);
}

static void erase_time(int number){
    draw_time(number, C_BLACK);
}

void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (8 * 2));
    GPIOA->PUPDR &= ~(3UL << (8 * 2));
}

uint8_t button_read(void) {
    return (GPIOA->IDR & (1 << 8)) ? 1 : 0;
}

void clear_screen(void) {
    display_send_command("cls 65535");   // White background
}

int main(void){
    delay_init();
    serial_stdio_init(115200);   
    delay_us_init();
    i2c_gpio_init();
    button_init();    

    display_init();
    delay_ms(1000);
    clear_screen();

    run();
    
    return 0;
}


