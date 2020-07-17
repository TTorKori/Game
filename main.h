#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#ifndef MAIN_H
#define MAIN_H

#define NUM_STARS 10

enum STATUS {
	LIVES, GAME, GAMEOVER
};

typedef struct {
	float x, y;
	float dx, dy;
	char* name;
	short lives;
	int onLedge, isDead;
	int animFrame, facingLeft;
} Man;

typedef struct {
	int x, y;
} Lean;

typedef struct {
	int x, y, w, h;
} Ledge;

typedef struct {
	
	float scrollX;

	Man man;
	SDL_Texture* manFrames[2];
	SDL_Texture* ground;
	SDL_Texture* lean;
	SDL_Texture* label;
	SDL_Texture* gameOverLabel;

	int time, deathCountDown;
	int statusState;
	int musicChannel;
	
	TTF_Font* font;
	Ledge ledges[100];
	Lean leans[NUM_STARS];
	Mix_Chunk* bgMusic;

	SDL_Renderer* renderer;
} GameState;

void renderRect(GameState* game);


#endif