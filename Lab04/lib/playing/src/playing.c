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

    if(ctx->dino_x > PATH_WIDTH)
        ctx->dino_x = PATH_WIDTH;
    else if(ctx->dino_x < 0)
        ctx->dino_x = 0;


    for(int i = 0; i < TOTAL_OBSTACLES; i++){
        if(ctx->obstacles[i].active){
            ctx->obstacles[i].y -= OBSTACLE_SPEED;
            
            if(ctx->obstacles[i].y <= 0){
                if(!(ctx->obstacles[i].x+ctx->obstacles[i].width/2 < ctx->dino_x - DINO_WIDTH/2) && !(ctx->obstacles[i].x-ctx->obstacles[i].width/2 > ctx->dino_x + DINO_WIDTH/2)){
                    ctx->current_state = GAME_STATE_GAME_OVER;
                }
                ctx->obstacles[i].active = 0;
            }

        }
    }
}

void render_playing(GameContext *ctx){
    
}
