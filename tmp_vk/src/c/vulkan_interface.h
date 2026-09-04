#pragma once

#include "vulkan_objects.h"

struct SDL_Window;

typedef struct VulkanInterface
{
	VkInstance instance;
	PhysicalDeviceData physical_device_data;
	Surface* surface;
	Device* device;
	Swapchain* swapchain;
	Allocator* allocator;
	QueueTasks* graphics_tasks;
	QueueTasks* transfer_tasks;
	QueueTasks* compute_tasks;
	uint8_t max_frames_in_flight;
} VulkanInterface;

VulkanInterface* VulkanInterface_create(SDL_Window* window);
void VulkanInterface_recreate_swapchain(VulkanInterface* vi);
void VulkanInterface_destroy(VulkanInterface* vi);