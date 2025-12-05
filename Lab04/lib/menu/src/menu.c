#include <stdio.h>
#include "menu.h"
#include "input.h"

void update_menu(GameContext *ctx){
    if(is_button_pressed()){
        ctx->current_state = GAME_STATE_PLAYING;
        display_send_command("page play");
    }
    ctx->score = 0;
}

void render_menu(GameContext *ctx){

}