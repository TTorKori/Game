#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "status.h"

const int WIDTH = 640, HEIGHT = 480;
const float GRAVITY = 0.32f;

void initGame(GameState* game) {
	game->scrollX = 0.0f;
	game->man.x = 10;
	game->man.y = 300;
	game->man.dx = 0;
	game->man.dy = 0;
	game->man.onLedge = 0;
	game->man.animFrame = 0;
	game->man.lives = 3;
	game->man.isDead = 0;
	game->time = 0;
	game->man.facingLeft = 1;
	game->statusState = LIVES;
	game->deathCountDown = -1;
}

void resetGame(GameState* game) {
	game->man.x = 10;
	game->man.y = 300;
	game->man.dx = 0;
	game->man.dy = 0;
	game->man.onLedge = 0;
	game->man.animFrame = 0;
	game->man.isDead = 0;
}

void renderRect(GameState* game) {

	if (game->statusState == LIVES) {
		draw_status_lives(game);
	}
	else if (game->statusState == GAME) {

		SDL_Rect manRect;
		manRect.x = (int)(game->scrollX + game->man.x);
		manRect.y = (int)game->man.y;
		SDL_QueryTexture(game->manFrames[0], NULL, NULL, &manRect.w, &manRect.h);

		SDL_Rect leanRect;
		SDL_QueryTexture(game->lean, NULL, NULL, &leanRect.w, &leanRect.h);

		SDL_Rect ledgeRect;
		SDL_QueryTexture(game->ground, NULL, NULL, &ledgeRect.w, &ledgeRect.h);
		SDL_RenderClear(game->renderer);
		SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(game->renderer, NULL);

		for (int i = 0; i < 100; i++) {
			ledgeRect.x = (int)game->scrollX + game->ledges[i].x;
			ledgeRect.y = game->ledges[i].y;
			SDL_RenderCopy(game->renderer, game->ground, NULL, &ledgeRect);
		}


		/*for (int i = 0; i < NUM_STARS; i++) {
			leanRect.x = (int)(game->scrollX + game->leans[i].x);
			leanRect.y = game->leans[i].y;
			SDL_RenderCopy(game->renderer, game->lean, NULL, &leanRect);
		}*/

		SDL_RenderCopyEx(game->renderer, game->manFrames[game->man.animFrame], NULL, &manRect, 0, NULL, game->man.facingLeft == 0);
	}
	SDL_RenderPresent(game->renderer);
}

int processEvents(GameState* game) {
	SDL_Event winEvent;
	while (SDL_PollEvent(&winEvent)) {
		switch (winEvent.type) {
			case SDL_QUIT:
				return 1;
				break;
			case SDL_KEYDOWN:
				switch (winEvent.key.keysym.sym) {
					case SDLK_ESCAPE:
						return 1;
						break;
					case SDLK_UP:
						if (game->man.onLedge) {
							game->man.dy = -8;
							game->man.onLedge = 0;
						}
						break;
				}
				break;
			default:
				break;
		}
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) {
		game->man.dy -= 0.2f;
	}

	if (state[SDL_SCANCODE_LEFT]) {
		game->man.dx -= 0.5;
		if (game->man.dx < -6) {
			game->man.dx = -6;
		}
		game->man.facingLeft = 1;
	}
	else if (state[SDL_SCANCODE_RIGHT]) {
		game->man.dx += 0.5;
		if (game->man.dx > 6) {
			game->man.dx = 6;
		}
		game->man.facingLeft = 0;
	}
	else {
		game->man.animFrame = 0;
		game->man.dx *= 0.8f;
		if (fabsf(game->man.dx) < 0.1f) {
			game->man.dx = 0;
		}
	}
	//if (state[SDL_SCANCODE_UP]) {
	//	game->man.y -= 10;
	//}
	//if (state[SDL_SCANCODE_DOWN]) {
	//	game->man.y += 10;
	//}
	return 0;
}


void process(GameState* game) {
	game->time++;

	if (game->statusState == LIVES) {
		if (game->time > 120) {
			shutdown_status_lives(game);
			game->statusState = GAME;
			game->musicChannel = Mix_PlayChannel(-1, game->bgMusic, -1);
		}
	}
	else if (game->statusState == GAMEOVER) {
		if (game->time > 240) {
			SDL_Quit();
			exit(0);
		}
	}

	if (game->statusState == GAME) {

		if (!game->man.isDead) {
			Man* man = &game->man;
			man->x += man->dx;
			man->y += man->dy;

			if (man->dx != 0 && man->onLedge) {
				if (game->time % 8 == 0) {
					if (man->animFrame == 0) {
						man->animFrame = 1;
					}
					else if (man->animFrame == 1) {
						man->animFrame = 0;
					}
				}
			}
			man->dy += GRAVITY;
		}

		if (game->man.isDead && game->deathCountDown < 0) {
			game->deathCountDown = 120;
		}
		if (game->deathCountDown >= 0) {
			game->deathCountDown--;
			if (game->deathCountDown < 0) {
				game->man.lives--;

				if(game->man.lives >= 0) {
					init_status_lives(game);
					game->statusState = LIVES;
					game->time = 0;

					resetGame(game);
				}
				else {
					init_game_over(game);
					game->statusState = GAMEOVER;
				}
			}
		}
	}

	game->scrollX = -(game->man.x - 320);
	if (game->scrollX > 0) {
		game->scrollX = 0;
	}
}

int collide2d(float x1, float y1, float x2, float y2, float w1, float h1, float w2, float h2) {
	return (!((x1 > (x2 + w2)) || (x2 > (x1 + w1)) || (y1 > (y2 + h2)) || (y2 > (y1 + h1))));
}


void collisionDetect(GameState* game) {
	for (int i = 0; i < NUM_STARS; i++) {
		if (collide2d(game->man.x, game->man.y, game->leans[i].x, game->leans[i].y, 21, 75, 25, 25)) {
			game->man.isDead = 1;
			Mix_HaltChannel(game->musicChannel);
			break;
		};
	}


	for (int i = 0; i < 100; i++) {
		float mW = 21, mH = 64;
		float mX = game->man.x, mY = game->man.y;
		float bW = (float)game->ledges[i].w, bH = (float)game->ledges[i].h;
		float bX = (float)game->ledges[i].x, bY = (float)game->ledges[i].y;

		if (mX + mW / 2 > bX && mX + mW / 2 < bX + bW) {
			if (mY < bY + bH && mY > bY && game->man.dy < 0) {
				game->man.y = bY + bH;
				mY = bY + bH;

				game->man.dy = 0;
				game->man.onLedge = 0;
			}
		}

		if (mX + mW > bX && mX < bX + bW) {
			if (mY + mH > bY && mY < bY && game->man.dy > 0) {
				game->man.y = bY - mH;
				mY = bY - mH;


				game->man.dy = 0;
				game->man.onLedge = 1;
			}
		}


		if ((mY + mH > bY) && (mY < bY + bH)) {
			//Right edge collision
			if (mX < bX+bW && mX+mW > bX+bW && game->man.dx < 0) {
				game->man.x = bX + bW;
				mX = bX + bW;

				game->man.dx = 0;
			}
			//Left Edge Collision
			else if (mX + mW > bX && mX < bX && game->man.dx > 0) {
				game->man.x = bX - mW;
				mX = bX - mW;

				game->man.dx = 0;
			}
		}
	}
};

int main(int argc, char* args[]) {
	SDL_Window* window = NULL;
	SDL_Surface* surface = NULL;

	srand((int)time(NULL));

	TTF_Init();
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

	GameState game;
	initGame(&game);

	/*for (int i = 0; i < NUM_STARS; i++) {
		game.leans[i].x = rand()%2000;
		game.leans[i].y = rand()%480 - 30;
	}*/

	for (int i = 0; i < 25; i++) {
		game.ledges[i].w = 30;
		game.ledges[i].h = 30;
		game.ledges[i].x = i * 30;
		game.ledges[i].y = HEIGHT - 30;
	}

	for (int i = 25; i < 40; i++) {
		game.ledges[i].w = 30;
		game.ledges[i].h = 30;
		game.ledges[i].x = i * 30;
		game.ledges[i].y = HEIGHT - 300;
	}

	for (int i = 40; i < 60; i++) {
		game.ledges[i].w = 30;
		game.ledges[i].h = 30;
		game.ledges[i].x = i * 35;
		game.ledges[i].y = rand()%480;
	}

	for (int i = 60; i < 100; i++) {
		game.ledges[i].w = 30;
		game.ledges[i].h = 30;
		game.ledges[i].x = i * 30;
		game.ledges[i].y = HEIGHT - 300;
	}
	game.ledges[98].x = WIDTH / 2 - 30;
	game.ledges[98].y = HEIGHT / 2;
	game.ledges[99].x = WIDTH / 2;
	game.ledges[99].y = HEIGHT / 2;

	int done = 0;
	
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL could not be initialised!");
		return EXIT_FAILURE;
	}

	if (!(IMG_Init(IMG_INIT_PNG) && 2)) {
		printf("SDL Image could not be initialised!");
		SDL_Quit();
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow("Game Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("SDL window could not be created!");
		SDL_Quit();
		return EXIT_FAILURE;
	}

	game.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (game.renderer == NULL) {
		SDL_DestroyWindow(window);
		printf("SDL renderer could not be created!");
		SDL_Quit();
		return EXIT_FAILURE;
	}

	surface = IMG_Load("assets/lit.png");
	game.manFrames[0] = SDL_CreateTextureFromSurface(game.renderer, surface);
	SDL_FreeSurface(surface);
	
	if (game.manFrames[0] == NULL) {
		printf("SDL surface could not be made!");
		SDL_GetError();
		SDL_DestroyRenderer(game.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	};

	surface = IMG_Load("assets/lit2.png");
	game.manFrames[1] = SDL_CreateTextureFromSurface(game.renderer, surface);
	if (game.manFrames[1] == NULL) {
		printf("SDL surface could not be made!");
		SDL_GetError();
		SDL_DestroyRenderer(game.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	};

	surface = IMG_Load("assets/lean.png");
	game.lean = SDL_CreateTextureFromSurface(game.renderer, surface);
	SDL_FreeSurface(surface);

	if (game.lean == NULL) {
		printf("SDL surface could not be made!");
		SDL_GetError();
		SDL_DestroyRenderer(game.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	};

	surface = IMG_Load("assets/ground.png");
	game.ground = SDL_CreateTextureFromSurface(game.renderer, surface);
	SDL_FreeSurface(surface);

	if (game.ground == NULL) {
		printf("SDL surface could not be made!");
		SDL_GetError();
		SDL_DestroyRenderer(game.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	};

	game.font = TTF_OpenFont("assets/Ernest.ttf", 48);
	if (!game.font) {
		printf("SDL TTF font could not be created!");
		SDL_GetError();
		SDL_DestroyRenderer(game.renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	game.bgMusic = Mix_LoadWAV("assets/bgMusic.wav");
	if (game.bgMusic != NULL) {
		Mix_VolumeChunk(game.bgMusic, 64);
	}

	game.label = NULL;

	init_status_lives(&game);

	while (!done) {
		done = processEvents(&game);
		renderRect(&game);
		process(&game);
		collisionDetect(&game);
	}

	if (game.label != NULL) {
		SDL_DestroyTexture(game.label);
	}

	Mix_FreeChunk(game.bgMusic);
	TTF_CloseFont(game.font);
	SDL_DestroyTexture(game.manFrames[0]);
	SDL_DestroyTexture(game.lean);
	SDL_DestroyRenderer(game.renderer);
	SDL_DestroyWindow(window);
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
	return EXIT_SUCCESS;
}