#include "vulkan_interface.h"
#include "vulkan_objects.h"
#include "utils.h"

#include <SDL3/SDL_vulkan.h>

VulkanInterface* VulkanInterface_create(SDL_Window* window)
{
	VK_CHECK("volk initialize", volkInitialize());

	VulkanInterface* vi = (VulkanInterface*)(calloc(1, sizeof(VulkanInterface)));

	uint32_t vk_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0);
	Uint32 count = 0;
	const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&count);
	vi->instance = Instance_create(exts, count, vk_api_version);
	vi->surface = Surface_create(window);
	vi->physical_device_data = Instance_get_physical_device_data(vi->surface->surface);
	Surface_populate_surface_data(vi->surface, vi->physical_device_data.physical_device);
	vi->device = Device_create(vi->physical_device_data);
	vi->swapchain = Swapchain_create(vi->surface, vi->physical_device_data.graphics_queue_family_index, "swapchain");
	vi->max_frames_in_flight = vi->swapchain->images_count;
	vi->allocator = Allocator_create(vi->physical_device_data.physical_device, vk_api_version);
	vi->graphics_tasks = QueueTasks_create(vi->device->graphics_queue, vi->physical_device_data.graphics_queue_family_index, vi->max_frames_in_flight, "graphics tasks");
	vi->transfer_tasks = QueueTasks_create(vi->device->transfer_queue, vi->physical_device_data.transfer_queue_family_index, vi->max_frames_in_flight, "transfer tasks");
	vi->compute_tasks = QueueTasks_create(vi->device->compute_queue, vi->physical_device_data.compute_queue_family_index, vi->max_frames_in_flight, "compute tasks");

	return vi;
}

void VulkanInterface_recreate_swapchain(VulkanInterface* vi)
{
	Swapchain_destroy(vi->swapchain);
	Surface_populate_surface_data(vi->surface, vi->physical_device_data.physical_device);
	vi->swapchain = Swapchain_create(vi->surface, vi->physical_device_data.graphics_queue_family_index, "swapchain");
}

void VulkanInterface_destroy(VulkanInterface* vi)
{
	if (vi != NULL)
	{
		QueueTasks_destroy(vi->compute_tasks);
		QueueTasks_destroy(vi->transfer_tasks);
		QueueTasks_destroy(vi->graphics_tasks);
		Allocator_destroy(vi->allocator);
		Swapchain_destroy(vi->swapchain);
		Device_destroy(vi->device);
		Surface_destroy(vi->surface);
		Instance_destroy();

		free(vi);
	}
}
