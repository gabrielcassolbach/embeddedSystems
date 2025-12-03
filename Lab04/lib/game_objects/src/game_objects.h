#pragma once

#define TOTAL_OBSTACLES 5
#define OBSTACLE_SPEED 2

#define PATH_WIDTH 480
#define PATH_TOP 320

#define DINO_WIDTH 40

#define FRAMERATE 60

#define SPAWN_PERIOD FRAMERATE/1 

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER,
} GameState;

typedef enum {
    FOSSIL,
    ROCK, 
    TREE,
    N_OBSTACLES
} ObstacleType;

static int WIDTHS[N_OBSTACLES] = {50, 20, 10};

typedef struct {
    int x, y, width; 
    int prev_y;
    int active; 
    ObstacleType type; 
} Obstacle;

typedef struct {
    GameState current_state;
    Obstacle obstacles[TOTAL_OBSTACLES];
    int dino_x;
    int lastdraw_dino_x; 
    int frame_counter; 
    int score;
} GameContext; 