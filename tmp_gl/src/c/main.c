#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <glad/gl.h>

// #include "app.h"
// #include "utils.h"
// #include "events.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define SDL_CHECK(result)               \
	if (!result) \
	{                      \
	    SDL_Log("%s\n", SDL_GetError());    \
    }

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	SDL_CHECK(SDL_Init(SDL_INIT_VIDEO));

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_CHECK(SDL_CreateWindowAndRenderer("Ten Minute Physics", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL, &window, &renderer));

	if (window == NULL)
	{
		SDL_Log("%s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);

	gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

	SDL_CHECK(SDL_ShowWindow(window));

	// App* app = App_create(window);
	// *appstate = app;

	// events_initialize();
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	// App* app = (App*)(appstate);

	// if (SDL_GetWindowFlags(app->window) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED))
	// {
	// 	return SDL_APP_CONTINUE;
	// }

	if (event->type == SDL_EVENT_QUIT)
	{
		return SDL_APP_SUCCESS;
	}

	// App_process_event(app, event);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	// App* app = (App*)(appstate);

	// if (SDL_GetWindowFlags(app->window) & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED))
	// {
	// 	return SDL_APP_CONTINUE;
	// }

	// App_iterate(app);

	glClearColor((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f);
	glClearDepth(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SDL_CHECK(SDL_GL_SwapWindow(window));

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	// App* app = (App*)(appstate);
	// App_destroy(app);
}