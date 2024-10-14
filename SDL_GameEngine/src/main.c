#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glad/glad.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "engine/global.h"
#include "engine/config.h"
#include "engine/input.h"
#include "engine/time.h"
#include "engine/physics.h"
#include "engine/util.h"
#include "engine/entity.h"
#include "engine/render.h"
#include "engine/animation.h"
#include "engine/audio.h"

static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *SOUND_JUMP;

static const f32 SPEED_ENEMY_LARGE = 200;
static const f32 SPEED_ENEMY_SMALL = 400;
static const f32 HEALTH_ENEMY_LARGE = 7;
static const f32 HEALTH_ENEMY_SMALL = 3;

typedef enum collision_layer {
	COLLISION_LAYER_PLAYER = 1,
	COLLISION_LAYER_ENEMY = 1 << 1,
	COLLISION_LAYER_TERRAIN = 1 << 2,
	COLLISION_LAYER_ENEMY_PASSTHROUGH = 1 << 3
} Collision_Layer;

static f32 render_width;
static f32 render_height;

static bool should_quit = false;
static vec4 player_color = {0, 1, 1, 1};
static bool player_is_grounded = false;
static usize anim_player_walk_id;
static usize anim_player_idle_id;
static usize anim_enemy_small_id;
static usize anim_enemy_large_id;

static ui8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
static ui8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
static ui8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

static void input_handle(Body *body_player) {
	if(global.input.escape)
		should_quit = true;

	Animation *walk_anim = animation_get(anim_player_walk_id);
	Animation *idle_anim = animation_get(anim_player_idle_id);

	f32 velx = 0;
	f32 vely = body_player->velocity[1];

	if(global.input.right) {
		velx += 300;
		walk_anim->is_flipped = false;
		idle_anim->is_flipped = false;
	}
	if(global.input.left) {
		velx -= 300;
		walk_anim->is_flipped = true;
		idle_anim->is_flipped = true;
	}
	if(global.input.up && player_is_grounded) {
		player_is_grounded = false;
		vely = 1500;
		audio_sound_play(SOUND_JUMP);
	}

	body_player->velocity[0] = velx;
	body_player->velocity[1] = vely;
}

void player_on_hit(Body *self, Body *other, Hit hit) {
	if(other->collision_layer == COLLISION_LAYER_ENEMY) {
		player_color[0] = 1;
		player_color[2] = 0;
	}
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit) {
	if(hit.normal[1] > 0) {
		player_is_grounded = true;
	}
}

void enemy_small_on_hit_static(Body *self, Static_Body *other, Hit hit) {
	if(hit.normal[0] > 0) {
		self->velocity[0] = SPEED_ENEMY_SMALL;
	}

	if(hit.normal[0] < 0) {
		self->velocity[0] = -SPEED_ENEMY_SMALL;
	} 
}

void enemy_large_on_hit_static(Body *self, Static_Body *other, Hit hit) {
	if(hit.normal[0] > 0) {
		self->velocity[0] = SPEED_ENEMY_LARGE;
	}

	if(hit.normal[0] < 0) {
		self->velocity[0] = -SPEED_ENEMY_LARGE;
	} 
}

void fire_on_hit(Body *self, Body *other, Hit hit) {
	if(other->collision_layer == COLLISION_LAYER_ENEMY) {
		for(usize i = 0; i < entity_count(); ++i) {
			Entity *entity = entity_get(i);
			if(entity->body_id == hit.other_id) {
				Body *body = physics_body_get(entity->body_id);
				body->is_active = false;
				entity->is_active = false;
				break;
			}
		}
	}
}

void spawn_enemy(bool is_small, bool is_enraged, bool is_flipped) {
	f32 spawn_x = is_flipped ? render_width : 0;
	f32 speed = is_small ? SPEED_ENEMY_SMALL : SPEED_ENEMY_LARGE;

	if(is_enraged) {
		speed *= 1.5;
	}

	vec2 position = {spawn_x, (render_height - 64)};

	if(is_small) {
		vec2 size = {12, 12};
		vec2 sprite_offset = {0, 6};
		vec2 velocity = {is_flipped ? -speed : speed, 0};
		//entity_create(position, size, sprite_offset, velocity, COLLISION_LAYER_ENEMY, enemy_mask, false, anim_enemy_small_id, NULL, enemy_small_on_hit_static);
	}
	else {
		vec2 size = {20, 20};
		vec2 sprite_offset = {0, 10};
		vec2 velocity = {is_flipped ? -speed : speed, 0};
		//entity_create(position, size, sprite_offset, velocity, COLLISION_LAYER_ENEMY, enemy_mask, false, anim_enemy_large_id, NULL, enemy_large_on_hit_static);
	}
}

int main(int argc, char *argv[]) {
	time_init(60);
	config_init();
	SDL_Window *window = render_init();
	physics_init();
	entity_init();
	animation_init();
	audio_init();

	audio_sound_load(&SOUND_JUMP, "assets/jump.wav");
	audio_music_load(&MUSIC_STAGE_1, "assets/map.wav");
	audio_music_play(MUSIC_STAGE_1);

	SDL_ShowCursor(false);

	usize player_id = entity_create((vec2){100, 200}, (vec2){24, 24}, (vec2){0}, (vec2){0, 0} , COLLISION_LAYER_PLAYER, player_mask, false, (usize)-1, player_on_hit, player_on_hit_static);

	i32 window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	render_width = window_width / render_get_scale();
	render_height = window_height / render_get_scale();

	// LEVEL SETUP
	{
		physics_static_body_create((vec2){render_width * 0.5, render_height - 16}, (vec2){render_width, 32}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width * 0.25 - 16, 16}, (vec2){render_width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width * 0.75 + 16, 16}, (vec2){render_width *0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){16, render_height * 0.5 - 3 * 32}, (vec2){32, render_height}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width - 16, render_height * 0.5 - 3 * 32}, (vec2){32, render_height}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){32 + 64, render_height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width -32 -64, render_height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width * 0.5, render_height - 32 * 3 - 16}, (vec2){192, 32}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){render_width * 0.5, 32 * 3 + 24}, (vec2){448, 32}, COLLISION_LAYER_TERRAIN);
		physics_static_body_create((vec2){16, render_height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH);
		physics_static_body_create((vec2){render_width - 16, render_height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH);
	}

	Sprite_Sheet sprite_sheet_player;
	Sprite_Sheet sprite_sheet_map;
	Sprite_Sheet sprite_sheet_enemy_small;
	Sprite_Sheet sprite_sheet_enemy_large;
	Sprite_Sheet sprite_sheet_props;

	render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24);
	render_sprite_sheet_init(&sprite_sheet_map, "assets/map.png", 640, 360);
	render_sprite_sheet_init(&sprite_sheet_enemy_small, "assets/enemy_small.png", 24, 24);
	render_sprite_sheet_init(&sprite_sheet_enemy_large, "assets/enemy_large.png", 40, 40);
	render_sprite_sheet_init(&sprite_sheet_props, "assets/props_16x16.png", 16, 16);

	usize adef_player_walk_id = animation_definition_create(&sprite_sheet_player, 0.1, 0, (ui8[]){1, 2, 3, 4, 5, 6, 7}, 7);
	usize adef_player_idle_id = animation_definition_create(&sprite_sheet_player, 0, 0, (ui8[]){0}, 1);
	anim_player_walk_id = animation_create(adef_player_walk_id, true);
	anim_player_idle_id = animation_create(adef_player_idle_id, false);

	usize adef_enemy_small_id = animation_definition_create(&sprite_sheet_enemy_small, 0.1, 1, (ui8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
	usize adef_enemy_large_id = animation_definition_create(&sprite_sheet_enemy_large, 0.1, 1, (ui8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
	anim_enemy_small_id = animation_create(adef_enemy_small_id, true);
	anim_enemy_large_id = animation_create(adef_enemy_large_id, true);

	Entity *player = entity_get(player_id);
	player->animation_id = anim_player_idle_id;

	f32 spawn_timer = 0;
	ui32 texture_slots[8] = {0};

	while(!should_quit) {
		time_update();

		SDL_Event event;
		
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: 
					should_quit = true;
					break;
				default:
					break;
			}
		}

		Body *body_player = physics_body_get(player->body_id);

		player->animation_id = body_player->velocity[0] != 0 ?
				anim_player_walk_id : anim_player_idle_id;

		input_update();
		input_handle(body_player);
		physics_update();
		animation_update(global.time.delta);

		// SPAWN ENEMIES
		{
			
			spawn_timer -= global.time.delta;
			if(spawn_timer <= 0) {
				spawn_timer = (f32)((rand() % 200) + 200) / 100.f;
				spawn_timer *= 0.2;


				bool is_flipped = rand() % 100 >= 50;
				bool is_small = rand() % 100 > 18;
				f32 spawn_x = is_flipped ? 540 : 100;

				spawn_enemy(is_small, false, is_flipped);
	
				//usize enityt_id = entity_create((vec2){spawn_x, 200}, (vec2){20, 20}, (vec2){0, 0},
						//COLLISION_LAYER_ENEMY, enemy_mask, false, (usize)-1, NULL, enemy_small_on_hit_static);
				//Entity *entity = entity_get(entity_id);
				//Body *body = physics_body_get(entity->body_id);
				//float speed = SPEED_ENEMY_SMALL * ((rand() % 100) * 0.01) + 100;
				//body->velocity[0] = is_flipped ? -speed : speed;
			}
		}


		render_begin();

		render_sprite_sheet_frame(&sprite_sheet_map, 0, 0, (vec2){render_width * 0.5, render_height * 0.5}, false, (vec4){1, 1, 1, 0.2}, texture_slots);

		for(usize i = 0; i < entity_count(); ++i) {
			Entity* entity = entity_get(i);
			Body *body = physics_body_get(entity->body_id);			

			if(body->is_active)
				render_aabb((f32*)body, TURQUOISE);
			else
				render_aabb((f32*)body, RED);
		}

		for(usize i = 0; i < physics_static_body_count(); ++i) {
			render_aabb((f32*)physics_static_body_get(i), WHITE);
		}

		for(usize i = 0; i < entity_count(); ++i) {
			Entity *entity = entity_get(i);

			if(!entity->is_active)
				continue;
			if(!entity->is_active || entity->animation_id == (usize)-1)
				continue;

			Body *body = physics_body_get(entity->body_id);
			Animation *anim = animation_get(entity->animation_id);

			if(body->velocity[0] < 0)
				anim->is_flipped = true;
			else if(body->velocity[0] > 0)
				anim->is_flipped = false;

			vec2 pos;
			vec2_add(pos, body->aabb.position, entity->sprite_offset);
			animation_render(anim, pos, WHITE, texture_slots);
		}

		render_sprite_sheet_frame(&sprite_sheet_player, 1, 2, (vec2){100, 100}, false, WHITE, texture_slots);
		render_sprite_sheet_frame(&sprite_sheet_player, 0, 4, (vec2){100, 100}, false, WHITE, texture_slots);

		render_end(window, texture_slots);
		
		player_color[0] = 0;
		player_color[2] = 1;

		time_update_late();
	}

	return 0;
}