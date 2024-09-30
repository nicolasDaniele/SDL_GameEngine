#pragma once

#include <SDL2/SDL.h>

#include "../types.h"
#include "../render.h"

typedef struct render_state_internal {
	ui32 vao_quad;
	ui32 vbo_quad;
	ui32 ebo_quad;
	ui32 vao_line;
	ui32 vbo_line;
	ui32 shader_default;
	ui32 texture_color;
	mat4x4 projection;
} Render_State_Internal;

SDL_Window *render_init_window(ui32 width, ui32 height);
void render_init_color_texture(ui32 *texture);
void render_init_shaders(Render_State_Internal *state);
void render_init_quad(ui32 *vao, ui32 *bvo, ui32 *ebo);
void render_init_line(ui32 *vao, ui32 *vbo);
ui32 render_shader_create(const char *path_vert, const char *path_frag);