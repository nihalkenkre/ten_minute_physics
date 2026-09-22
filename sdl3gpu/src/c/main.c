#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "utils.h"
#include "app.h"

static SDL_Window* window = NULL;
static SDL_GPUDevice* device = NULL;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    window = SDL_CreateWindow("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE);
    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    App_create(window, device);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    App_event(event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        return SDL_APP_CONTINUE;

    App_iterate();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    App_destroy();

	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);

    SDL_Quit();
}
