#include "gui.h"

#include <SDL3/SDL.h>
#include <memory>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>


GUI* GUI_create(SDL_Window* window, SDL_GPUDevice* device)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLGPU(window);

	ImGui_ImplSDLGPU3_InitInfo init_info = {
		.Device = device,
		.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window),
		.MSAASamples = SDL_GPU_SAMPLECOUNT_1,
		.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
		.PresentMode = SDL_GPU_PRESENTMODE_MAILBOX
	};

	ImGui_ImplSDLGPU3_Init(&init_info);

	GUI* gui = reinterpret_cast<GUI*>(std::calloc(1, sizeof(GUI)));
	gui->window = window;
	gui->device = device;

	return gui;
}

bool GUI_process_event(GUI* gui, SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void GUI_render(GUI* gui, SDL_GPUCommandBuffer* command_buffer, const SDL_GPUColorTargetInfo* target_info)
{
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, target_info, 1, nullptr);
	ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
	SDL_EndGPURenderPass(render_pass);
}

void GUI_destroy(GUI* gui)
{
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();

	if (gui != nullptr)
	{
		std::free(gui);
	}
}
