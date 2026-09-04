#pragma once

#include "entity.h"
#include "components.h"
#include "vulkan_objects.h"

typedef struct Scene
{
	Entity* entities;
	MeshRendererComponent* mesh_renderer_components;
	TransformComponent* transform_components;
	PhysicsComponent* physics_components;
	CameraComponent* camera_components;

	size_t entities_count;
	size_t mesh_renderer_components_count;
	size_t transform_components_count;
	size_t physics_components_count;
	size_t camera_components_count;

	BufferResource* display_buffer;
	BufferResource* physics_buffer;
	BufferResource* transform_buffer;
} Scene;

Scene* Scene_create(const char* path);
void Scene_replace(Scene* scene, const char* path, SDL_RWLock* rw_lock);
void Scene_simulate(Scene* scene, const float delta_time, const size_t substeps, const size_t frame_number);
void Scene_play(Scene* scene, const size_t frame_number);
void Scene_render(Scene* scene, SDL_RWLock* rw_lock);
void Scene_destroy(Scene* scene);