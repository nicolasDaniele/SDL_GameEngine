#pragma once 

#include "types.h"

typedef struct time_state {
	f32 delta;
	f32 now;
	f32 last;

	f32 frame_last;
	f32 frame_delay;
	f32 frame_time;

	ui32 frame_rate;
	ui32 frame_count;
} Time_State;

void time_init(ui32 frame_rate);
void time_update(void);
void time_update_late(void);