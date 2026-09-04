#include "app.h"
#include "events.h"
#include "utils.h"
#include "gui.h"
#include "scene.h"
#include "render_system.h"

#include <SDL3/SDL.h>

static GUI *gui;
static Scene *scene;
static TimelineMode timeline_mode;
static float delta_time = 1.f / 24.f;
static RenderSystem *render_system;
static SDL_RWLock* rw_lock = NULL;

typedef struct SceneCreateThreadData
{
	const char *path;
} SceneCreateThreadData;

static SceneCreateThreadData sctd;

int Scene_replace_ThreadFunc(void *data)
{
	SDL_Mutex *mutex = SDL_CreateMutex();
	SDL_LockMutex(mutex);

	SceneCreateThreadData *sctd = (SceneCreateThreadData *)(data);
	Scene_replace(scene, sctd->path, rw_lock);

	SDL_UnlockMutex(mutex);
	SDL_DestroyMutex(mutex);

	return 0;
}

App *App_create(SDL_Window *window)
{
	App *app = (App *)(calloc(1, sizeof(App)));
	app->window = window;
	render_system = RenderSystem_create(window);
	gui = GUI_create(window, render_system->vulkan_interface);
	delta_time = 1 / 24.f;
	scene = Scene_create("");
	rw_lock = SDL_CreateRWLock();

	return app;
}

Uint32 App_play(void *data, SDL_TimerID timer_id, Uint32 interval)
{
	App *app = (App *)(data);

	Scene_play(scene, gui->frame_number);

	if (timeline_mode == TIMELINE_MODE_STOPPED)
		return 0;
	else
		return interval;
}

void App_process_event(App *app, SDL_Event *event)
{
	GUI_process_event(gui, event);

	if (event->type == events.FileOpen.type)
	{
		sctd.path = (const char *)(event->user.data1);
		SDL_DetachThread(SDL_CreateThread(Scene_replace_ThreadFunc, "Scene_replace", &sctd));
	}
	else if (event->type == events.Simulate.type)
	{
		timeline_mode = TIMELINE_MODE_SIMULATING;
	}
	else if (event->type == events.Play.type)
	{
		if (timeline_mode == TIMELINE_MODE_STOPPED)
		{
			timeline_mode = TIMELINE_MODE_PLAYING;
			SDL_TimerID play_timer = SDL_AddTimer((Uint32)(delta_time * 1000), App_play, app);
		}
	}
	else if (event->type == events.Stop.type)
	{
		timeline_mode = TIMELINE_MODE_STOPPED;
	}
	else if (event->type == events.FPSChanged.type)
	{
		delta_time = 1.f / *((float *)(event->user.data1));
	}
	else if (event->type == events.SimulationFrameDone.type)
	{
		GUI_increment_frame(gui);
	}
	else if (event->type == events.PlayFrameDone.type)
	{
		GUI_increment_frame(gui);
	}
	else if (event->type == SDL_EVENT_WINDOW_RESIZED)
	{
		vkDeviceWaitIdle(render_system->vulkan_interface->device->device);
		RenderSystem_recreate_swapchain(render_system);
		RenderSystem_recreate_depth_texture(render_system);
	}
}

void App_simulate(App *app)
{
	Scene_simulate(scene, delta_time, (size_t)(gui->substeps), (size_t)(gui->frame_number));
}

void App_render(App *app)
{
	int w, h;
	SDL_CHECK(SDL_GetWindowSize(app->window, &w, &h));

	RenderSystem_render(render_system, gui, scene, rw_lock);
}

void App_iterate(App *app)
{
	if (timeline_mode == TIMELINE_MODE_SIMULATING)
	{
		App_simulate(app);
	}

	App_render(app);
}

void App_destroy(App *app)
{
	if (app != NULL)
	{
		vkDeviceWaitIdle(render_system->vulkan_interface->device->device);
		GUI_destroy(gui, render_system->vulkan_interface->device->device);
		Scene_destroy(scene);
		RenderSystem_destroy(render_system);
		SDL_DestroyRWLock(rw_lock);
		
		free(app);
	}
}
