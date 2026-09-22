#include "app.h"
#include "utils.h"
#include "gui.h"
#include "events.h"
#include "scene.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct _App {
    SDL_Window* window;
    SDL_GPUDevice* device;
} App;

static App app;

void App_create(SDL_Window* window, SDL_GPUDevice* device)
{
    SDL_CHECK(SDL_ClaimWindowForGPUDevice(device, window));
    SDL_CHECK(SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX));

	app.window = window;
	app.device = device;
	GUI_create(window, device);
}

void App_event(SDL_Event* event)
{
	bool imgui_want_focus = GUI_process_event(event);

	if (event->type == events.FileOpen.type) 
	{
		Scene_create(event->user.data1);
	}
}

void App_simulate()
{
}

void App_render()
{
	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app.device);
	SDL_CHECK(command_buffer);

	SDL_GPUTexture* sc_image = NULL;
	SDL_CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app.window, &sc_image, NULL, NULL));

	const SDL_GPUColorTargetInfo target_info = {
		.texture = sc_image,
		.cycle = true,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.clear_color = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f}
	};

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, NULL);
	SDL_EndGPURenderPass(render_pass);

	GUI_render(command_buffer, &target_info);

	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void App_iterate()
{
	App_simulate();
	App_render();
}

void App_destroy()
{
	SDL_CHECK(SDL_WaitForGPUIdle(app.device));

	GUI_destroy();
	Scene_destroy();
}
