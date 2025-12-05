#include "input.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

uint8_t detected_addr = 0;
int player_input = 0; 
SemaphoreHandle_t xInputMutex;

void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
    GPIOA->MODER &= ~(3UL << (8 * 2));
    GPIOA->PUPDR &= ~(3UL << (8 * 2));
    GPIOA->PUPDR |=  (1UL << (8 * 2));
}

uint8_t button_read(void) {
    return (GPIOA->IDR & (1 << 8)) ? 1 : 0;
}


int mpu_init(uint8_t addr){
    return i2c_write_reg(addr, MPU_PWR_MGMT_1, 0x00);
}

int mpu_read_accel_g(uint8_t addr, float *ax, float *ay, float *az){
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

    if (detected_addr == 0xFF) return 0x0;
    
    if (mpu_init(detected_addr) != 0) return 0x0;

    return detected_addr;
}

int get_player_input(void) {
    if(!detected_addr) 
        detected_addr = mpu_connect();

    float r_ax, r_ay, r_az;
    int ay = 0;
    if (mpu_read_accel_g(detected_addr, &r_ax, &r_ay, &r_az) == 0)
        ay = (int)(r_ay*1000);
        

    if((abs(ay) >= 50)) return -ay/20;
    return 0; 
}

uint8_t is_button_pressed(void) {
    printf("button: %d\n", button_read());
    return button_read();
}
 
void vInputTask(void *arg) {
    (void)arg;
    xInputMutex = xSemaphoreCreateMutex();

    for (;;) {    
        int result = get_player_input();
        if (xSemaphoreTake(xInputMutex, portMAX_DELAY) == pdTRUE) {
            player_input = result; 
            xSemaphoreGive(xInputMutex);
        }
          
    }
    
}


