#pragma once

#include "render.h"

#define MAX_FRAMES 16

typedef struct animation_frame {
	f32 duration;
	ui8 row;
	ui8 column;
} Animation_Frame;

typedef struct animation_definition {
	Sprite_Sheet *sprite_sheet;
	Animation_Frame frames[MAX_FRAMES];
	ui8 frame_count;
} Animation_Definition;

typedef struct animation {
	Animation_Definition *definition;
	f32 current_frame_time;
	ui8 current_frame_index;
	bool does_loop;
	bool is_active;
	bool is_flipped;
} Animation;

void animation_init(void);
usize animation_definition_create(Sprite_Sheet *sprite_sheet, f32 *durations, ui8 *rows, ui8 *columns, ui8 frame_count);
usize animation_create(usize animation_definition_id, bool does_loop);
void animation_destroy(usize id);
Animation *animation_get(usize id);
void animation_update(f32 dt);