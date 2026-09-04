#include "utils.h"
#include "entity.h"

#include <string.h>
#include <cgltf.h>

mat4s Utils_get_transform_for_gltf_node(const cgltf_node* node)
{
	mat4s xform = glms_mat4_identity();

	if (node == NULL)
		return xform;

	if (node->has_matrix)
	{
		xform = glms_mat4_make(node->matrix);
	}
	else
	{
		if (node->has_translation)
		{
			xform = glms_translate(xform, glms_vec3_make(node->translation));
		}

		if (node->has_rotation)
		{
			versors rot_quat = glms_quat_make(node->rotation);
			xform = glms_rotate(xform, glms_quat_angle(rot_quat), glms_quat_axis(rot_quat));
		}

		if (node->has_scale)
		{
			xform = glms_scale(xform, glms_vec3_make(node->scale));
		}
	}

	return xform;
}

#ifdef _DEBUG
//PFN_vkSetDebugUtilsObjectNameEXT vk_SetDebugUtilsObjectNameEXT = NULL;
//VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
//{
	//return vk_SetDebugUtilsObjectNameEXT(device, pNameInfo);
//}

void Utils_set_object_name(const VkDevice device, const VkObjectType type, const uint64_t handle, const char* name)
{
	const VkDebugUtilsObjectNameInfoEXT name_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectType = type,
		.objectHandle = handle,
		.pObjectName = name,
	};

	VK_CHECK("setting name", vkSetDebugUtilsObjectNameEXT(device, &name_info));
}
#endif

void Utils_change_image_layout(
	const VkCommandBuffer cmd_buff,
	const VkPipelineStageFlags2 src_stage_mask, const VkAccessFlags2 src_access_mask,
	const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 dst_access_mask,
	const VkImageLayout old_layout, const VkImageLayout new_layout,
	const uint32_t src_q_fly_idx, const uint32_t dst_q_fly_idx,
	const VkImageAspectFlags aspect_mask,
	const VkImage image)
{
	const VkImageMemoryBarrier2 img_mem_barr = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = src_stage_mask,
		.srcAccessMask = src_access_mask,
		.dstStageMask = dst_stage_mask,
		.dstAccessMask = dst_access_mask,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = src_q_fly_idx,
		.dstQueueFamilyIndex = dst_q_fly_idx,
		.image = image,
		.subresourceRange = {
			.aspectMask = aspect_mask,
			.levelCount = 1,
			.layerCount = 1,
		},
	};

	const VkDependencyInfo dep_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &img_mem_barr,
	};

	vkCmdPipelineBarrier2(cmd_buff, &dep_info);
}

bool is_extension_supported(VkExtensionProperties* arr, const size_t arr_size, const char* str)
{
	for (size_t i = 0; i < arr_size; ++i)
	{
		if (strcmp(arr[i].extensionName, str) == 0)
		{
			return true;
		}
	}

	return false;
}

int32_t find_entity_id(const Entity* arr, const size_t arr_size, const int i)
{
	for (size_t _i = 0; _i < arr_size; ++_i)
	{
		if (arr[_i].id == i)
		{
			return (int32_t)_i;
		}
	}

	return -1;

}

int32_t find_surface_format(const VkSurfaceFormat2KHR* arr, const size_t arr_size, const VkSurfaceFormat2KHR fmt)
{
	for (size_t i = 0; i < arr_size; ++i)
	{
		if (arr[i].surfaceFormat.format == fmt.surfaceFormat.format && arr[i].surfaceFormat.colorSpace == fmt.surfaceFormat.colorSpace)
		{
			return (int32_t)i;
		}
	}

	return -1;
}

int32_t find_queue_family_index(const VkDeviceQueueCreateInfo* arr, const size_t arr_size, const uint32_t queue_family_index)
{
	for (size_t i = 0; i < arr_size; ++i)
	{
		if (arr[i].queueFamilyIndex == queue_family_index)
		{
			return (int32_t)i;
		}
	}

	return -1;
}

VkPresentModeKHR find_present_mode(const VkPresentModeKHR* arr, const size_t arr_size)
{
	for (size_t i = 0; i < arr_size; ++i)
	{
		if (arr[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
