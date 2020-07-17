#include <stdio.h>
#include "main.h"

void init_status_lives(GameState* gamestate) {
	
	char str[128] = "";
	sprintf(str, "LIVES LEFT: %d", (int)gamestate->man.lives);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Surface* tmp = TTF_RenderText_Blended(gamestate->font, str, white);
	gamestate->label = SDL_CreateTextureFromSurface(gamestate->renderer, tmp);
	SDL_FreeSurface(tmp);
}
void draw_status_lives(GameState* gamestate) {
	SDL_Rect manRect;
	manRect.x = 320;
	manRect.y = 240;
	SDL_QueryTexture(gamestate->manFrames[0], NULL, NULL, &manRect.w, &manRect.h);

	SDL_SetRenderDrawColor(gamestate->renderer, 255, 255, 0, 0);
	SDL_RenderClear(gamestate->renderer);

	SDL_RenderCopyEx(gamestate->renderer, gamestate->manFrames[0], 
		NULL, &manRect, 0, NULL, gamestate->man.facingLeft == 0);

	SDL_SetRenderDrawColor(gamestate->renderer, 255, 255, 255, 255);

	SDL_Rect textRect;
	SDL_QueryTexture(gamestate->label, NULL, NULL, &textRect.w, &textRect.h);
	textRect.x = 320 - textRect.w/2;
	textRect.y = 240 - 50;

	SDL_RenderCopy(gamestate->renderer, gamestate->label, NULL, &textRect);

}
void shutdown_status_lives(GameState* gamestate) {
	SDL_DestroyTexture(gamestate->label);
	gamestate->label = NULL;
}

void init_game_over(GameState* gamestate) {
	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Surface* tmp = TTF_RenderText_Blended(gamestate->font, "GAME OVER", white);
	gamestate->gameOverLabel = SDL_CreateTextureFromSurface(gamestate->renderer, tmp);
	SDL_FreeSurface(tmp);

	SDL_Rect textRect;
	SDL_QueryTexture(gamestate->gameOverLabel, NULL, NULL, &textRect.w, &textRect.h);
	textRect.x = 320 - textRect.w / 2;
	textRect.y = 240 - 50;

	SDL_RenderCopy(gamestate->renderer, gamestate->gameOverLabel, NULL, &textRect);
}

