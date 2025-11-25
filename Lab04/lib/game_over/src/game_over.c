#include <stdio.h>
#include "game_over.h"
#include "input.h"

void update_game_over(GameContext *ctx){
    if(is_button_pressed())
        ctx->current_state = GAME_STATE_MENU;
}

void render_game_over(GameContext *ctx){

}