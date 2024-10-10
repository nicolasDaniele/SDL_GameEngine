#include "../entity.h"
#include "../array_list.h"
#include "../util.h"

#include <stdio.h>

static Array_List *entity_list;

void entity_init(void) {
	entity_list = array_list_create(sizeof(Entity), 0);
}

usize entity_create(vec2 position, vec2 size, vec2 velocity, ui8 collision_layer, ui8 collision_mask, bool is_kinematic, On_Hit on_hit, On_Hit_Static on_hit_static) {
	usize id = entity_list->len;

	for(usize i = 0; i < entity_list->len; ++i) {
		Entity *entity = array_list_get(entity_list, i);
		if(!entity->is_active) {
			printf("WARNING: entity_create: entity with id %zd is inactive\n", i);
			id = i;
			break;
		}
	}

	if(id == entity_list->len) {
		if(array_list_append(entity_list, &(Entity){0}) == (usize)-1) {
			printf("WARNING: entity_create: Could not append entity to list\n");
			ERROR_EXIT("Could not append entity to list\n");
		}
	}

	Entity *entity = entity_get(id);
	*entity = (Entity){
		.is_active = true,
		.animation_id = (usize)-1,
		.body_id = physics_body_create(position, size, velocity, collision_layer, collision_mask, is_kinematic, on_hit, on_hit_static),
	};

	//printf("entity_create: id: %zd\n", id);

	return id;
}

Entity *entity_get(usize id) {
	//printf("entity_get %zd\n", id);

	return array_list_get(entity_list, id);
}

usize entity_count() {
	return entity_list->len;
}