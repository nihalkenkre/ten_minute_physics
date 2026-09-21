#pragma once

#include <SDL3/SDL.h>

typedef struct _Events
{
	SDL_Event FileOpen;
	SDL_Event StartRender;
	SDL_Event StopRender;

	SDL_Event RenderDimsChanged;

	SDL_Event RenderStarted;
	SDL_Event RenderStopped;

	SDL_Event RenderSampleDone;
	SDL_Event ReloadShaders;
} Events;

#ifdef __cplusplus
extern "C" {
#endif

	Events events;

#ifdef __cplusplus
}
#endif

void Events_Initialize();