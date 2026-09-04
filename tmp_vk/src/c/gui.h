#pragma once

#include <SDL3/SDL.h>
#include "vulkan_interface.h"

struct VulkanInterface;

typedef struct GUI {
	int32_t substeps;
	int32_t frame_number;
	int32_t frame_start;
	int32_t frame_end;
} GUI;

#ifdef __cplusplus
extern "C" {
#endif
	GUI* GUI_create(SDL_Window* window, const VulkanInterface* vulkan_interface);
	void GUI_process_event(GUI* gui, SDL_Event* event);
	void GUI_render(GUI* gui, const VkCommandBuffer command_buffer);
	void GUI_increment_frame(GUI* gui);
	void GUI_destroy(GUI* gui, const VkDevice device);
#ifdef __cplusplus
}
#endif