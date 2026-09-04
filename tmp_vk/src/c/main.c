#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include "app.h"
#include "utils.h"
#include "events.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	SDL_CHECK(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window* window = SDL_CreateWindow("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

	if (window == NULL)
	{
		SDL_Log("%s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_CHECK(SDL_ShowWindow(window));

	App* app = App_create(window);
	*appstate = app;

	events_initialize();
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	App* app = (App*)(appstate);

	if (SDL_GetWindowFlags(app->window) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED))
	{
		return SDL_APP_CONTINUE;
	}

	if (event->type == SDL_EVENT_QUIT)
	{
		return SDL_APP_SUCCESS;
	}

	App_process_event(app, event);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	App* app = (App*)(appstate);

	if (SDL_GetWindowFlags(app->window) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED))
	{
		return SDL_APP_CONTINUE;
	}

	App_iterate(app);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	App* app = (App*)(appstate);

	App_destroy(app);
}