#include "raylib.h"
#include "raymath.h"
#include "game.h"
#include "config.h"
#include "sprites.h"

RenderTexture2D render_target;

// Game state update and draw function type defines
typedef void(*UpdateFunc)(Game *game, float dt);
typedef void(*DrawFunc)(Game *game);

// Game state update and draw function arrays, state acts as index
UpdateFunc game_update_funcs[] = { TitleUpdate, MainUpdate, OverScreenUpdate };
DrawFunc game_draw_funcs[] = { TitleDraw, MainDraw, OverScreenDraw };

void GameInit(Game *game) 
{
	// Initialize config struct and read options from file
	game->conf = (Config){0};
	ConfigRead(&game->conf, "options.txt");

	// Initialize camera
	game->cam = (Camera2D) {
		.target = {0, 0},
		.offset = Vector2Scale((Vector2){VIRTUAL_WIDTH, VIRTUAL_HEIGHT}, 0.5f),
		.rotation = 0.0f,
		.zoom = 1.0f
	};

	game->input_state = (InputState){0};

	EntHandlerInit(&game->ent_handler, &game->sprite_loader, &game->cam);
}

void GameRenderInit(Game *game) 
{
	// Load empty texture, used as buffer for scaling
	render_target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
	SetTextureFilter(render_target.texture, TEXTURE_FILTER_POINT);

	// Set source and destination rectangle values for window scaling
	game->render_src_rec  = (Rectangle) { 0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT };
	game->render_dest_rec = (Rectangle) { 0, 0, game->conf.windowWidth, game->conf.windowHeight };
}

void GameContentInit(Game *game) 
{
	game->sprite_loader = (SpriteLoader){0};
	LoadSpritesAll(&game->sprite_loader);
}

void GameUpdate(Game *game) 
{
	// Get delta time once only, pass to other update functions
	float delta_time = GetFrameTime();

	// Send quit request on hitting escape
	if(IsKeyPressed(KEY_ESCAPE))
		game->flags |= GAME_QUIT_REQUEST;

	// Default to gamepad controls if available and input method unspecified
	if((game->flags & INPUT_SPECIFIED) == 0) 
		if(IsGamepadAvailable(0)) game->input_method = GAMEPAD; 

	ProcessInput(&game->input_state, delta_time);
	
	// Call state appropriate update function
	game_update_funcs[game->state](game, delta_time);
}

void GameDrawToBuffer(Game *game) 
{
	// Draw to render_target texture
	BeginTextureMode(render_target);
	ClearBackground(BLACK);

	// Call state appropriate draw function
	game_draw_funcs[game->state](game);

	EndTextureMode();
}

void GameDrawToWindow(Game *game) 
{
	BeginDrawing();
	ClearBackground(BLACK);
	
	// Draw scaled render_target texture to window 
	DrawTexturePro(render_target.texture, game->render_src_rec, game->render_dest_rec, Vector2Zero(), 0, WHITE);
	EndDrawing();
}

void GameClose(Game *game) 
{
	UnloadRenderTexture(render_target);
	SpriteLoaderClose(&game->sprite_loader);
}

void TitleUpdate(Game *game, float delta_time) 
{
	if(IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
		MainStart(game);
}

void TitleDraw(Game *game) 
{
	Vector2 screen_center = Vector2Scale((Vector2){VIRTUAL_WIDTH, VIRTUAL_HEIGHT}, 0.5f);
	char *prompt_text = (game->input_method == KEYBOARD) ? "press space to play" : "press A to play";

	DrawText("Fish game Demo", screen_center.x - 380, screen_center.y - 100, 100, RAYWHITE);
	DrawText(prompt_text, screen_center.x - 160, screen_center.y + 100, 32, RAYWHITE);
}

void MainUpdate(Game *game, float delta_time) 
{
	EntHandlerUpdate(&game->ent_handler, delta_time);
}

void MainDraw(Game *game) 
{
	BeginMode2D(game->cam);
	EntHandlerDraw(&game->ent_handler);
	EndMode2D();
}

void OverScreenUpdate(Game *game, float delta_time) 
{
}

void OverScreenDraw(Game *game) 
{
}

void OptionsScreenUpdate(Game *game, float delta_time) 
{
}

void OptionsScreenDraw(Game *game) 
{
}

void MainStart(Game *game) 
{
	EntMake(&game->ent_handler, ENT_PLAYER);
	game->ent_handler.ents[0].position = (Vector2){-90, 100};

	PlayerData *p = game->ent_handler.ents[0].data;
	p->input = &game->input_state;
	
	//game->ent_handler.ents[0].position = (Vector2){0, 0};

	AsteroidSpawn(&game->ent_handler, (Vector2){0, 0});
	AsteroidSpawn(&game->ent_handler, (Vector2){100, -300});

	game->state = GAME_MAIN;
}

