#include "stm32f4xx.h"
#include <stdio.h>
#include "i2c.h"

/* --- pinos I2C por software (ajuste se quiser outros) --- */
#define I2C_PORT GPIOB
#define I2C_SCL_PIN (1U << 6)   /* PB6 */
#define I2C_SDA_PIN (1U << 7)   /* PB7 */

/* MPU6050 I2C address (AD0 = 0 -> 0x68) */
#define MPU6050_ADDR 0x68

/* MPU-6050 registers */
#define MPU_ACCEL_XOUT_H 0x3B
#define MPU_PWR_MGMT_1    0x6B
#define MPU_WHO_AM_I      0x75

void delay_us_i2c(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < cycles);
}


/* pequeno pulso para gerar clock I2C - ajuste para ~100kHz */
void i2c_delay_us(void){ delay_us_i2c(4); }

/* configura pinos PB6/PB7 como open-drain (entrada por leitura) */
void i2c_gpio_init(void){
    /* habilita clock GPIOB */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;

    uint32_t s = __builtin_ctz(I2C_SCL_PIN);
    uint32_t d = __builtin_ctz(I2C_SDA_PIN);

    /* MODER: set both as general purpose output (01) temporariamente */
    I2C_PORT->MODER &= ~((3U << (2*s)) | (3U << (2*d)));
    I2C_PORT->MODER |=  ((1U << (2*s)) | (1U << (2*d)));

    /* OTYPER: open-drain (1) */
    I2C_PORT->OTYPER |= (1U << s) | (1U << d);

    /* OSPEEDR: medium */
    I2C_PORT->OSPEEDR &= ~((3U << (2*s)) | (3U << (2*d)));
    I2C_PORT->OSPEEDR |=  ((1U << (2*s)) | (1U << (2*d)));

    /* PUPDR: none (external pull-ups preferíveis) */
    I2C_PORT->PUPDR &= ~((3U << (2*s)) | (3U << (2*d)));

    /* garante linhas em high (libera) - como open-drain, colocar 1 = high (pull-up) */
    I2C_PORT->BSRR = I2C_SCL_PIN | I2C_SDA_PIN;
    i2c_delay_us();
}

/* configura SDA como input (libera linha) */
void sda_release(void){
    uint32_t d = __builtin_ctz(I2C_SDA_PIN);
    I2C_PORT->MODER &= ~(3U << (2*d)); /* 00 = input */
}

/* configura SDA como output (dirigir a linha) */
void sda_drive(void){
    uint32_t d = __builtin_ctz(I2C_SDA_PIN);
    I2C_PORT->MODER &= ~(3U << (2*d));
    I2C_PORT->MODER |=  (1U << (2*d)); /* 01 = output */
}

int sda_read(void){
    return ( (I2C_PORT->IDR & I2C_SDA_PIN) != 0 );
}

void scl_set(void){ I2C_PORT->BSRR = I2C_SCL_PIN; }
void scl_clr(void){ I2C_PORT->BSRR = (uint32_t)I2C_SCL_PIN << 16; }
void sda_set(void){ I2C_PORT->BSRR = I2C_SDA_PIN; }
void sda_clr(void){ I2C_PORT->BSRR = (uint32_t)I2C_SDA_PIN << 16; }

void i2c_start(void){
    sda_release(); /* high (float) */
    scl_set();
    i2c_delay_us();
    sda_drive();
    sda_clr();     /* SDA low while SCL high */
    i2c_delay_us();
    scl_clr();
}

void i2c_stop(void){
    sda_drive();
    sda_clr();
    i2c_delay_us();
    scl_set();
    i2c_delay_us();
    sda_release(); /* SDA goes high -> stop */
    i2c_delay_us();
}

int i2c_write_byte(uint8_t b){
    sda_drive();
    for (int i = 7; i >= 0; --i){
        if (b & (1U << i)) sda_set(); else sda_clr();
        i2c_delay_us();
        scl_set();
        i2c_delay_us();
        scl_clr();
    }
    /* ACK bit: libera SDA e lê */
    sda_release();
    i2c_delay_us();
    scl_set();
    i2c_delay_us();
    int nack = sda_read();
    scl_clr();
    sda_drive();
    return nack; /* 0 = ACK, 1 = NACK */
}

uint8_t i2c_read_byte(int ack){
    uint8_t b = 0;
    sda_release(); /* SDA input */
    for (int i = 7; i >= 0; --i){
        scl_set();
        i2c_delay_us();
        if (sda_read()) b |= (1U << i);
        scl_clr();
        i2c_delay_us();
    }
    /* enviar ACK/NACK */
    sda_drive();
    if (ack) sda_clr(); else sda_set(); /* ACK = 0 on bus */
    i2c_delay_us();
    scl_set();
    i2c_delay_us();
    scl_clr();
    sda_release();
    return b;
}

int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value){
    i2c_start();
    if (i2c_write_byte((dev_addr << 1) | 0x0)) { i2c_stop(); return -1; } /* NACK */
    if (i2c_write_byte(reg)) { i2c_stop(); return -2; }
    if (i2c_write_byte(value)) { i2c_stop(); return -3; }
    i2c_stop();
    return 0;
}

int i2c_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len){
    i2c_start();
    if (i2c_write_byte((dev_addr << 1) | 0x0)) { i2c_stop(); return -1; } /* write addr NACK */
    if (i2c_write_byte(reg)) { i2c_stop(); return -2; }
    /* repeated start for read */
    i2c_start();
    if (i2c_write_byte((dev_addr << 1) | 0x1)) { i2c_stop(); return -3; } /* read addr */
    for (uint8_t i = 0; i < len; ++i){
        buf[i] = i2c_read_byte(i < (len-1)); /* ACK for all but last */
    }
    i2c_stop();
    return 0;
}
