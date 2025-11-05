#pragma once
#include <stdint.h>

void i2c_gpio_init(void);
void i2c_start(void);
void i2c_stop(void);
int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value);
int i2c_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);