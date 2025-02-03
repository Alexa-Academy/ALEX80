#ifndef _BREAKOUT_H
#define _BREAKOUT_H

void init_game();
void draw_game();
void update_game(char input);
void updateBall();
void movePaddle(int direction);
void ball_start();
void paddle_start();

void ballLost();
void noBlocks();

#endif // _BREAKOUT_H