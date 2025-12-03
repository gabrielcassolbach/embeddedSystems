#pragma once

int is_button_pressed(void);
int get_player_input(void);

#define MPU6050_ADDR 0x68
#define MPU_ACCEL_XOUT_H 0x3B
#define MPU_PWR_MGMT_1    0x6B
#define MPU_WHO_AM_I      0x75