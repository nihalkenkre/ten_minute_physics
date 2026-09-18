#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "utils.h"
#include "app.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window* window = SDL_CreateWindow("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_CHECK(window);
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    SDL_CHECK(device);
    App* app = App_create(window, device);
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

    App_iterate(app);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    App* app = (App*)(appstate);

    App_destroy(app);

    SDL_Quit();
}
