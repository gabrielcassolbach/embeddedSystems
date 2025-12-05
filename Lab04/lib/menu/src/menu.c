#include <stdio.h>
#include "menu.h"
#include "input.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t xTimeMutex;
int time_sec;

void update_menu(GameContext *ctx){
   
    if(is_button_pressed()){
        ctx->current_state = GAME_STATE_PLAYING;

        if (xSemaphoreTake(xTimeMutex, portMAX_DELAY) == pdTRUE) {
            time_sec = 0;
            xSemaphoreGive(xTimeMutex);
        }

        display_send_command("page play");
    }
    printf("updating menu....\n");
    
    ctx->score = 0;
}

void render_menu(GameContext *ctx){

}