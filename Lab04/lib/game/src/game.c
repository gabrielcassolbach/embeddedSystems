#include "menu.h" 
#include "input.h"
#include "playing.h"
#include "game_over.h"
#include "stm32f4xx.h"

void sleep_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000UL);
    while ((DWT->CYCCNT - start) < cycles);
}

void render(GameContext *ctx) {
    switch (ctx->current_state){
            
        case GAME_STATE_MENU:
            render_menu(ctx);
            break;

        case GAME_STATE_PLAYING: 
            render_playing(ctx);
            break;

        case GAME_STATE_GAME_OVER: 
            render_game_over(ctx);
            break;

        default:
            break;
    }

}

void update(GameContext *ctx)  {

    switch (ctx->current_state){
            
        case GAME_STATE_MENU:
            update_menu(ctx);
            break;

        case GAME_STATE_PLAYING:
            update_playing(ctx);
            break;

        case GAME_STATE_GAME_OVER: 
            update_game_over(ctx);
            break;
    
        default:
            break;
    }
    ctx->frame_counter++;
}

void run(){
    GameContext ctx;
    ctx.current_state = GAME_STATE_PLAYING;

    for(int i = 0; i < TOTAL_OBSTACLES; i++){
        ctx.obstacles[i].active = 0;
        ctx.obstacles[i].type = 0;
        ctx.obstacles[i].width = 0;
        ctx.obstacles[i].x = 0;
        ctx.obstacles[i].y = 0;
    }

    ctx.frame_counter = 0;

    ctx.dino_x = PATH_WIDTH/2 - DINO_WIDTH/2;
    ctx.lastdraw_dino_x = 0;

    while(1){
        render(&ctx);
        update(&ctx);
        sleep_us((1000000)/FRAMERATE);
    }
}
