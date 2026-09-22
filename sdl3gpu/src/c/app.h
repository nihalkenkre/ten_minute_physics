#pragma once

#include <SDL3/SDL.h>
#include "gui.h"

void App_create(SDL_Window* window, SDL_GPUDevice* device);
void App_event(SDL_Event* event);
void App_iterate();
void App_destroy();
