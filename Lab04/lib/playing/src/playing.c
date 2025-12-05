#include <stdio.h>
#include "game_objects.h"
#include "input.h"
#include <stdlib.h>


int random_between(int min, int max) {
    return min + rand() % (max - min + 1);
}

void spawn_obstacle(GameContext *ctx){
    for(int i = 0; i < TOTAL_OBSTACLES; i++){
        if(!ctx->obstacles[i].active){
            ctx->obstacles[i].active = 1;
            ctx->obstacles[i].type = random_between(0,2);
            ctx->obstacles[i].width = WIDTHS[ctx->obstacles[i].type];
            ctx->obstacles[i].x = random_between(ctx->obstacles[i].width/2, PATH_WIDTH-ctx->obstacles[i].width/2);
            ctx->obstacles[i].y = PATH_TOP;   
            return;
        }
    }
}

void update_playing(GameContext *ctx){
    (ctx -> score)++;

    if(ctx->frame_counter%SPAWN_PERIOD == 0){
        spawn_obstacle(ctx);
    }

    ctx->dino_x += get_player_input();

    if(ctx->dino_x > PATH_WIDTH-DINO_WIDTH) ctx->dino_x = PATH_WIDTH-DINO_WIDTH;
    
    else if(ctx->dino_x < 0) ctx->dino_x = 0;
    

    for(int i = 0; i < TOTAL_OBSTACLES; i++){
        if(ctx->obstacles[i].active){
            ctx->obstacles[i].prev_y = ctx->obstacles[i].y;
            ctx->obstacles[i].y -= OBSTACLE_SPEED;
            
            if(ctx->obstacles[i].y <= DINO_WIDTH)
                if((ctx->obstacles[i].x+ctx->obstacles[i].width >= ctx->dino_x) && (ctx->obstacles[i].x <= ctx->dino_x + DINO_WIDTH)){
                    ctx->current_state = GAME_STATE_GAME_OVER;
                    display_send_command("page game_over");
                    printf("CORIIASFJPASIFJPOASIFJAPSOIF");
                }

            if(ctx->obstacles[i].y <= 0){
                
                printf("obsx %d\n obsw%d dinox %d\n dinow %d\n ", ctx->obstacles[i].x, ctx->obstacles[i].width, ctx->dino_x, DINO_WIDTH);
                ctx->obstacles[i].active = 0;
            }
        }
    }
}

void draw_obstacle(int x, int y, int obstacle_w) {
    char cmd[64];
    // x, y = 30, width = 80, height = 40, color = 31 (blue)
    sprintf(cmd, "fill %d,%d,%d,80,31", x, y, obstacle_w);
    display_send_command(cmd);
}

void clear_obstacle(int x, int y, int obstacle_w) {
    char cmd[64];
    sprintf(cmd, "fill %d,%d,%d,80,65535", x, y, obstacle_w); // White (background)
    display_send_command(cmd);
}

void draw_dino(int x) {
    char cmd[64];
    // x, y = 30, width = 80, height = 40, color = 31 (blue)
    sprintf(cmd, "fill %d,%d,%d,%d,31", x, 0, DINO_WIDTH, DINO_WIDTH);
    display_send_command(cmd);
}

void clear_dino(int x) {
    char cmd[64];
    sprintf(cmd, "fill %d,%d,%d,%d,65535", x, 0, DINO_WIDTH, DINO_WIDTH); // White (background)
    display_send_command(cmd);
}

void draw_clock(int min, int sec){
    char cmd[64];
    sprintf(cmd, "t0.txt=\"%2d:%2d\"", min, sec);
    display_send_command(cmd);
}

void render_playing(GameContext *ctx){
    
    
    for(int i = 0; i < TOTAL_OBSTACLES; i++){
        int ow, ox, oy, prev_oy;

        ox = ctx->obstacles[i].x; 
        oy = ctx->obstacles[i].y;
        prev_oy = ctx->obstacles[i].prev_y;
        ow = ctx->obstacles[i].width;

        clear_obstacle(ox, prev_oy, ow);
        if(ctx->obstacles[i].active){   
            draw_obstacle(ox, oy, ow);
        }
        
    }

    int min, sec;

    sec = ctx->frame_counter/FRAMERATE;
    min = sec/60;
    sec = sec%60;



    clear_dino(ctx->lastdraw_dino_x);
    draw_dino(ctx->dino_x);
    ctx->lastdraw_dino_x = ctx->dino_x;
}
