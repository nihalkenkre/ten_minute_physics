#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <SDL3/SDL.h>

typedef struct PhysicalDeviceData
{
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties2KHR properties;
	VkPhysicalDeviceMemoryProperties2KHR memory_properties;
	uint32_t graphics_queue_family_index;
	uint32_t compute_queue_family_index;
	uint32_t transfer_queue_family_index;
} PhysicalDeviceData;

VkInstance Instance_create(const char* const* extensions, const uint32_t extensions_count, const uint32_t vk_api_version);
void Instance_destroy();

PhysicalDeviceData Instance_get_physical_device_data(const VkSurfaceKHR surface);

typedef struct Surface
{
	VkSurfaceKHR surface;
	VkPresentModeKHR present_mode;
	VkSurfaceFormat2KHR surface_format;
	VkSurfaceCapabilities2KHR capabilities;
} Surface;

Surface* Surface_create(SDL_Window* window);
void Surface_destroy(Surface* s);
void Surface_populate_surface_data(Surface* s, const VkPhysicalDevice physical_device);

typedef struct Device
{
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue transfer_queue;
	VkQueue compute_queue;
}Device;

Device* Device_create(PhysicalDeviceData pdd);
void Device_destroy(Device* d);

typedef struct Swapchain
{
	VkSwapchainKHR swapchain;
	VkImage* images;
	VkImageView* image_views;
	uint32_t images_count;
} Swapchain;

Swapchain* Swapchain_create(const Surface* surface_data, const uint32_t graphics_queue_family_index, const char* name);
void Swapchain_destroy(Swapchain* s);

typedef struct QueueTasks
{
	VkCommandPool command_pool;
	VkCommandBuffer* command_buffers;
	VkSemaphore* semaphores;
	uint64_t* semaphore_values;
	size_t count;
	VkQueue queue;
	uint32_t queue_family_index;
}QueueTasks;

QueueTasks* QueueTasks_create(const VkQueue queue, const uint32_t queue_family_index, const uint8_t max_frames_in_flight, const char* name);
void QueueTasks_destroy(QueueTasks* qt);

void QueueTasks_record_tasks(QueueTasks* qt, const uint8_t frame_in_flight);
void QueueTasks_change_image_layout(
	QueueTasks* qt,
	const VkPipelineStageFlags2 src_stage_mask, const VkAccessFlags2 src_access_mask,
	const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 dst_access_mask,
	const VkImageLayout old_layout, const VkImageLayout new_layout,
	const uint32_t src_q_fly_idx, const uint32_t dst_q_fly_idx,
	const VkImageAspectFlags aspect_mask,
	const uint8_t frame_in_flight,
	const VkImage image
);
void QueueTasks_submit_tasks(
	QueueTasks* qt,
	QueueTasks* wait_qt_1, QueueTasks* wait_qt_2,
	const VkPipelineStageFlags2 wait_mask_1, const VkPipelineStageFlags2 wait_mask_2,
	const VkPipelineStageFlags2 signa_mask_qt,
	const uint8_t frame_in_flight
);

typedef struct Allocator
{
	VmaAllocator allocator;
} Allocator;

Allocator* Allocator_create(const VkPhysicalDevice physical_device, const uint32_t vk_api_version);
void Allocator_destroy(Allocator* allocator);

typedef struct ImageResource
{
	VkImage image;
	VkImageView image_view;
	VkSampler sampler;

	VmaAllocation allocation;
	VmaAllocationInfo2 allocation_info;
} ImageResource;

ImageResource* ImageResource_create(const VkExtent3D extent, const VkFormat format, const VkImageUsageFlags usage, const char* name);
void ImageResource_destroy(ImageResource* ir);

typedef struct BufferResource
{
	VkBuffer buffer;

	VkDeviceAddress address;
	VkDeviceOrHostAddressKHR device_or_host_address;
	VkDeviceOrHostAddressConstKHR device_or_host_address_const;
	VmaAllocation allocation;
	VmaAllocationInfo2 allocation_info;
} BufferResource;

BufferResource* BufferResource_create(const size_t size, const VkBufferUsageFlags usage, const VmaAllocationCreateFlags vma_alloc_flags, const size_t min_alignment, const char* name);
void BufferResource_destroy(BufferResource* br);
