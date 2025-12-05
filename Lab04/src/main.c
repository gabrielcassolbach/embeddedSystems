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
#include "input.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }
void delay_us(uint32_t us);

extern SemaphoreHandle_t xTimeMutex;
extern int time_sec;

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

void clear_screen(void) {
    display_send_command("cls 65535");   // White background
}

void vTimeTask(void *arg) {
    (void)arg;
    xTimeMutex = xSemaphoreCreateMutex();
    for (;;) {
        if (xSemaphoreTake(xTimeMutex, portMAX_DELAY) == pdTRUE) {
            time_sec += 1;
            xSemaphoreGive(xTimeMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void){
    serial_stdio_init(115200);
    delay_us_init();   
    i2c_gpio_init();
 
    button_init();    
    display_init();

    printf("starting threads\n");

    // Define THREAD do input - Leitura do Acelerômetro.
    xTaskCreate(vInputTask, "input", 256, NULL, 1, NULL);
    xTaskCreate(run, "main_thread", 256, NULL, 1, NULL);
    xTaskCreate(vTimeTask, "time", 256, NULL, 1, NULL);
    
    printf("scheduling tasks\n");
    vTaskStartScheduler();

    while (1) { __NOP(); }
    return 0;
}


