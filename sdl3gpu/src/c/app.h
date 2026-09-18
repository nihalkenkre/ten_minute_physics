#pragma once

#include <SDL3/SDL.h>
#include "gui.h"

typedef struct _App {
    SDL_Window* window;
    SDL_GPUDevice* device;
    GUI* gui;
} App;

App* App_create(SDL_Window* window, SDL_GPUDevice* device);
void App_event(App* app, SDL_Event* event);
void App_iterate(App* app);
void App_destroy(App* app);
