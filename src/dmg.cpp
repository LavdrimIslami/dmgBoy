// dmg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "memory/Cartridge.h"
#include "memory/mmu.h"
#include "cpu/cpu.h"
constexpr uint64_t FRAME_TIME_NS = (uint64_t)(1e9 * 70224.0 / 4194304.0); // ~16,742,706 ns

int main(int argc, char* argv[]) {
	Cartridge cartridge;
	//cartridge.loadROM(".\\rom\\tetris.gb");
    cartridge.loadROM(".\\rom\\mario.gb");

    MMU mmu(cartridge);
    CPU cpu(mmu);

    std::cout << "\nRead address at 0x0104 returns: " << std::hex << (int)mmu.read8(0x0104) << std::endl;
    std::cout << "\n";

    cartridge.getCartridgeType();
    cartridge.getTitle();

    SDL_Window* window;                    // Declare a pointer
    bool done = false;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL3

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "DMGBoy ",                  // window title
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_RESIZABLE                  // flags - see below
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    // The Game Boy resolution is 160x144
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    // ARGB colors for shades 0-3 (White, Light Gray, Dark Gray, Black)
    uint32_t palette[4] = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
    uint32_t pixels[160 * 144];
    uint64_t steps = 0;
    uint64_t lastFrameTime = SDL_GetTicksNS();
    while (!done) {
        // Assuming cpu.step() internally advances the MMU/PPU cycles
        uint8_t cycles = cpu.step();
        steps++;

        if (mmu.getPPU().consumeFrameReady()) {
            const uint8_t* fb = mmu.getPPU().getFramebuffer();
            for (int i = 0; i < 160 * 144; i++) {
                pixels[i] = palette[fb[i]];
            }

            SDL_UpdateTexture(texture, NULL, pixels, 160 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                    bool isPressed = (event.type == SDL_EVENT_KEY_DOWN);
                    switch (event.key.key) {
                    case SDLK_RIGHT:  mmu.handleInput(true, 0, isPressed); break;
                    case SDLK_LEFT:   mmu.handleInput(true, 1, isPressed); break;
                    case SDLK_UP:     mmu.handleInput(true, 2, isPressed); break;
                    case SDLK_DOWN:   mmu.handleInput(true, 3, isPressed); break;
                    case SDLK_Z:      mmu.handleInput(false, 0, isPressed); break;
                    case SDLK_X:      mmu.handleInput(false, 1, isPressed); break;
                    case SDLK_RSHIFT: mmu.handleInput(false, 2, isPressed); break;
                    case SDLK_RETURN: mmu.handleInput(false, 3, isPressed); break;
                    }
                }
                if (event.type == SDL_EVENT_QUIT) {
                    done = true;
                }
            }

            // Frame pacing — runs exactly once per frame, regardless of event count
            uint64_t now = SDL_GetTicksNS();
            uint64_t elapsed = now - lastFrameTime;
            if (elapsed < FRAME_TIME_NS) {
                SDL_DelayNS(FRAME_TIME_NS - elapsed);
            }
            lastFrameTime = SDL_GetTicksNS();
        }
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);

    // Clean up
    SDL_Quit();
    return 0;
}