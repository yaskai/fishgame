#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "raylib.h"
#include "raymath.h"
#include "ent_handler.h"
#include "entity.h"

uint16_t type_max[] = {
	MAX_PLAYERS,
	MAX_FISH,
	MAX_NPCS,
	MAX_ASTEROIDS
};

Vector2 ray_start;
Vector2 ray_end;

typedef void(*ReserveDataFunc)(EntHandler *handler, Entity *ent);
ReserveDataFunc data_reserve_funcs[] = { &ReserveDataPlayer, &ReserveDataFish, &ReserveDataNpc, &ReserveDataAsteroid };

typedef void(*EntUpdateFunc)(Entity *ent, float dt);
EntUpdateFunc ent_update_funcs[] = { &PlayerUpdate, &AsteroidUpdate, NULL, NULL };

typedef void(*EntDrawFunc)(Entity *ent, SpriteLoader *sl);
EntDrawFunc ent_draw_funcs[] = { &PlayerDraw, &AsteroidDraw, NULL, NULL };

void EntHandlerInit(EntHandler *handler, SpriteLoader *sprite_loader, Camera2D *camera) 
{
	handler->sprite_loader = sprite_loader;
	handler->camera = camera;
}

void EntHandlerUpdate(EntHandler *handler, float dt) 
{
	int16_t nearest_body_id = -1;
	int16_t cast_body_id = -1;
	float nearest_body_dist = FLT_MAX;

	Entity *player = &handler->ents[0];
	PlayerData *p = player->data;
	Vector2 player_center = Vector2Add(player->position, player->center_offset);

	ray_start = player_center;
	ray_end = Vector2Add(player_center, Vector2Scale(p->orbit_dir, 2000));

	for(uint16_t i = 0; i < handler->count; i++) 
	{
		// Get entity pointer
		Entity *ent = &handler->ents[i];

		// Skip update if not active
		if(!(ent->flags & ENT_ACTIVE)) continue;

		// Call entity's update function
		if(ent->update) ent->update(ent, dt);

		// Find nearest orbitable object to player 
		if(!(ent->flags & ENT_IS_BODY)) continue;

		if(player->flags & ENT_CAST_ORBIT)
		{
			if(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 3.0f, player_center, ray_end))
			{
				//if(i == p->anchor_id) return;
				//p->anchor_id = i;
			}
		}

		float dist = Vector2Distance(EntCenter(ent), player_center);
		if(dist < nearest_body_dist) 
		{
			nearest_body_id = i;
			nearest_body_dist = dist;
		}
	}

	if(nearest_body_id > -1) 
	{
		Entity *body = &handler->ents[nearest_body_id];
		Vector2 body_center = EntCenter(body);

		if(player->flags & ENT_ORBIT)
			p->prev_anchor_id = p->anchor_id;

		if(CheckCollisionCircles(player_center, player->radius, body_center, body->radius * 3)) 
		{
			//p->anchor_id = nearest_body_id;
			
			if(((player->flags & ENT_ORBIT) == 0) || p->anchor_id != nearest_body_id)
			//if((player->flags & ENT_ORBIT) == 0)
			{
				EntOrbitStart(player, body);
				p->anchor_id = nearest_body_id;
			}
		}

		//PlayerPhysicsOrbit(player, body, dt);
		EntOrbitUpdate(player, body, dt);
	}
}

void EntHandlerDraw(EntHandler *handler) 
{
	//DrawLine(ray_start.x, ray_start.y, ray_end.x, ray_end.y, WHITE);

	for(uint16_t i = 1; i < handler->count; i++) {
		// Get entity pointer
		Entity *ent = &handler->ents[i];
			
		// Skip draw call if entitiy inactive
		if(!(ent->flags & ENT_ACTIVE)) continue;
		
		// Call entity's draw function
		if(ent->draw) ent->draw(ent, handler->sprite_loader);
	}
	
	handler->ents[0].draw(&handler->ents[0], handler->sprite_loader);
}

int16_t EntMake(EntHandler *handler, uint8_t type) 
{
	// Get count for entity type
	uint16_t *type_count = &handler->type_counts[type];
	
	// Dont't add entity data if slots are full
	if(*type_count >= type_max[type]) return -1;
	
	// Get entity pointer
	Entity *ent = &handler->ents[handler->count]; 
	ent->type = type;

	// Initialize entity
	*ent = (Entity){0};
	ent->flags |= ENT_ACTIVE;

	// Set function pointers
	ent->update = ent_update_funcs[type];
	ent->draw = ent_draw_funcs[type];
	
	// Reserve data
	data_reserve_funcs[type](handler, &handler->ents[handler->count]);
	
	// Increment count for entity type
	(*type_count)++;

	// Return entity's index 
	return handler->count++;
}

void ReserveDataPlayer(EntHandler *handler, Entity *ent) 
{
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_PLAYER];

	// Init data
	PlayerData player_data = (PlayerData){0};
	handler->player_data[data_id] = player_data;

	// Set entity data pointer
	ent->data = &handler->player_data[data_id];
	PlayerInit(ent, handler->sprite_loader, handler->camera);
}

void ReserveDataAsteroid(EntHandler *handler, Entity *ent) 
{
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_ASTEROID];

	// Init data
	AsteroidData data = (AsteroidData){0};
	handler->asteroid_data[data_id] = data;

	// Set entity data pointers
	ent->data = &handler->asteroid_data[data_id];
}

void ReserveDataFish(EntHandler *handler, Entity *ent) 
{
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_FISH];

	// Init data
	FishData data = (FishData){0};
	handler->fish_data[data_id] = data;

	// Set entity data pointer
	ent->data = &handler->fish_data[data_id];
}

void ReserveDataNpc(EntHandler *handler, Entity *ent) 
{
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_NPC];

	// Init data
	NpcData data = (NpcData){0};
	handler->npc_data[data_id] = data;

	// Set entity data pointer
	ent->data = &handler->fish_data[data_id];
}

void AsteroidSpawn(EntHandler *handler, Vector2 position) 
{
	int16_t id = EntMake(handler, ENT_ASTEROID);
	if(id == -1) return;

	Entity *ast = &handler->ents[id];
	ast->position = position;
	ast->radius = handler->sprite_loader->spr_pool[1].frame_w * 0.5f;
	ast->center_offset = (Vector2){ast->radius, ast->radius};
	ast->flags |= ENT_IS_BODY;
}

