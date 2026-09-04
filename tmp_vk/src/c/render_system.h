#pragma once

#include "scene.h"
#include "gui.h"

struct SDL_Window;

typedef struct RenderSystem
{
	VulkanInterface* vulkan_interface;
} RenderSystem;

RenderSystem* RenderSystem_create(SDL_Window* window);

void RenderSystem_render(RenderSystem* rs, GUI* gui, Scene* scene, SDL_RWLock* rw_lock);

void RenderSystem_recreate_swapchain(RenderSystem* rs);
void RenderSystem_recreate_depth_texture(RenderSystem* rs);

void RenderSystem_destroy(RenderSystem* rs);