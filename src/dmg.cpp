// dmg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "memory/Cartridge.h"
#include "memory/mmu.h"
#include "cpu/cpu.h"

int main(int argc, char* argv[]) {

	Cartridge cartridge;
	//cartridge.loadROM(".\\rom\\tetris.gb");
    cartridge.loadROM(".\\rom\\cpu_instrs.gb");
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
        "An SDL3 window",                  // window title
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

    while (!done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        // Do game logic, present a frame, etc.
        cpu.step();
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;
}