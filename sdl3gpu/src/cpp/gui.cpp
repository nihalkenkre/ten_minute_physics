#include "gui.h"
#include "events.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <memory>

#include <imnodes.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <ImGuiFileDialog.h>

typedef struct _GUI
{
	SDL_Window* window;
	SDL_GPUDevice* device;
} GUI;

static GUI gui;
static std::string file_path = "";

void GUI_create(SDL_Window* window, SDL_GPUDevice* device)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImNodes::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplSDL3_InitForSDLGPU(window);

	ImGui_ImplSDLGPU3_InitInfo init_info = {
		.Device = device,
		.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window),
		.MSAASamples = SDL_GPU_SAMPLECOUNT_1,
		.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
		.PresentMode = SDL_GPU_PRESENTMODE_MAILBOX
	};

	ImGui_ImplSDLGPU3_Init(&init_info);

	gui.window = window;
	gui.device = device;
}

bool GUI_process_event(SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void GUI_render(SDL_GPUCommandBuffer* command_buffer, const SDL_GPUColorTargetInfo* target_info)
{
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Awesome Panel");
	ImGui::Text("Frame time: %0.3f ms", 1000.f / ImGui::GetIO().Framerate);

	if (ImGui::Button("Load GLTF"))
	{
		IGFD::FileDialogConfig config;
		config.path = ".";

		ImGuiFileDialog::Instance()->OpenDialog("GLTFDlg", "Choose GLTF File", ".gltf,.glb", config);
	}

	if (ImGuiFileDialog::Instance()->Display("GLTFDlg"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			file_path = ImGuiFileDialog::Instance()->GetFilePathName();

			events.FileOpen.user.data1 = reinterpret_cast<void*>((char*)file_path.c_str());
			SDL_CHECK(SDL_PushEvent(&events.FileOpen));
		}

		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::End();

	ImGui::Begin("Nodes");
	ImNodes::BeginNodeEditor();

	ImNodes::BeginNode(1);
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted("Simple node");
	ImNodes::EndNodeTitleBar();

	ImNodes::BeginInputAttribute(2);
	ImGui::Text("input");
	ImNodes::EndInputAttribute();

	ImNodes::BeginOutputAttribute(3);
	ImGui::Indent(40);
	ImGui::Text("output");
	ImNodes::EndOutputAttribute();
	ImNodes::EndNode();

	ImNodes::EndNodeEditor();
	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();

	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, target_info, 1, nullptr);
	ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
	SDL_EndGPURenderPass(render_pass);

	ImGuiIO& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void GUI_destroy()
{
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImNodes::DestroyContext();
	ImGui::DestroyContext();
}
