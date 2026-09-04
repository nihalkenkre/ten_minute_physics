#pragma once

#include <stdint.h>

typedef struct Entity
{
	char name[64];
	uint32_t id;

	int32_t transform_component_idx;
	int32_t mesh_component_idx;
	int32_t physics_component_idx;
	int32_t camera_component_idx;
} Entity;