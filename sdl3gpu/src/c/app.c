#include "app.h"
#include "utils.h"

#include <stdlib.h>


App* App_create(SDL_Window* window, SDL_GPUDevice* device)
{
    App* app = SDL_calloc(1, sizeof(App));

    app->window = window;
    app->device = device;

    return app;
}

void App_event(App* app, SDL_Event* event)
{
}

void App_iterate(App* app)
{
}

void App_simulate(App* app)
{
}

void App_render(App* app)
{
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
}

void App_destroy(App* app)
{
    if (app != NULL)
    {
        SDL_free(app);
    }
}
