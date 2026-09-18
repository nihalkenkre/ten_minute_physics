#include "app.h"
#include "utils.h"
#include "gui.h"

#include <stdlib.h>
#include <stdio.h>


App* App_create(SDL_Window* window, SDL_GPUDevice* device)
{
	App* app = SDL_calloc(1, sizeof(App));

    SDL_CHECK(SDL_ClaimWindowForGPUDevice(device, window));
    SDL_CHECK(SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX));

	app->window = window;
	app->device = device;
	app->gui = GUI_create(window, device);

	return app;
}

void App_event(App* app, SDL_Event* event)
{
	if (GUI_process_event(app->gui, event)) return;
}

void App_simulate(App* app)
{
}

void App_render(App* app)
{
	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app->device);
	SDL_CHECK(command_buffer);

	SDL_GPUTexture* sc_image = NULL;
	SDL_CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app->window, &sc_image, NULL, NULL));

	const SDL_GPUColorTargetInfo target_info = {
		.texture = sc_image,
		.cycle = true,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.clear_color = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f}
	};

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, NULL);
	SDL_EndGPURenderPass(render_pass);

	GUI_render(app->gui, command_buffer, &target_info);

	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void App_iterate(App* app)
{
	App_simulate(app);
	App_render(app);
}

void App_destroy(App* app)
{
	if (app != NULL)
	{
		GUI_destroy(app->gui);
		SDL_CHECK(SDL_WaitForGPUIdle(app->device));

		SDL_ReleaseWindowFromGPUDevice(app->device, app->window);
		SDL_DestroyGPUDevice(app->device);
		SDL_DestroyWindow(app->window);

		SDL_free(app);
	}
}
