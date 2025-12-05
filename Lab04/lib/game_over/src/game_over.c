#include <stdio.h>
#include "game_over.h"
#include "input.h"
#include "FreeRTOS.h"

void update_game_over(GameContext *ctx){
    if(is_button_pressed()){
        for(int i = 0; i < TOTAL_OBSTACLES; i++)
            ctx->obstacles[i].active = 0;   
            
        
        
        ctx->current_state = GAME_STATE_MENU;
        
        display_send_command("page menu");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void set_score(int score){
    char cmd[64];
    sprintf(cmd, "t2.txt=\"%d\"", score);
    display_send_command(cmd);
}

void render_game_over(GameContext *ctx){
 
    set_score(ctx->score);
}