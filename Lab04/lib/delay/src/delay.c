#include "stm32f4xx.h"
#include "delay.h"
#include "FreeRTOS.h"

/* contador de milissegundos (incrementado no SysTick) */
static volatile uint32_t g_ms = 0;

void delay_ms(uint32_t ms){
    vTaskDelay(pdMS_TO_TICKS(ms)); 
}
