#include "events.h"
#include "utils.h"

Events events = {};

void Events_Initialize()
{
	bool sdl_result = true;
	size_t events_count = 0;
	for (
		uint8_t* e = (uint8_t*)(&events);
		e < (uint8_t*)(&events) + sizeof(events);
		e += sizeof(SDL_Event))
	{
		++events_count;
	}

	uint32_t id = SDL_RegisterEvents((int)(events_count));
	SDL_CHECK(id);

	for (
		uint8_t* e = (uint8_t*)(&events);
		e < (uint8_t*)(&events) + sizeof(events);
		e += sizeof(SDL_Event))
	{
		(SDL_Event*)(e)->type = id++;
	}
}