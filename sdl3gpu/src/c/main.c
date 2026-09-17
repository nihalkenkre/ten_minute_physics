#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "utils.h"
#include "app.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    App* app = SDL_malloc(sizeof(App));
    app->window = SDL_CreateWindow("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_CHECK(app->window);
    app->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    SDL_CHECK(app->device);

    SDL_CHECK(SDL_ClaimWindowForGPUDevice(app->device, app->window));

    *appstate = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    App* app = (App*)(appstate);
    App_event(app, event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    App* app = (App*)(appstate);

    if (SDL_GetWindowFlags(app->window) & SDL_WINDOW_MINIMIZED)
        return SDL_APP_CONTINUE;

    App_simulate(app);
    App_render(app);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    App* app = (App*)(appstate);

    SDL_ReleaseWindowFromGPUDevice(app->device, app->window);
    SDL_DestroyWindow(app->window);

    App_destroy(app);

    SDL_Quit();
}
