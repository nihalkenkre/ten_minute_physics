#pragma once

#define SDL_CHECK(result)                \
    if (!result)                         \
    {                                    \
        SDL_Log("%s\n", SDL_GetError()); \
    }
