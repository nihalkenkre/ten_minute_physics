#include "gui.h"
#include "events.h"
#include "utils.h"
#include "vulkan_interface.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <ImGuiFileDialog.h>

static char c_file_path[_MAX_PATH];
static float fps = 24.f;
static VkDescriptorPool imgui_descriptor_pool = VK_NULL_HANDLE;

GUI* GUI_create(SDL_Window* window, const VulkanInterface* vulkan_interface)
{
	fps = 24;

	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;

	ImGui_ImplSDL3_InitForVulkan(window);

	const VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 1000,
		.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes)),
		.pPoolSizes = pool_sizes,
	};

	VK_CHECK("create imgui desc pool", vkCreateDescriptorPool(vulkan_interface->device->device, &pool_info, nullptr, &imgui_descriptor_pool));

	ImGui_ImplVulkan_InitInfo imgui_init_info = {
		.Instance = vulkan_interface->instance,
		.PhysicalDevice = vulkan_interface->physical_device_data.physical_device,
		.Device = vulkan_interface->device->device,
		.Queue = vulkan_interface->device->graphics_queue,
		.DescriptorPool = imgui_descriptor_pool,
		.MinImageCount = vulkan_interface->swapchain->images_count,
		.ImageCount = vulkan_interface->max_frames_in_flight,
		.PipelineInfoMain = {
			.PipelineRenderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &vulkan_interface->surface->surface_format.surfaceFormat.format,
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
			},
		},
		.UseDynamicRendering = true,
	};

	SDL_CHECK(ImGui_ImplVulkan_Init(&imgui_init_info));

	GUI* gui = reinterpret_cast<GUI*>(std::calloc(1, sizeof(GUI)));
	gui->frame_start = 1;
	gui->frame_end = 250;
	gui->frame_number = 1;
	gui->substeps = 10;

	return gui;
}

void GUI_process_event(GUI* gui, SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
}

void GUI_render(GUI* gui, const VkCommandBuffer command_buffer)
{
	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Awesome stuff"))
	{
		ImGui::Text("Frame time: %0.3f ms", 1000.f / ImGui::GetIO().Framerate);

		if (ImGui::Button("Load GLTF"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";

			ImGuiFileDialog::Instance()->OpenDialog("GLTFDlg", "Choose GLTF File", ".glb,.gltf", config);
		}

		if (ImGuiFileDialog::Instance()->Display("GLTFDlg"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				auto file_path = ImGuiFileDialog::Instance()->GetFilePathName();
				strcpy(c_file_path, file_path.c_str());

				events.FileOpen.user.data1 = c_file_path;
				SDL_CHECK(SDL_PushEvent(&events.FileOpen));
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGui::InputFloat("FPS", &fps))
		{
			fps = max(fps, 1.f);
			events.FPSChanged.user.data1 = &fps;
			SDL_CHECK(SDL_PushEvent(&events.FPSChanged));
		}

		if (ImGui::DragInt("Substeps", &gui->substeps))
		{
			gui->substeps = max(gui->substeps, 1);

			events.SubstepsChanged.user.data1 = &gui->substeps;
			SDL_CHECK(SDL_PushEvent(&events.SubstepsChanged));
		}

		if (ImGui::Button("Simulate"))
		{
			SDL_CHECK(SDL_PushEvent(&events.Simulate));
		}

		if (ImGui::Button("Play"))
		{
			SDL_CHECK(SDL_PushEvent(&events.Play));
		}

		if (ImGui::SliderInt("Timeline", &gui->frame_number, gui->frame_start, gui->frame_end))
		{
		}

		if (ImGui::Button("Stop"))
		{
			SDL_CHECK(SDL_PushEvent(&events.Stop));
		}
	}

	ImGui::End();
	ImGui::EndFrame();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

void GUI_increment_frame(GUI* gui)
{
	if (++gui->frame_number >= 250)
		gui->frame_number = 1;

	gui->frame_number = (gui->frame_number);
}

void GUI_destroy(GUI* gui, const VkDevice device)
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(device, imgui_descriptor_pool, nullptr);

	if (gui != nullptr)
	{
		std::free(gui);
	}
}
