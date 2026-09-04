#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <stdlib.h>

#define SDL_CHECK(result)               \
	if (!result) \
	{                      \
	    SDL_Log("%s\n", SDL_GetError());    \
    }

typedef struct App
{
    SDL_Window* window;
    SDL_GPUDevice* device;
} App;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    App* app = SDL_malloc(sizeof(App));
    app->window = SDL_CreateWindow("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_CHECK(app->window);
    app->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    SDL_CHECK(app->device);

    *appstate = app;

    SDL_CHECK(SDL_ClaimWindowForGPUDevice(app->device, app->window));

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    App* app = (App*)(appstate);

    SDL_GPUCommandBuffer* cmd_buff = SDL_AcquireGPUCommandBuffer(app->device);
    SDL_CHECK(cmd_buff);

    SDL_GPUTexture* sc_image = NULL;
    SDL_CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buff, app->window, &sc_image, NULL, NULL));

    const SDL_GPUColorTargetInfo target_info = {
        .texture = sc_image,
        .cycle = true,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .clear_color = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f}
    };

    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(cmd_buff, &target_info, 1, NULL);
    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(cmd_buff);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    App* app = (App*)(appstate);

    SDL_ReleaseWindowFromGPUDevice(app->device, app->window);
    SDL_DestroyWindow(app->window);

    SDL_free(app);

    SDL_Quit();
}
