#pragma once

#include <SDL3/SDL.h>

typedef enum TimelineMode
{
	TIMELINE_MODE_STOPPED,
	TIMELINE_MODE_SIMULATING,
	TIMELINE_MODE_PLAYING,
} TimelineMode;

typedef struct _App
{
	SDL_Window* window;
} App;

App* App_create(SDL_Window* window);
void App_process_event(App* app, SDL_Event* event);
void App_iterate(App* app);
void App_destroy(App* app);
