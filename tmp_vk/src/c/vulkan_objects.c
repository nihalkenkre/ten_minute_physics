#include "vulkan_objects.h"
#include "utils.h"

#include <SDL3/SDL_vulkan.h>

static VkInstance i = VK_NULL_HANDLE;
static VkDevice d = VK_NULL_HANDLE;
static VmaAllocator a = VK_NULL_HANDLE;

VkInstance Instance_create(const char* const* extensions, const uint32_t extensions_count, const uint32_t vk_api_version)
{
	VkResult result = VK_SUCCESS;

	const char* more_exts[] = {
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
#ifdef _DEBUG
		  VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

	size_t req_ext_count = _countof(more_exts) + extensions_count;
	char** req_ext_names = calloc(req_ext_count, sizeof(char*));
	size_t req_ext_idx = 0;

	for (size_t e = 0; e < extensions_count; ++e)
	{
		req_ext_names[req_ext_idx] = calloc(strlen(extensions[e]) + 1, sizeof(char));
		strcpy(req_ext_names[req_ext_idx], extensions[e]);
		++req_ext_idx;
	}

	for (size_t e = 0; e < _countof(more_exts); ++e)
	{
		req_ext_names[req_ext_idx] = calloc(strlen(more_exts[e]) + 1, sizeof(char));
		strcpy(req_ext_names[req_ext_idx], more_exts[e]);
		++req_ext_idx;
	}

	uint32_t properties_count = 0;
	VK_CHECK("enumerate instance extensions", vkEnumerateInstanceExtensionProperties(NULL, &properties_count, NULL));

	VkExtensionProperties* properties = calloc(properties_count, sizeof(VkExtensionProperties));
	VK_CHECK("enumerate instance extensions", vkEnumerateInstanceExtensionProperties(NULL, &properties_count, properties));

	for (size_t r = 0; r < req_ext_count; ++r)
	{
		if (is_extension_supported(properties, properties_count, req_ext_names[r]) == -1)
		{
			printf("Extension %s not supported by instance.\n", req_ext_names[r]);
		}
	}

	const VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Ten Minute Physics",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Ten Minute Physics",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk_api_version,
	};

	const VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = (uint32_t)(req_ext_count),
		.ppEnabledExtensionNames = (const char* const*)req_ext_names,
	};

	VK_CHECK("create instance", vkCreateInstance(&create_info, NULL, &i));

#ifdef  _DEBUG
	//vk_SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(i, "vkSetDebugUtilsObjectNameEXT"));
#endif //  _DEBUG

	for (size_t r = 0; r < req_ext_count; ++r)
	{
		free(req_ext_names[r]);
	}

	free(req_ext_names);
	free(properties);

	volkLoadInstance(i);

	return i;
}

void Instance_destroy()
{
	if (i != VK_NULL_HANDLE)
	{
		vkDestroyInstance(i, NULL);
	}
}

PhysicalDeviceData Instance_get_physical_device_data(const VkSurfaceKHR surface)
{
	uint32_t physical_device_count = 0;
	VK_CHECK("enumerate physical devices", vkEnumeratePhysicalDevices(i, &physical_device_count, NULL));

	VkPhysicalDevice* physical_devices = calloc(physical_device_count, sizeof(VkPhysicalDevice));
	VK_CHECK("enumerate physical devices", vkEnumeratePhysicalDevices(i, &physical_device_count, physical_devices));

	PhysicalDeviceData pdd = {
		.properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		},
		.memory_properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
		},
	};

	for (size_t p = 0; p < physical_device_count; ++p)
	{
		VkPhysicalDevice physical_device = physical_devices[p];
		vkGetPhysicalDeviceProperties2(physical_device, &pdd.properties);

		if (pdd.properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			uint32_t q_fly_cnt = 0;
			vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &q_fly_cnt, NULL);

			VkQueueFamilyProperties2* q_fly_props = calloc(q_fly_cnt, sizeof(VkQueueFamilyProperties2));

			for (size_t q = 0; q < q_fly_cnt; ++q)
			{
				q_fly_props[q].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
			}

			vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &q_fly_cnt, q_fly_props);

			// find graphics family
			for (uint32_t q = 0; q < q_fly_cnt; ++q)
			{
				VkBool32 is_supported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, q, surface, &is_supported);

				if (is_supported &&
					SDL_Vulkan_GetPresentationSupport(i, physical_device, q) &&
					q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					pdd.physical_device = physical_device;
					pdd.graphics_queue_family_index = q;
					pdd.transfer_queue_family_index = q;
					pdd.compute_queue_family_index = q;

					vkGetPhysicalDeviceMemoryProperties2(physical_device, &pdd.memory_properties);

					break;
				}
			}

			// Find transfer family
			for (uint32_t q = q_fly_cnt - 1; q >= 0; --q)
			{
				if ((q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					pdd.transfer_queue_family_index = q;
					break;
				}
			}


			if (pdd.transfer_queue_family_index == VK_QUEUE_FAMILY_IGNORED)
			{
				for (uint32_t q = q_fly_cnt - 1; q >= 0; --q)
				{
					if (q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
					{
						pdd.transfer_queue_family_index = q;
						break;
					}
				}
			}

			// Find compute family
			for (uint32_t q = q_fly_cnt - 1; q >= 0; --q)
			{
				if ((q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					pdd.compute_queue_family_index = q;
					break;
				}
			}

			if (pdd.compute_queue_family_index == VK_QUEUE_FAMILY_IGNORED)
			{
				for (uint32_t q = q_fly_cnt - 1; q >= 0; --q)
				{
					if (q_fly_props[q].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
					{
						pdd.compute_queue_family_index = q;
						break;
					}
				}
			}

			free(q_fly_props);
		}
	}

	return pdd;
}

Surface* Surface_create(SDL_Window* window)
{
	Surface* s = (Surface*)calloc(1, sizeof(Surface));
	s->capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

	SDL_CHECK(SDL_Vulkan_CreateSurface(window, i, NULL, &s->surface));

	return s;
}

void Surface_destroy(Surface* s)
{
	if (s != NULL && i != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(i, s->surface, NULL);

		free(s);
	}
}

void Surface_populate_surface_data(Surface* s, const VkPhysicalDevice physical_device)
{
	{
		uint32_t surf_forms_count = 0;
		VkPhysicalDeviceSurfaceInfo2KHR surface_info = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
			.surface = s->surface,
		};
		VK_CHECK("get surface formats", vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &surf_forms_count, NULL));

		VkSurfaceFormat2KHR* surf_forms = calloc(surf_forms_count, sizeof(VkSurfaceFormat2KHR));
		for (size_t sf = 0; sf < surf_forms_count; ++sf)
		{
			surf_forms[sf].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
		}

		VK_CHECK("get surface formats", vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &surf_forms_count, surf_forms));

		VkSurfaceFormat2KHR fmt = {
			.surfaceFormat = {
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			}
		};

		int32_t f = find_surface_format(surf_forms, surf_forms_count, fmt);

		if (f >= 0)
		{
			s->surface_format = fmt;
		}
		else
		{
			printf("Could not find surface format.\n");
		}

		free(surf_forms);
	}

	{
		uint32_t present_modes_count = 0;
		VK_CHECK("get present modes", vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, s->surface, &present_modes_count, NULL));

		VkPresentModeKHR* present_modes = calloc(present_modes_count, sizeof(VkPresentModeKHR));
		VK_CHECK("get surface formats", vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, s->surface, &present_modes_count, present_modes));

		s->present_mode = find_present_mode(present_modes, present_modes_count);

		free(present_modes);
	}

	const VkPhysicalDeviceSurfaceInfo2KHR surface_info = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
		.surface = s->surface,
	};

	VK_CHECK("get surface capabilitites", vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info, &s->capabilities));
}

Device* Device_create(PhysicalDeviceData pdd)
{
	const char* req_ext_names[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
	};

	uint32_t properties_count = 0;
	VK_CHECK("enumerate device extensions", vkEnumerateDeviceExtensionProperties(pdd.physical_device, NULL, &properties_count, NULL));

	VkExtensionProperties* properties = calloc(properties_count, sizeof(VkExtensionProperties));
	VK_CHECK("enumerate device extensions", vkEnumerateDeviceExtensionProperties(pdd.physical_device, NULL, &properties_count, properties));

	for (size_t r = 0; r < _countof(req_ext_names); ++r)
	{
		if (!is_extension_supported(properties, properties_count, req_ext_names[r]))
		{
			printf("Extension %s not supported by device.\n", req_ext_names[r]);
		}
	}

	// check for unified layouts
	if (is_extension_supported(properties, properties_count, VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME))
	{

	}
	
	//check for descriptor heap
	if (is_extension_supported(properties, properties_count, VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME))
	{

	}

	VkDeviceQueueCreateInfo* d_q_cis = calloc(3, sizeof(VkDeviceQueueCreateInfo));
	d_q_cis[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	d_q_cis[0].queueFamilyIndex = pdd.graphics_queue_family_index;
	d_q_cis[0].queueCount = 1;
	size_t d_q_ci_idx = 1;

	int32_t res = find_queue_family_index(d_q_cis, 3, pdd.transfer_queue_family_index);
	if (res >= 0)
	{
		++d_q_cis[res].queueCount;
	}
	else
	{
		d_q_cis[d_q_ci_idx].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		d_q_cis[d_q_ci_idx].queueFamilyIndex = pdd.transfer_queue_family_index;
		d_q_cis[d_q_ci_idx].queueCount = 1;
		++d_q_ci_idx;
	}

	res = find_queue_family_index(d_q_cis, 3, pdd.compute_queue_family_index);
	if (res >= 0)
	{
		++d_q_cis[res].queueCount;
	}
	else
	{
		d_q_cis[d_q_ci_idx].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		d_q_cis[d_q_ci_idx].queueFamilyIndex = pdd.compute_queue_family_index;
		d_q_cis[d_q_ci_idx].queueCount = 1;
		++d_q_ci_idx;
	}

	for (size_t idx = 0; idx < d_q_ci_idx; ++idx)
	{
		float* priorities = calloc(d_q_cis[idx].queueCount, sizeof(float));

		for (size_t qp = 0; qp < d_q_cis[idx].queueCount; ++qp)
		{
			priorities[qp] = 1.f;
		}

		d_q_cis[idx].pQueuePriorities = priorities;
	}

	VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR img_lyts_feats = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR,
	};

	VkPhysicalDeviceDescriptorHeapFeaturesEXT desc_heap_feats = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
		.pNext = &img_lyts_feats,
	};

	VkPhysicalDeviceDynamicRenderingFeatures dyn_rend_feats = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
		.pNext = &desc_heap_feats,
	};

	VkPhysicalDeviceSynchronization2Features sync2_feats = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
		.pNext = &dyn_rend_feats,
	};

	VkPhysicalDeviceVulkan11Features feats11 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = &sync2_feats,
	};

	VkPhysicalDeviceVulkan12Features feats12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &feats11,
	};

	VkPhysicalDeviceFeatures2 feats2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &feats12,
	};

	vkGetPhysicalDeviceFeatures2(pdd.physical_device, &feats2);

	const VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &feats2,
		.queueCreateInfoCount = (uint32_t)(d_q_ci_idx),
		.pQueueCreateInfos = d_q_cis,
		.enabledExtensionCount = _countof(req_ext_names),
		.ppEnabledExtensionNames = req_ext_names,
	};

	Device* dev = (Device*)(calloc(1, sizeof(Device)));

	VK_CHECK("create device", vkCreateDevice(pdd.physical_device, &create_info, NULL, &dev->device));
	d = dev->device;

	VkDeviceQueueInfo2 queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
	};

	for (size_t idx = 0; idx < d_q_ci_idx; ++idx)
	{
		for (uint32_t q = 0; q < d_q_cis[idx].queueCount; ++q)
		{
			if (d_q_cis[idx].queueFamilyIndex == pdd.graphics_queue_family_index && dev->graphics_queue == VK_NULL_HANDLE)
			{
				queue_info.queueFamilyIndex = pdd.graphics_queue_family_index;
				queue_info.queueIndex = q;
				vkGetDeviceQueue2(d, &queue_info, &dev->graphics_queue);
			}
			else if (d_q_cis[idx].queueFamilyIndex == pdd.transfer_queue_family_index && dev->transfer_queue == VK_NULL_HANDLE)
			{
				queue_info.queueFamilyIndex = pdd.transfer_queue_family_index;
				queue_info.queueIndex = q;
				vkGetDeviceQueue2(d, &queue_info, &dev->transfer_queue);
			}
			else if (d_q_cis[idx].queueFamilyIndex == pdd.compute_queue_family_index && dev->compute_queue == VK_NULL_HANDLE)
			{
				queue_info.queueFamilyIndex = pdd.compute_queue_family_index;
				queue_info.queueIndex = q;
				vkGetDeviceQueue2(d, &queue_info, &dev->compute_queue);
			}
		}
	}

#ifdef _DEBUG
	Utils_set_object_name(d, VK_OBJECT_TYPE_DEVICE, (uint64_t)(d), "device");
	Utils_set_object_name(d, VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)(pdd.physical_device), pdd.properties.properties.deviceName);
	Utils_set_object_name(d, VK_OBJECT_TYPE_QUEUE, (uint64_t)(dev->graphics_queue), "graphics queue");
	Utils_set_object_name(d, VK_OBJECT_TYPE_QUEUE, (uint64_t)(dev->transfer_queue), "transfer queue");
	Utils_set_object_name(d, VK_OBJECT_TYPE_QUEUE, (uint64_t)(dev->compute_queue), "compute queue");
#endif	// _DEBUG

	return dev;
}

void Device_destroy(Device* dev)
{
	if (dev != NULL && dev->device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(dev->device, NULL);

		free(dev);
	}
}

Swapchain* Swapchain_create(const Surface* surface_data, const uint32_t graphics_queue_family_index, const char* name)
{
	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface_data->surface,
		.minImageCount = surface_data->capabilities.surfaceCapabilities.minImageCount,
		.imageFormat = surface_data->surface_format.surfaceFormat.format,
		.imageColorSpace = surface_data->surface_format.surfaceFormat.colorSpace,
		.imageExtent = surface_data->capabilities.surfaceCapabilities.currentExtent,
		.imageArrayLayers = 1,
		.imageUsage = surface_data->capabilities.surfaceCapabilities.supportedUsageFlags,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &graphics_queue_family_index,
		.preTransform = surface_data->capabilities.surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = surface_data->present_mode,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	Swapchain* s = (Swapchain*)(calloc(1, sizeof(Swapchain)));

	VK_CHECK("create swapchain", vkCreateSwapchainKHR(d, &create_info, NULL, &s->swapchain));
	VK_CHECK("get swapchain images", vkGetSwapchainImagesKHR(d, s->swapchain, &s->images_count, NULL));

	s->images = calloc(s->images_count, sizeof(VkImage));
	VK_CHECK("get swapchain images", vkGetSwapchainImagesKHR(d, s->swapchain, &s->images_count, s->images));

	VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = surface_data->surface_format.surfaceFormat.format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1,
		},
	};

	s->image_views = calloc(s->images_count, sizeof(VkImageView));
	for (uint32_t i = 0; i < s->images_count; ++i)
	{
		image_view_create_info.image = s->images[i];
		VK_CHECK("create swapchain image view", vkCreateImageView(d, &image_view_create_info, NULL, &s->image_views[i]));
	}

#ifdef _DEBUG
	Utils_set_object_name(d, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)(s->swapchain), name);

	for (uint32_t i = 0; i < s->images_count; ++i)
	{
		char image_name[256] = "swapchain image ";
		char i_str[4];
		_itoa(i, i_str, 10);
		strcat(image_name, i_str);
		Utils_set_object_name(d, VK_OBJECT_TYPE_IMAGE, (uint64_t)(s->images[i]), image_name);

		char image_view_name[256] = "swapchain image view ";
		_itoa(i, i_str, 10);
		strcat(image_view_name, i_str);
		Utils_set_object_name(d, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)(s->image_views[i]), image_view_name);
	}
#endif
	return s;
}

void Swapchain_destroy(Swapchain* s)
{
	if (s != NULL && d != VK_NULL_HANDLE)
	{
		for (size_t iv = 0; iv < s->images_count; ++iv)
			vkDestroyImageView(d, s->image_views[iv], NULL);

		vkDestroySwapchainKHR(d, s->swapchain, NULL);

		free(s);
	}
}

QueueTasks* QueueTasks_create(const VkQueue queue, const uint32_t queue_family_index, const uint8_t max_frames_in_flight, const char* name)
{
	QueueTasks* qt = (QueueTasks*)(calloc(1, sizeof(QueueTasks)));
	qt->queue = queue;
	qt->queue_family_index = queue_family_index;
	qt->count = max_frames_in_flight;

	qt->command_buffers = calloc(max_frames_in_flight, sizeof(VkCommandBuffer));
	qt->semaphores = calloc(max_frames_in_flight, sizeof(VkSemaphore));
	qt->semaphore_values = calloc(max_frames_in_flight, sizeof(uint64_t));

	for (size_t v = 0; v < max_frames_in_flight; ++v)
	{
		qt->semaphore_values[v] = 1;
	}

	const VkSemaphoreTypeCreateInfo sem_type_ci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = 0,
	};

	const VkSemaphoreCreateInfo tl_sem_ci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &sem_type_ci,
	};

	VkSemaphoreSignalInfo sem_sig_inqtc = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
		.value = 1,
	};

	const VkCommandPoolCreateInfo cmd_pool_ci = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family_index,
	};

	VK_CHECK("create command pool", vkCreateCommandPool(d, &cmd_pool_ci, NULL, &qt->command_pool));

#ifdef _DEBUG
	Utils_set_object_name(d, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)(qt->command_pool), "");
#endif // _DEBUG

	const VkCommandBufferAllocateInfo cmd_buff_ai = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = qt->command_pool,
		.commandBufferCount = max_frames_in_flight,
	};

	VK_CHECK("allocator cmd buffs", vkAllocateCommandBuffers(d, &cmd_buff_ai, qt->command_buffers));

#ifdef _DEBUG
	for (size_t cb = 0; cb < qt->count; ++cb)
	{
		char obj_name[256];
		strcpy(obj_name, name);
		strcat(obj_name, " command buffer ");
		char i_str[4];
		_itoa((int)cb, i_str, 10);
		strcat(obj_name, i_str);
		Utils_set_object_name(d, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)(qt->command_buffers[cb]), obj_name);
	}
#endif // _DEBUG

	for (uint8_t fr = 0; fr < qt->count; ++fr)
	{
		VK_CHECK("create frame semaphore", vkCreateSemaphore(d, &tl_sem_ci, NULL, qt->semaphores + fr));
#ifdef _DEBUG
		char obj_name[256];
		strcpy(obj_name, name);
		strcat(obj_name, " semaphore ");
		char i_str[4];
		_itoa(fr, i_str, 10);
		strcat(obj_name, i_str);
		Utils_set_object_name(d, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)(qt->semaphores[fr]), obj_name);
#endif // _DEBUG
		sem_sig_inqtc.semaphore = qt->semaphores[fr];

		VK_CHECK("signal frame semaphore", vkSignalSemaphore(d, &sem_sig_inqtc));
	}

	return qt;
}

void QueueTasks_destroy(QueueTasks* qt)
{
	if (qt != NULL && d != VK_NULL_HANDLE)
	{
		for (size_t s = 0; s < qt->count; ++s)
		{
			vkDestroySemaphore(d, qt->semaphores[s], NULL);
		}

		vkDestroyCommandPool(d, qt->command_pool, NULL);

		free(qt->semaphores);
		free(qt->semaphore_values);
		free(qt->command_buffers);

		free(qt);
	}
}

void QueueTask_transfer_record_tasks(QueueTasks* qt, const uint8_t frame_in_flight)
{
	const VkSemaphoreWaitInfo wait_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = qt->semaphores + frame_in_flight,
		.pValues = qt->semaphore_values + frame_in_flight,
	};

	VK_CHECK("wait for transfer sem", vkWaitSemaphores(d, &wait_info, UINT64_MAX));

	const VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK("begin xfer cmd buff", vkBeginCommandBuffer(qt->command_buffers[frame_in_flight], &begin_info));
}

void QueueTask_transfer_submit_tasks(QueueTasks* qt, QueueTasks* graphics_wait, QueueTasks* compute_wait, const uint8_t frame_in_flight)
{
	VK_CHECK("end transfer command buffer", vkEndCommandBuffer(qt->command_buffers[frame_in_flight]));

	const VkSemaphoreSubmitInfo wait_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = graphics_wait->semaphores[frame_in_flight],
			.value = graphics_wait->semaphore_values[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = compute_wait->semaphores[frame_in_flight],
			.value = compute_wait->semaphore_values[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
		}
	};

	const VkCommandBufferSubmitInfo cmd_buff_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = qt->command_buffers[frame_in_flight],
		}
	};

	const VkSemaphoreSubmitInfo sig_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = qt->semaphores[frame_in_flight],
			.value = ++qt->semaphore_values[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		}
	};

	const VkSubmitInfo2 submit_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = _countof(wait_sem_infos),
			.pWaitSemaphoreInfos = wait_sem_infos,
			.commandBufferInfoCount = _countof(cmd_buff_infos),
			.pCommandBufferInfos = cmd_buff_infos,
			.signalSemaphoreInfoCount = _countof(sig_sem_infos),
			.pSignalSemaphoreInfos = sig_sem_infos,
		}
	};

	VK_CHECK("submit transfer tasks", vkQueueSubmit2(qt->queue, _countof(submit_infos), submit_infos, VK_NULL_HANDLE));
}

void QueueTasks_record_tasks(QueueTasks* qt, const uint8_t frame_in_flight)
{
	const VkSemaphoreWaitInfo wait_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = qt->semaphores + frame_in_flight,
		.pValues = qt->semaphore_values + frame_in_flight,
	};

	VK_CHECK("wait for graphics sem", vkWaitSemaphores(d, &wait_info, UINT64_MAX));

	const VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK("begin xfer cmd buff", vkBeginCommandBuffer(qt->command_buffers[frame_in_flight], &begin_info));
}

void QueueTasks_change_image_layout(
	QueueTasks* qt,
	const VkPipelineStageFlags2 src_stage_mask, const VkAccessFlags2 src_access_mask,
	const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 dst_access_mask,
	const VkImageLayout old_layout, const VkImageLayout new_layout,
	const uint32_t src_q_fly_idx, const uint32_t dst_q_fly_idx,
	const VkImageAspectFlags aspect_mask,
	const uint8_t frame_in_flight,
	const VkImage image
)
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

	vkCmdPipelineBarrier2(qt->command_buffers[frame_in_flight], &dep_info);
}

void QueueTasks_submit_tasks(
	QueueTasks* qt,
	QueueTasks* wait_qt_1, QueueTasks* wait_qt_2,
	const VkPipelineStageFlags2 wait_mask_1, const VkPipelineStageFlags2 wait_mask_2,
	const VkPipelineStageFlags2 signal_mask_qt,
	const uint8_t frame_in_flight
)
{
	VK_CHECK("end graphics command buffer", vkEndCommandBuffer(qt->command_buffers[frame_in_flight]));

	const VkSemaphoreSubmitInfo wait_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = wait_qt_1->semaphores[frame_in_flight],
			.value = wait_qt_1->semaphore_values[frame_in_flight],
			.stageMask = wait_mask_1
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = wait_qt_2->semaphores[frame_in_flight],
			.value = wait_qt_2->semaphore_values[frame_in_flight],
			.stageMask = wait_mask_2,
		}
	};

	const VkCommandBufferSubmitInfo cmd_buff_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = qt->command_buffers[frame_in_flight],
		}
	};

	uint64_t value = 0;
	VK_CHECK("get sem counter val", vkGetSemaphoreCounterValue(d, qt->semaphores[frame_in_flight], &value));

	const VkSemaphoreSubmitInfo sig_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = qt->semaphores[frame_in_flight],
			.value = ++qt->semaphore_values[frame_in_flight],
			.stageMask = signal_mask_qt,
		}
	};

	const VkSubmitInfo2 submit_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = _countof(wait_sem_infos),
			.pWaitSemaphoreInfos = wait_sem_infos,
			.commandBufferInfoCount = _countof(cmd_buff_infos),
			.pCommandBufferInfos = cmd_buff_infos,
			.signalSemaphoreInfoCount = _countof(sig_sem_infos),
			.pSignalSemaphoreInfos = sig_sem_infos,
		}
	};

	VK_CHECK("submit graphics tasks", vkQueueSubmit2(qt->queue, _countof(submit_infos), submit_infos, VK_NULL_HANDLE));
}

Allocator* Allocator_create(const VkPhysicalDevice physical_device, const uint32_t vk_api_version)
{
	const VmaAllocatorCreateInfo allocator_create_info = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physical_device,
		.device = d,
		.instance = i,
		.vulkanApiVersion = vk_api_version,
	};

	Allocator* al = (Allocator*)calloc(1, sizeof(Allocator));
	VK_CHECK("create vma allocator", vmaCreateAllocator(&allocator_create_info, &al->allocator));

	a = al->allocator;

	return al;
}

void Allocator_destroy(Allocator* al)
{
	if (al != NULL)
	{
		vmaDestroyAllocator(al->allocator);

		free(al);
	}
}

ImageResource* ImageResource_create(const VkExtent3D extent, const VkFormat format, const VkImageUsageFlags usage, const char* name)
{
	ImageResource* ir = (ImageResource*)(calloc(1, sizeof(ImageResource)));

	const VkImageCreateInfo ci = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
	};

	const VmaAllocationCreateInfo alloc_ci = {
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	VK_CHECK("create image", vmaCreateImage(a, &ci, &alloc_ci, &ir->image, &ir->allocation, &ir->allocation_info.allocationInfo));

	VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT)
	{
		aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	const VkImageViewCreateInfo iv_ci = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = ir->image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = aspect_mask,
			.levelCount = 1,
			.layerCount = 1,
		},
	};

	VK_CHECK("create image view", vkCreateImageView(d, &iv_ci, NULL, &ir->image_view));

	const VkSamplerCreateInfo s_ci = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	};

	VK_CHECK("create sampler", vkCreateSampler(d, &s_ci, NULL, &ir->sampler));

#ifdef _DEBUG
	char image_name[256];
	strcpy(image_name, name);
	strcat(image_name, " image");
	Utils_set_object_name(d, VK_OBJECT_TYPE_IMAGE, (uint64_t)(ir->image), image_name);

	char image_view_name[256];
	strcpy(image_view_name, name);
	strcat(image_view_name, " image view");
	Utils_set_object_name(d, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)(ir->image_view), image_view_name);

	char sampler_name[256];
	strcpy(sampler_name, name);
	strcat(sampler_name, " sampler");
	Utils_set_object_name(d, VK_OBJECT_TYPE_SAMPLER, (uint64_t)(ir->sampler), sampler_name);
#endif	// _DEBUG

	return ir;
}

void ImageResource_destroy(ImageResource* ir)
{
	if (ir != NULL)
	{
		vmaDestroyImage(a, ir->image, ir->allocation);
		vkDestroyImageView(d, ir->image_view, NULL);
		vkDestroySampler(d, ir->sampler, NULL);

		free(ir);
	}
}

BufferResource* BufferResource_create(const size_t size, const VkBufferUsageFlags usage, const VmaAllocationCreateFlags vma_alloc_flags, const size_t min_alignment, const char* name)
{
	BufferResource* br = calloc(1, sizeof(BufferResource));

	const VkBufferCreateInfo ci = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
	};

	const VmaAllocationCreateInfo alloc_ci = {
		.flags = vma_alloc_flags,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	VK_CHECK("create buffer", vmaCreateBufferWithAlignment(a, &ci, &alloc_ci, min_alignment, &br->buffer, &br->allocation, &br->allocation_info.allocationInfo));

	return br;
}

void BufferResource_destroy(BufferResource* br)
{
	if (br != NULL)
	{
		vmaDestroyBuffer(a, br->buffer, br->allocation);

		free(br);
	}
}
