#pragma once

#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

	void GUI_create(SDL_Window* window, SDL_GPUDevice* device);
	bool GUI_process_event(SDL_Event* event);
	void GUI_render(SDL_GPUCommandBuffer* command_buffer, const SDL_GPUColorTargetInfo* target_info);
	void GUI_destroy();

#ifdef __cplusplus
}
#endif 
