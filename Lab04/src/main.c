#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "serial.h"
#include "st7789.h"
#include "board.h"
#include <stdint.h>
#include "i2c.h"
#include <stdio.h>
#include "game.h"

// LED pins
#define LED1_PIN   12
#define LED2_PIN   13
#define LED3_PIN   14

// Bit masks
#define LED1       (1U << LED1_PIN)
#define LED2       (1U << LED2_PIN)
#define LED3       (1U << LED3_PIN)

static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }
void delay_us(uint32_t us);

#define TRIG_PORT GPIOA
#define TRIG_PIN  2
#define ECHO_PORT GPIOA
#define ECHO_PIN  1
#define MPU6050_ADDR 0x68
#define MPU_ACCEL_XOUT_H 0x3B
#define MPU_PWR_MGMT_1    0x6B
#define MPU_WHO_AM_I      0x75

static inline uint32_t micros(void){
    return (uint32_t)((uint64_t)DWT->CYCCNT * 1000000ULL / SystemCoreClock);
}

static inline uint32_t pin_index(uint16_t pin_mask){
    return (uint32_t)__builtin_ctz((unsigned)pin_mask);
}

void hcsr04_init(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    volatile uint32_t tmp = RCC->AHB1ENR; (void)tmp;

    uint32_t t = pin_index(TRIG_PIN);
    uint32_t e = pin_index(ECHO_PIN);

    GPIOA->MODER &= ~((3U << (2*t)) | (3U << (2*e)));
    GPIOA->MODER |=  (1U << (2*t)); 
    GPIOA->OTYPER &= ~(1U << t);

    GPIOA->OSPEEDR &= ~(3U << (2*t));
    GPIOA->OSPEEDR |=  (1U << (2*t));

    GPIOA->PUPDR &= ~(3U << (2*t));
    GPIOA->PUPDR &= ~(3U << (2*e));
    GPIOA->PUPDR |=  (2U << (2*e)); 

    TRIG_PORT->BSRR = (uint32_t)TRIG_PIN << 16; 
}
  
float hcsr04_read_cm(void){
    const uint32_t timeout_us = 30000U; 

    TRIG_PORT->BSRR = (uint32_t)TRIG_PIN << 16; 
    delay_us(2);
    TRIG_PORT->BSRR = TRIG_PIN;                 
    delay_us(10);
    TRIG_PORT->BSRR = (uint32_t)TRIG_PIN << 16; 

    uint32_t t_start = micros();
    uint32_t deadline = t_start + timeout_us;
    while ((ECHO_PORT->IDR & ECHO_PIN) == 0U) {
        if (micros() > deadline) return -1.0f; 
    }

    uint32_t t_rise = micros();
    deadline = t_rise + timeout_us;
    while ((ECHO_PORT->IDR & ECHO_PIN) != 0U) {
        if (micros() > deadline) return -1.0f;  
    }
    uint32_t t_fall = micros();

    uint32_t pulse_us = t_fall - t_rise;

    float distance_cm = (float)pulse_us * 0.0343f / 2.0f;
    return distance_cm;
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

static int mpu_init(uint8_t addr){
    return i2c_write_reg(addr, MPU_PWR_MGMT_1, 0x00);
}

static int mpu_read_accel_g(uint8_t addr, float *ax, float *ay, float *az){
    uint8_t buf[6];
    int r = i2c_read_regs(addr, MPU_ACCEL_XOUT_H, buf, 6);
    if (r != 0) return r;
    int16_t rx = (int16_t)( (buf[0] << 8) | buf[1] );
    int16_t ry = (int16_t)( (buf[2] << 8) | buf[3] );
    int16_t rz = (int16_t)( (buf[4] << 8) | buf[5] );
    const float scale = 16384.0f;
    *ax = (float)rx / scale;
    *ay = (float)ry / scale;
    *az = (float)rz / scale;
    return 0;
}

uint8_t mpu_connect(void){
    uint8_t detected_addr = 0xFF;
    for (uint8_t a = 0x68; a <= 0x69; ++a){
        uint8_t who = 0;
        if (i2c_read_regs(a, MPU_WHO_AM_I, &who, 1) == 0){
            if (who == 0x68){
                detected_addr = a;
                break;
            }
        }
    }
    if (detected_addr == 0xFF){
        //printf("MPU6050 nao detectado em 0x68/0x69\n");
        return 0x0;
    }
    //printf("MPU detectado em 0x%02X\n", detected_addr);

    if (mpu_init(detected_addr) != 0){
        //printf("Falha ao inicializar MPU\n");
        return 0x0;
    }

    return detected_addr;
}


void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (8 * 2));
    GPIOA->PUPDR &= ~(3UL << (8 * 2));
}

void leds_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~((3U << (LED1_PIN * 2)) |
                      (3U << (LED2_PIN * 2)) |
                      (3U << (LED3_PIN * 2)));

    GPIOB->MODER |=  ((1U << (LED1_PIN * 2)) |
                      (1U << (LED2_PIN * 2)) |
                      (1U << (LED3_PIN * 2)));
    GPIOB->OTYPER &= ~(LED1 | LED2 | LED3);
    GPIOB->OSPEEDR &= ~((3U << (LED1_PIN * 2)) |
                        (3U << (LED2_PIN * 2)) |
                        (3U << (LED3_PIN * 2)));
    GPIOB->BSRR = ((LED1 | LED2 | LED3) << 16);
}

void led_on(uint8_t led_num) {
    switch (led_num) {
        case 1: GPIOB->BSRR = LED1; break;
        case 2: GPIOB->BSRR = LED2; break;
        case 3: GPIOB->BSRR = LED3; break;
        default: break;
    }
}

void led_off(uint8_t led_num) {
    switch (led_num) {
        case 1: GPIOB->BSRR = (LED1 << 16); break;
        case 2: GPIOB->BSRR = (LED2 << 16); break;
        case 3: GPIOB->BSRR = (LED3 << 16); break;
        default: break;
    }
}

uint8_t button_read(void) {
    return (GPIOA->IDR & (1 << 8)) ? 1 : 0;
}

void hc12_init(void) {
    // 1. Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;    // GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;   // USART2 clock

    // 2. Configure PA2 (TX) and PA3 (RX) as alternate function
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2)));   // AF mode
    GPIOA->AFR[0] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4)));
    GPIOA->AFR[0] |=  ((7U << (2 * 4)) | (7U << (3 * 4)));  // AF7 = USART2

    // 3. Configure USART2
    USART2->CR1 = 0;   // reset control register 1
    USART2->CR2 = 0;   // reset control register 2
    USART2->CR3 = 0;   // no flow control

    
     /* BRR = pclk / baud (formato mantissa.frac4), com arredondamento */
    uint32_t div16 = (SystemCoreClock + (9600/2u)) / 9600; // arredonda
    uint32_t mant  = div16 / 16u;
    uint32_t frac  = div16 % 16u;

    USART2->BRR = (mant << 4) | (frac & 0xFu);

    // 6. 1 stop bit (default)
    USART2->CR2 &= ~(USART_CR2_STOP);

    // 7. Enable USART, transmitter and receiver
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
}


void hc12_send_char(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void hc12_send_string(const char *s) {
    while (*s) hc12_send_char(*s++);
}

void detect_event(int ax, int ay, int car_dist, int *events, int *last_event){
    if(micros() - (*last_event) < 1000000){
        return; 
    }

    if(car_dist < 500 && (int) ay <= -900){
        led_off(2); led_off(3);
        led_on(1);
        (*events)++;
        (*last_event) = micros();
        erase_time((*events) - 1);
        draw_time((*events), C_GREEN);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "batida\r\n");
        hc12_send_string(buffer);
    }
    
    else if((int) ay <= -700){
        led_off(1); led_off(2);
        led_on(3);
        (*events)++;
        (*last_event) = micros();
        erase_time((*events) - 1);
        draw_time((*events), C_GREEN);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "freada brusca\r\n");
        hc12_send_string(buffer);
    }


    else if(abs(ax) >= 700){
        led_off(1); led_off(3);
        led_on(2);
        (*events)++;
        (*last_event) = micros();
        erase_time((*events) - 1);
        draw_time((*events), C_GREEN);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "curva acentuada\r\n");
        hc12_send_string(buffer);
    }
  
}


int main(void){
    delay_init();
    serial_stdio_init(115200);   
    delay_us_init();
    hcsr04_init();
    st7789_init();
    i2c_gpio_init();
    button_init();
    leds_init();
    hc12_init();

    run();
    return 0;
}


