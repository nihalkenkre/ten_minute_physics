#pragma once

#include <SDL3/SDL.h>

typedef struct _GUI
{
	SDL_Window* window;
	SDL_GPUDevice* device;
} GUI;

#ifdef __cplusplus
extern "C" {

#endif

	GUI* GUI_create(SDL_Window* window, SDL_GPUDevice* device);
	bool GUI_process_event(GUI* gui, SDL_Event* event);
	void GUI_render(GUI* gui, SDL_GPUCommandBuffer* command_buffer, const SDL_GPUColorTargetInfo* target_info);
	void GUI_destroy(GUI* gui);

#ifdef __cplusplus
}
#endif 
