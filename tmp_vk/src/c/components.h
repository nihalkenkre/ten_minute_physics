#pragma once

#include <volk.h>
#include <cglm/struct.h>

typedef struct TransformComponent
{
	VkDeviceSize xform_matrix_offset;
} TransformComponent;

typedef struct CameraComponent
{
	VkDeviceSize proj_matrix_offset;
	VkDeviceSize view_matrix_offset;
} CameraComponent;

typedef struct MeshRendererComponent
{
	VkDeviceSize positions_offset;
	VkDeviceSize normals_offset;
	VkDeviceSize uv0_offset;
	VkDeviceSize indices_offset;

	size_t indices_count;
} MeshRendererComponent;

typedef struct PhysicsComponent
{
	VkDeviceSize positions_offset;
	VkDeviceSize indices_offset;

	size_t indices_count;
} PhysicsComponent;