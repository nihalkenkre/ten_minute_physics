#pragma once

#include <SDL3/SDL_events.h>

typedef struct Events
{
	SDL_Event FileOpen;
	SDL_Event Simulate;
	SDL_Event Play;
	SDL_Event Stop;
	SDL_Event FPSChanged;
	SDL_Event SubstepsChanged;
	SDL_Event SimulationFrameDone;
	SDL_Event PlayFrameDone;
} Events;

#ifdef __cplusplus
extern "C" {
#endif
	Events events;
#ifdef __cplusplus
}
#endif

void events_initialize();
