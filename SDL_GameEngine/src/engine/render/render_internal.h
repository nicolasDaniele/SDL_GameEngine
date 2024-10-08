#pragma once

#include <SDL2/SDL.h>

#include "../types.h"
#include "../render.h"

SDL_Window *render_init_window(ui32 width, ui32 height);
void render_init_color_texture(ui32 *texture);
void render_init_shaders(ui32 *shader_default, f32 render_width, f32 render_height);
void render_init_quad(ui32 *vao, ui32 *bvo, ui32 *ebo);
void render_init_line(ui32 *vao, ui32 *vbo);
ui32 render_shader_create(const char *path_vert, const char *path_frag);