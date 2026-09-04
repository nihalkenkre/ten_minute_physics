#include "render_system.h"
#include "vulkan_interface.h"
#include "utils.h"

static VkSemaphore* acquire_signal_semaphores;
static VkSemaphore* present_wait_semaphores;

static ImageResource* depth_image_resource = NULL;
static VkDescriptorPool imgui_descriptor_pool = VK_NULL_HANDLE;
static uint32_t sc_image_index = 0;
static uint8_t frame_in_flight = 0;

void create_acq_prsnt_sems(VulkanInterface* vulkan_interface)
{
	VkDevice device = vulkan_interface->device->device;
	uint8_t max_frames_in_flight = vulkan_interface->max_frames_in_flight;
	acquire_signal_semaphores = calloc(max_frames_in_flight, sizeof(VkSemaphore));
	present_wait_semaphores = calloc(max_frames_in_flight, sizeof(VkSemaphore));

	const VkSemaphoreCreateInfo bin_sem_ci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	for (uint8_t fr = 0; fr < max_frames_in_flight; ++fr)
	{
		VK_CHECK("create acq sig semaphore", vkCreateSemaphore(device, &bin_sem_ci, NULL, acquire_signal_semaphores + fr));
		VK_CHECK("create present wait semaphore", vkCreateSemaphore(device, &bin_sem_ci, NULL, present_wait_semaphores + fr));

#ifdef _DEBUG
		char i_str[4];
		_itoa(fr, i_str, 10);
		char acq_sig_sem_name[256] = "acq sig sem ";
		strcat(acq_sig_sem_name, i_str);
		Utils_set_object_name(device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)(acquire_signal_semaphores[fr]), acq_sig_sem_name);

		char prsnt_wait_sem_name[256] = "prsnt wait sem ";
		strcat(prsnt_wait_sem_name, i_str);
		Utils_set_object_name(device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)(present_wait_semaphores[fr]), prsnt_wait_sem_name);
#endif // _DEBUG
	}
}

void destroy_acq_prsnt_sems(RenderSystem* rs)
{
	if (rs != NULL)
	{
		VkDevice device = rs->vulkan_interface->device->device;

		for (uint8_t fr = 0; fr < rs->vulkan_interface->max_frames_in_flight; ++fr)
		{
			vkDestroySemaphore(device, acquire_signal_semaphores[fr], NULL);
			vkDestroySemaphore(device, present_wait_semaphores[fr], NULL);
		}
	}
}

RenderSystem* RenderSystem_create(SDL_Window* window)
{
	RenderSystem* rs = (RenderSystem*)(calloc(1, sizeof(RenderSystem)));
	rs->vulkan_interface = VulkanInterface_create(window);

	RenderSystem_recreate_depth_texture(rs);
	create_acq_prsnt_sems(rs->vulkan_interface);

	return rs;
}

void begin_frame(RenderSystem* rs)
{
	VkDevice device = rs->vulkan_interface->device->device;

	const VkSemaphore wait_semaphores[] = {
		rs->vulkan_interface->graphics_tasks->semaphores[frame_in_flight],
	};

	const uint64_t wait_semaphore_values[] = {
		rs->vulkan_interface->graphics_tasks->semaphore_values[frame_in_flight],
	};

	const VkSemaphoreWaitInfo wait_info = {
	  .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
	  .semaphoreCount = _countof(wait_semaphores),
	  .pSemaphores = wait_semaphores,
	  .pValues = wait_semaphore_values,
	};

	VK_CHECK("wait acq img", vkWaitSemaphores(device, &wait_info, UINT64_MAX));

	const VkAcquireNextImageInfoKHR acq_info = {
		.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
		.swapchain = rs->vulkan_interface->swapchain->swapchain,
		.timeout = UINT64_MAX,
		.semaphore = acquire_signal_semaphores[frame_in_flight],
		.deviceMask = 0x1,
	};

	uint32_t img_idx = 0;
	VK_CHECK("acq img idx", vkAcquireNextImage2KHR(device, &acq_info, &img_idx));

	VkImage sc_image = rs->vulkan_interface->swapchain->images[img_idx];
	VkImageView sc_image_view = rs->vulkan_interface->swapchain->image_views[img_idx];

	VkRenderingAttachmentInfo col_attachs[] = {
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = sc_image_view,
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.color = {
					.float32 = {
						0.1f,
						0.1f,
						0.1f,
						1.0f,
					},
				},
			},
		},
	};

	const VkRenderingAttachmentInfo depth_attachment_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = depth_image_resource->image_view,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = {
			.depthStencil = {
				.depth = 1.f,
			},
		},
	};

	VkExtent2D extent = rs->vulkan_interface->surface->capabilities.surfaceCapabilities.currentExtent;

	const VkRenderingInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.extent = extent,
		},
		.layerCount = 1,
		.colorAttachmentCount = _countof(col_attachs),
		.pColorAttachments = col_attachs,
		.pDepthAttachment = &depth_attachment_info,
	};

	const VkViewport viewports[] = {
		{
			.width = (float)(extent.width),
			.height = (float)(extent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f,
		},
	};

	const VkRect2D scissors[] = {
		{
			.extent = extent,
		},
	};

	VkCommandBuffer command_buffer = rs->vulkan_interface->graphics_tasks->command_buffers[frame_in_flight];
	const VkCommandBufferBeginInfo cmd_buff_bi = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK("cmd buff begin", vkBeginCommandBuffer(command_buffer, &cmd_buff_bi));
	Utils_change_image_layout(
		command_buffer,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		VK_IMAGE_ASPECT_COLOR_BIT, sc_image
	);

	vkCmdBeginRendering(command_buffer, &rendering_info);

	vkCmdSetScissor(command_buffer, 0, _countof(scissors), scissors);
	vkCmdSetViewport(command_buffer, 0, _countof(viewports), viewports);

	sc_image_index = img_idx;
}

void end_frame(const RenderSystem* rs)
{
	VkImage sc_image = rs->vulkan_interface->swapchain->images[sc_image_index];
	VkCommandBuffer command_buffer = rs->vulkan_interface->graphics_tasks->command_buffers[frame_in_flight];

	const VkSemaphoreSubmitInfo wait_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = acquire_signal_semaphores[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = rs->vulkan_interface->transfer_tasks->semaphores[frame_in_flight],
			.value = rs->vulkan_interface->transfer_tasks->semaphore_values[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = rs->vulkan_interface->compute_tasks->semaphores[frame_in_flight],
			.value = rs->vulkan_interface->compute_tasks->semaphore_values[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		}
	};

	const VkCommandBufferSubmitInfo cmd_buff_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = command_buffer,
		},
	};

	const VkSemaphoreSubmitInfo sig_sem_infos[] = {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = present_wait_semaphores[frame_in_flight],
			.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = rs->vulkan_interface->graphics_tasks->semaphores[frame_in_flight],
			.value = ++*(rs->vulkan_interface->graphics_tasks->semaphore_values + frame_in_flight),
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
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
		},
	};

	VkSwapchainKHR sc = rs->vulkan_interface->swapchain->swapchain;

	const VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = present_wait_semaphores + frame_in_flight,
		.swapchainCount = 1,
		.pSwapchains = &sc,
		.pImageIndices = &sc_image_index,
	};

	VkQueue graphics_queue = rs->vulkan_interface->device->graphics_queue;

	vkCmdEndRendering(command_buffer);
	Utils_change_image_layout(
		command_buffer,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		VK_IMAGE_ASPECT_COLOR_BIT,
		sc_image
	);
	VK_CHECK("end command buffer", vkEndCommandBuffer(command_buffer));
	VK_CHECK("gfx q submit", vkQueueSubmit2(graphics_queue, _countof(submit_infos), submit_infos, VK_NULL_HANDLE));
	VK_CHECK("gfx q present", vkQueuePresentKHR(graphics_queue, &present_info));
}

void RenderSystem_next_frame(RenderSystem* rs)
{
	frame_in_flight = (frame_in_flight + 1) % rs->vulkan_interface->max_frames_in_flight;
}

void RenderSystem_render(RenderSystem* rs, GUI* gui, Scene* scene, SDL_RWLock* rw_lock)
{
	begin_frame(rs);

	Scene_render(scene, rw_lock);
	GUI_render(gui, rs->vulkan_interface->graphics_tasks->command_buffers[frame_in_flight]);

	end_frame(rs);

	RenderSystem_next_frame(rs);
}

void RenderSystem_recreate_swapchain(RenderSystem* rs)
{
	VulkanInterface_recreate_swapchain(rs->vulkan_interface);
}

void RenderSystem_recreate_depth_texture(RenderSystem* rs)
{
	ImageResource_destroy(depth_image_resource);
	const VkExtent3D extent = {
		rs->vulkan_interface->surface->capabilities.surfaceCapabilities.currentExtent.width,
		rs->vulkan_interface->surface->capabilities.surfaceCapabilities.currentExtent.height,
		1
	};

	uint32_t q_fly_idxs[] =
	{ rs->vulkan_interface->physical_device_data.graphics_queue_family_index };

	depth_image_resource = ImageResource_create(
		extent,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		"depth texture"
	);

	QueueTasks_record_tasks(rs->vulkan_interface->graphics_tasks, frame_in_flight);
	QueueTasks_change_image_layout(
		rs->vulkan_interface->graphics_tasks,
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
		VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		frame_in_flight,
		depth_image_resource->image
	);
	QueueTasks_submit_tasks(rs->vulkan_interface->graphics_tasks, rs->vulkan_interface->transfer_tasks, rs->vulkan_interface->compute_tasks, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, frame_in_flight);
}

void RenderSystem_destroy(RenderSystem* rs)
{
	if (rs != NULL)
	{
		ImageResource_destroy(depth_image_resource);
		destroy_acq_prsnt_sems(rs);
		VulkanInterface_destroy(rs->vulkan_interface);

		free(rs);
	}
}
