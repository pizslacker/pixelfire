#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/* Resolution of the pixel fire buffer */
#define FIRE_WIDTH 640
#define FIRE_HEIGHT 320

/* Resolution of the actual SDL Window */
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

/* Classic 37-color fire palette (ARGB8888 format) */
static const uint32_t firePalette[37] = {
    0xFF070707, 0xFF1f0707, 0xFF2f0f07, 0xFF470f07, 0xFF571707, 0xFF671f07,
    0xFF771f07, 0xFF8f2707, 0xFF9f2f07, 0xFFaf3f07, 0xFFbf4707, 0xFFc74707,
    0xFFDF4F07, 0xFFdf5707, 0xFFdf5707, 0xFFd75f07, 0xFFd7670f, 0xFFcf6f0f,
    0xFFcf770f, 0xFFcf7f0f, 0xFFCF8717, 0xFFc78717, 0xFFc78f17, 0xFFc7971f,
    0xFFbf9f1f, 0xFFbf9f1f, 0xFFbfa727, 0xFFbfaf27, 0xFFBfb727, 0xFFbfbf2f,
    0xFFcfc72f, 0xFFcfcf37, 0xFFcfdf3f, 0xFFdfdf47, 0xFFefef4f, 0xFFffff5b,
    0xFFffffff // Hottest (White)
};

/* Heat map array and the final RGB framebuffer */
uint8_t firePixels[FIRE_HEIGHT][FIRE_WIDTH];
uint32_t frameBuffer[FIRE_HEIGHT][FIRE_WIDTH];

/* Spread the fire upwards from the bottom row */
void updateFire() {
    for (int x = 0; x < FIRE_WIDTH; x++) {
        for (int y = 1; y < FIRE_HEIGHT; y++) {
            int srcHeat = firePixels[y][x];
            
            if (srcHeat == 0) {
                firePixels[y - 1][x] = 0;
                continue;
            }

            // Introduce randomness to the decay and wind direction
            int randIdx = rand() % 3; 
            int dstX = x - randIdx + 1;
            int dstY = y - 1;

            if (dstX >= 0 && dstX < FIRE_WIDTH && dstY >= 0) {
                int newHeat = srcHeat - (randIdx & 1);
                firePixels[dstY][dstX] = newHeat > 0 ? newHeat : 0;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Could not initialize SDL2: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL2 Pixel Fire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // SDL2 requires the rendering driver index (-1 for first available) and flags
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // SDL2's way of handling internal resolution scaling (letterboxes by default)
    SDL_RenderSetLogicalSize(renderer, FIRE_WIDTH, FIRE_HEIGHT);

    // Create a streaming texture that acts as our framebuffer
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FIRE_WIDTH, FIRE_HEIGHT);

    // Seed the absolute bottom row of our fire heat map with maximum heat (index 36)
    for (int x = 0; x < FIRE_WIDTH; x++) {
        firePixels[FIRE_HEIGHT - 1][x] = 36;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            // SDL2 uses SDL_QUIT instead of SDL3's SDL_EVENT_QUIT
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        updateFire();

        // Map the heat values (0-36) into ARGB colors for our framebuffer
        for (int y = 0; y < FIRE_HEIGHT; y++) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                int heat = firePixels[y][x];
                frameBuffer[y][x] = firePalette[heat];
            }
        }

        // Push the framebuffer to the SDL Texture
        SDL_UpdateTexture(texture, NULL, frameBuffer, FIRE_WIDTH * sizeof(uint32_t));

        // Render to screen
        SDL_RenderClear(renderer);
        // SDL2 uses SDL_RenderCopy instead of SDL3's SDL_RenderTexture
        SDL_RenderCopy(renderer, texture, NULL, NULL); 
        SDL_RenderPresent(renderer);

        // Cap frame rate at roughly 60 FPS
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}