#include <stdio.h>
#include "game_over.h"
#include "input.h"

void update_game_over(GameContext *ctx){
    if(is_button_pressed()){
        ctx->current_state = GAME_STATE_MENU;
        display_send_command("page menu");
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