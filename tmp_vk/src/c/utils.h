#pragma once

#include <volk.h>
#include <cgltf.h>
#include <cglm/struct.h>
#include "entity.h"

#define SDL_CHECK(result)               \
	if (!result) \
	{                      \
	    SDL_Log("%s\n", SDL_GetError());    \
    }

#define VK_CHECK(action, result)              \
    if (result != VK_SUCCESS)               \
    {                                       \
        printf("%s %d\n", action, result);      \
    }

struct cgltf_node;

mat4s Utils_get_transform_for_gltf_node(const cgltf_node* node);
void Utils_change_image_layout(
	const VkCommandBuffer cmd_buff,
	const VkPipelineStageFlags2 src_stage_mask, const VkAccessFlags2 src_access_mask,
	const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 dst_access_mask,
	const VkImageLayout old_layout, const VkImageLayout new_layout,
	const uint32_t src_q_fly_idx, const uint32_t dst_q_fly_idx,
	const VkImageAspectFlags aspect_mask,
	const VkImage image);

bool is_extension_supported(VkExtensionProperties* arr, const size_t arr_size, const char* str);
int32_t find_entity_id(const Entity* arr, const size_t arr_size, const int i);
int32_t find_surface_format(const VkSurfaceFormat2KHR* arr, const size_t arr_size, const VkSurfaceFormat2KHR fmt);
int32_t find_queue_family_index(const VkDeviceQueueCreateInfo* arr, const size_t arr_size, const uint32_t queue_family_index);
VkPresentModeKHR find_present_mode(const VkPresentModeKHR* arr, const size_t arr_size);

#ifdef _DEBUG
extern PFN_vkSetDebugUtilsObjectNameEXT vk_SetDebugUtilsObjectNameEXT;
void Utils_set_object_name(const VkDevice device, const VkObjectType type, const uint64_t handle, const char* name);
#endif // _DEBUG