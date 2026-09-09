# DMGBoy

A Game Boy (DMG) emulator written in C++, built from scratch as a learning project. Uses SDL3 for windowing, rendering, and input.

![platform](https://img.shields.io/badge/platform-Windows-blue) ![language](https://img.shields.io/badge/language-C%2B%2B-orange)

## Status

Core CPU, timer, MBC1 cartridge support, and a working PPU are implemented. Several commercial games (Super Mario Land, Tetris) boot and are playable.

### Implemented
- **CPU** — full opcode table per [gbops](https://izik1.github.io/gbops/), interrupt handling, HALT
- **Timer** — DIV/TIMA/TMA/TAC with edge-detected overflow and DIV-write quirk
- **MBC1** — ROM/RAM banking with mode select, up to 2MB ROM / 32KB RAM
- **PPU** — scanline-based renderer:
  - Background and window layers
  - Sprites (OBJ), including 8x8/8x16 modes, X/Y flip, palettes, priority
  - LCDC/STAT/SCY/SCX/LY/LYC/palette registers
  - Mode 0–3 timing and STAT/VBlank interrupt sources
  - OAM DMA (`0xFF46`)
- **Joypad input** — mapped to arrow keys + Z/X/Enter/Right Shift
- **Frame pacing** — locked to ~59.7 FPS to match real hardware timing
- **Test ROM validation** — passes Blargg's `cpu_instrs.gb` (all 11 sub-tests)

### Not yet implemented
- PPU: pixel-FIFO-accurate mid-scanline timing (currently fixed-length mode 3)
- Additional MBCs (MBC3 with RTC, MBC5, MBC2)
- APU / audio
- Save file (battery-backed RAM) persistence
- Boot ROM

See [Roadmap](#roadmap) below for what's next.

## Controls

| Game Boy | Key |
|---|---|
| D-Pad | Arrow keys |
| A | Z |
| B | X |
| Select | Right Shift |
| Start | Enter |

## Building

> Built and tested with Visual Studio on Windows. Requires [SDL3](https://github.com/libsdl-org/SDL) (linked via vcpkg or a local SDL3 install).

1. Clone the repository
2. Open the solution in Visual Studio
3. Ensure SDL3 headers/libs are available to the project (via vcpkg, NuGet, or manual linking)
4. Build and run

ROMs are loaded from a `rom/` folder relative to the executable's working directory (e.g. `.\rom\mario.gb`) — update the path in `dmg.cpp` to point at whatever ROM you want to run. **No ROMs are included in this repository** — you'll need to provide your own legally-obtained ROM files.

## Project structure

```
├── cpu/          CPU core, opcode implementations, timer
├── memory/       MMU, Cartridge (+ MBC), PPU
├── dmg.cpp       Entry point — SDL setup, main loop, input
```

## Testing

Validated against [Blargg's test ROMs](https://github.com/retrymankind/blargg-gb-tests) — `cpu_instrs.gb` passes all 11 sub-tests (registers, timing, interrupts, etc.).

## Roadmap

- [ ] MBC3 (Pokémon Red/Blue/Yellow and other larger-era titles), including RTC
- [ ] Additional MBCs as needed (MBC5, MBC2)
- [ ] APU (4 sound channels + mixing + SDL audio backend)
- [ ] Battery save persistence
- [ ] Pixel-FIFO-accurate PPU timing

## Sources of truth

- Opcodes: [gbops](https://izik1.github.io/gbops/)
- Timer behavior: [Pan Docs — Timer and Divider Registers](https://gbdev.io/pandocs/Timer_and_Divider_Registers.html)
- General hardware reference: [Pan Docs](https://gbdev.io/pandocs/)

## Acknowledgments

Built as a guided, from-scratch learning project — architecture and hardware quirks referenced from the Pan Docs and gbops community documentation.
