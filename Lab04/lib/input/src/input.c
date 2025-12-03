#include "input.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"

uint8_t detected_addr = 0;


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

int get_player_input(void) {
    if(!detected_addr) 
        detected_addr = mpu_connect();

    float r_ax, r_ay, r_az;
    int ay = 0;
    if (mpu_read_accel_g(detected_addr, &r_ax, &r_ay, &r_az) == 0)
        ay = (int)(r_ay*1000);
        

    if((abs(ay) >= 50)) return ay/20;
    return 0; 
}

int is_button_pressed(void) {
    return 0;
}

unsigned char get_button_pressed(void) {
     
}


