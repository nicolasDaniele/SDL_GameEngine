#include <glad/glad.h>
#include <SDL2/SDL.h>

#include "../util.h"
#include "../global.h"

#include "../render.h"
#include "render_internal.h"

SDL_Window *render_init_window(ui32 width, ui32 height) {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		ERROR_EXIT("Could not init SDL: %s\n", SDL_GetError());
	}

	SDL_Window *window = SDL_CreateWindow(
		"MyGame",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);

	if(!window) {
		ERROR_EXIT("Failed to init window: %s\n", SDL_GetError());
	}

	SDL_GL_CreateContext(window);
	if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		ERROR_EXIT("Failed to load GL: %s\n", SDL_GetError());
	}

	return window;
}

void render_init_shaders(ui32 *shader_default, ui32 *shader_batch, f32 render_width, f32 render_height) {
	mat4x4 projection;
	*shader_default = render_shader_create("./shaders/default.vert", "./shaders/default.frag");
	*shader_batch = render_shader_create("./shaders/batch_quad.vert", "./shaders/batch_quad.frag");

	mat4x4_ortho(projection, 0, render_width, 0, render_height, -2, 2);

	glUseProgram(*shader_default);
	glUniformMatrix4fv(
		glGetUniformLocation(*shader_default, "projection"),
		1, GL_FALSE, &projection[0][0]
	);

	glUseProgram(*shader_batch);
	glUniformMatrix4fv(
		glGetUniformLocation(*shader_batch, "projection"),
		1, GL_FALSE, &projection[0][0]
	);

	for(ui32 i = 0; i < 8; ++i) {
		char name[] = "texture_slot_N";
		sprintf(name, "texture_slot_%u", i);
		glUniform1i(glGetUniformLocation(*shader_batch, name), i);
	}
}

void render_init_batch_quads(ui32 *vao, ui32 *vbo, ui32 *ebo) {
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	ui32 indices[MAX_BATCH_ELEMENTS];
	for(ui32 i = 0, offset = 0; i < MAX_BATCH_ELEMENTS; i += 6, offset += 4) {
		indices[i + 0] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;
		indices[i + 3] = offset + 2;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 0;
	}

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(Batch_Vertex), NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*)offsetof(Batch_Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void*)offsetof(Batch_Vertex, uvs));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT,GL_FALSE, sizeof(Batch_Vertex), (void*)offsetof(Batch_Vertex, color));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT,GL_FALSE, sizeof(Batch_Vertex), (void*)offsetof(Batch_Vertex, texture_slot));

	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BATCH_ELEMENTS * sizeof(ui32), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render_init_color_texture(ui32 *texture) {
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	ui8 solid_white[4] = {255, 255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, solid_white);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_init_quad(ui32 *vao, ui32 *vbo, ui32 *ebo){
	f32 vertices[] = {
		 0.5,  0.5, 0, 0, 0,
		 0.5, -0.5, 0, 0, 1,
		-0.5, -0.5, 0, 1, 1,
		-0.5,  0.5, 0, 1, 0	
	};

	ui32 indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glGenBuffers(1, ebo);

	glBindVertexArray(*vao);

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void render_init_line(ui32 *vao, ui32 *vbo) {
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}