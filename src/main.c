#include <stdio.h>
#include "raylib.h"
#include "game.h"
#include "config.h"

int main() {
	SetTraceLogLevel(LOG_ERROR);
	// Initialize game
	// Set window options, instantiate objects, allocate memory, etc.
	Game game = {0};
	GameInit(&game);

	// Open window, use values from config file
	SetConfigFlags(0);
	InitWindow(game.conf.windowWidth, game.conf.windowHeight, "Fish Game Demo");
	SetTargetFPS(game.conf.refreshRate);
	
	// Load empty texture for use as a buffer
	// Buffer is drawn and scaled to window resolution
	GameRenderInit(&game);

	GameContentInit(&game);

	SetExitKey(KEY_F10);	
	bool exit = false;

	while(!exit) {
		exit = (WindowShouldClose() || (game.flags & GAME_QUIT_REQUEST));	

		// Update game logic
		GameUpdate(&game);

		// Render to buffer
		GameDrawToBuffer(&game);

		// Render to screen
		GameDrawToWindow(&game);
	}

	// Free allocated memory, close application
	GameClose(&game);
	CloseWindow();
	return 0;
}

