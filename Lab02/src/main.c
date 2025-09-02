#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "serial.h"
#include "st7789.h"
#include "board.h"

static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }

static void demo_clear(void){
    printf("[demo 1] Clear screen (black)\r\n");
    st7789_fill_screen_dma(C_BLACK);
    st7789_draw_text_5x7(20, 20, "CLEAR BLACK", C_WHITE, 2, 0, 0);
}

static void demo_pixels(void){
    printf("[demo 2] Random pixels\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<2000;i++){
        int x = rand() % LCD_W;
        int y = rand() % LCD_H;
        uint16_t c = (uint16_t)rand();
        st7789_draw_pixel(x,y,c);
    }
}

static void demo_hlines(void){
    printf("[demo 3] Horizontal lines\r\n");
    for (int y=0;y<LCD_H;y+=10){
        uint16_t c = (uint16_t)rand();
        st7789_draw_hline(0,y,LCD_W,c);
    }
}

static void demo_vlines(void){
    printf("[demo 4] Vertical lines\r\n");
    for (int x=0;x<LCD_W;x+=10){
        uint16_t c = (uint16_t)rand();
        st7789_draw_vline(x,0,LCD_H,c);
    }
}

static void demo_rects(void){
    printf("[demo 5] Random rectangles (outline)\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<10;i++){
        int x = rand()%200;
        int y = rand()%200;
        int w = 20+rand()%40;
        int h = 20+rand()%40;
        uint16_t c = (uint16_t)rand();
        st7789_draw_rect(x,y,w,h,c);
    }
}

static void demo_fills(void){
    printf("[demo 6] Filled rectangles (DMA)\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<5;i++){
        int x = rand()%200;
        int y = rand()%200;
        int w = 20+rand()%40;
        int h = 20+rand()%40;
        uint16_t c = (uint16_t)rand();
        printf("  rect #%d: x=%d y=%d w=%d h=%d color=0x%04X\r\n",
               i, x,y,w,h,c);
        st7789_fill_rect_dma(x,y,w,h,c);
        delay_ms(200);
    }
}

static void demo_circles(void){
    printf("[demo 7] Random circles\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<8;i++){
        int x = rand()%LCD_W;
        int y = rand()%LCD_H;
        int r = 10+rand()%30;
        uint16_t c = (uint16_t)rand();
        printf("  circle #%d: cx=%d cy=%d r=%d color=0x%04X\r\n",
               i, x,y,r,c);
        st7789_draw_circle(x,y,r,c);
    }
}

static void demo_fillcircles(void){
    printf("[demo 8] Filled circles\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<6;i++){
        int x = rand()%LCD_W;
        int y = rand()%LCD_H;
        int r = 10+rand()%40;
        uint16_t c = (uint16_t)rand();
        printf("  fillcirc #%d: cx=%d cy=%d r=%d color=0x%04X\r\n",
               i, x,y,r,c);
        st7789_fill_circle(x,y,r,c);
    }
}

static void demo_text(void){
    printf("[demo 9] Text demo\r\n");
    st7789_fill_screen(C_BLACK);
    st7789_draw_text_5x7(10, 40, "UTFPR - SIS.EMBARCADOS", C_YELL, 2, 0, 0);
    st7789_draw_text_5x7(10, 80, "MENU DEMO", C_CYAN, 2, 0, 0);
}

static void draw_ball(int x, int c){
    st7789_fill_circle(x, LCD_H/2, 30, c);
}

static void draw_box(int c){
    st7789_fill_rect_dma(LCD_W-50,10,40,40,c);
}

static void draw_time(int number, int c){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "uptime: %d s", number);
    st7789_draw_text_5x7(10, 10, buffer, c, 2, 0, 0);
}

static void erase_ball(int x){
    draw_ball(x, C_BLACK);
}

static void erase_time(int number){
    draw_time(number, C_BLACK);
}

int main(void){
    delay_init();
    serial_stdio_init(115200);  
    
    st7789_init();
    st7789_set_speed_div(0);

    st7789_fill_screen(C_BLACK);
    int ball_x = 30, ball_vel = 4, ball_r = 30;
    int time_counter = 0; 

    for(;;){
        if (uart_rx_ready()){
            char c = uart_getc();
            switch(c){
                case '+': ball_vel += (ball_vel < 0 ? -1 : 1); break;
                case '-': ball_vel += (ball_vel > 0 ? -1 : 1); break;
                default: break;
            }
            printf("velocity: %d\n", ball_vel);
        }

        ((time_counter/20)%2 == 0) ? draw_box(C_RED) : draw_box(C_GREEN);

        ball_x += ball_vel;
        if((ball_vel > 0 && ball_x >= LCD_W-30) || (ball_x <= 30 && (ball_vel < 0)))
           ball_vel = (-1)*ball_vel;
        
        draw_time(time_counter/20, C_GREEN);
        draw_ball(ball_x, C_YELL); 
        
        delay_ms(50);
        
        erase_ball(ball_x);
        erase_time(time_counter/20);
        
        time_counter++;

    }
}
