#include "scene.h"
#include "utils.h"
#include "events.h"

#include <stdlib.h>
#include <cgltf.h>
#include <SDL3/SDL.h>

Scene* Scene_create(const char* path)
{
	Scene* s = (Scene*)calloc(1, sizeof(Scene));

	if (path[0] == 0)
	{
		return s;
	}

	cgltf_options options = { 0 };
	cgltf_data* gltf = NULL;

	if (cgltf_parse_file(&options, path, &gltf) != cgltf_result_success ||
		cgltf_load_buffers(&options, gltf, path) != cgltf_result_success ||
		cgltf_validate(gltf) != cgltf_result_success)
	{
		printf("Could not load GLTF from %s", path);
	}

	s->entities = (Entity*)calloc(gltf->nodes_count, sizeof(Entity));
	s->entities_count = gltf->nodes_count;

	s->transform_components = (TransformComponent*)calloc(gltf->nodes_count, sizeof(TransformComponent));
	s->transform_components_count = gltf->nodes_count;

	s->mesh_renderer_components = (MeshRendererComponent*)calloc(gltf->meshes_count, sizeof(MeshRendererComponent));
	s->mesh_renderer_components_count = gltf->meshes_count;

	s->physics_components = (PhysicsComponent*)calloc(gltf->meshes_count, sizeof(PhysicsComponent));
	s->physics_components_count = gltf->meshes_count;

	s->camera_components = (CameraComponent*)calloc(gltf->cameras_count, sizeof(CameraComponent));
	s->camera_components_count = gltf->cameras_count;

	for (size_t n = 0; n < gltf->nodes_count; ++n)
	{
		cgltf_node* curr_node = gltf->nodes + n;

		uint32_t id = rand();
		int32_t res = find_entity_id(s->entities, s->entities_count, id);

		while (res >= 0)
		{
			id = rand();
			res = find_entity_id(s->entities, s->entities_count, id);
		}

		TransformComponent tc = { 0 };

		s->transform_components[n] = tc;

		Entity e = {
			.id = id,
		};
		strcpy(e.name, curr_node->name);

		s->entities[n] = e;
	}

	cgltf_free(gltf);

	return s;
}

void Scene_replace(Scene* scene, const char* path, SDL_RWLock* rw_lock)
{
	if (path[0] == 0)
	{
		return;
	}

	cgltf_options options = { 0 };
	cgltf_data* gltf = NULL;

	if (cgltf_parse_file(&options, path, &gltf) != cgltf_result_success ||
		cgltf_load_buffers(&options, gltf, path) != cgltf_result_success ||
		cgltf_validate(gltf) != cgltf_result_success)
	{
		printf("Could not load GLTF from %s", path);
	}

	SDL_LockRWLockForWriting(rw_lock);
	free(scene->entities);
	free(scene->transform_components);
	free(scene->mesh_renderer_components);
	free(scene->physics_components);
	free(scene->camera_components);

	scene->entities = calloc(gltf->nodes_count, sizeof(Entity));
	scene->entities_count = gltf->nodes_count;

	scene->transform_components = calloc(gltf->nodes_count, sizeof(TransformComponent));
	scene->transform_components_count = gltf->nodes_count;

	scene->mesh_renderer_components = calloc(gltf->meshes_count, sizeof(MeshRendererComponent));
	scene->mesh_renderer_components_count = gltf->meshes_count;

	scene->physics_components = calloc(gltf->meshes_count, sizeof(PhysicsComponent));
	scene->physics_components_count = gltf->meshes_count;

	scene->camera_components = calloc(gltf->cameras_count, sizeof(CameraComponent));
	scene->camera_components_count = gltf->cameras_count;

	for (size_t n = 0; n < gltf->nodes_count; ++n)
	{
		cgltf_node* curr_node = gltf->nodes + n;

		uint32_t id = rand();
		int32_t res = find_entity_id(scene->entities, scene->entities_count, id);

		while (res != -1)
		{
			id = rand();
			res = find_entity_id(scene->entities, scene->entities_count, id);
		}

		TransformComponent tc = { 0 };

		scene->transform_components[n] = tc;

		Entity e = {
			.id = id,
		};

		strcpy(e.name, curr_node->name);

		scene->entities[n] = e;
	}

	cgltf_free(gltf);
	SDL_UnlockRWLock(rw_lock);

	BufferResource* br = BufferResource_create(scene->entities_count * sizeof(Entity), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, 0, "test");
	BufferResource_destroy(br);
}

void Scene_simulate(Scene* scene, const float delta_time, const size_t substeps, const size_t frame_number)
{
	float sdt = delta_time / substeps;

	for (size_t s = 0; s < substeps; ++s)
	{
	}

	SDL_CHECK(SDL_PushEvent(&events.SimulationFrameDone));
}

void Scene_play(Scene* scene, const size_t frame_number)
{
	SDL_CHECK(SDL_PushEvent(&events.PlayFrameDone));
}

void Scene_render(Scene* scene, SDL_RWLock* rw_lock)
{
	SDL_LockRWLockForReading(rw_lock);
	SDL_UnlockRWLock(rw_lock);
}

void Scene_destroy(Scene* scene)
{
	if (scene != NULL)
	{
		free(scene->entities);
		free(scene->transform_components);
		free(scene->mesh_renderer_components);
		free(scene->physics_components);
		free(scene->camera_components);

		free(scene);
	}
}

void Scene_begin_pass(Scene* scene)
{}

void Scene_end_pass(Scene* scene)
{}
