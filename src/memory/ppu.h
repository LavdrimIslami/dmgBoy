#pragma once
#include <cstdint>
#include <iostream>

class PPU {
public:
    // vram/oam are raw pointers into MMU's existing arrays.
    // vram points at address 0x8000, oam points at address 0xFE00 — i.e. pass
    // MMU's `vram` and `oam` arrays directly (they decay to pointers).
    PPU(uint8_t* vram, uint8_t* oam);

    // Advances the PPU by `cycles` T-cycles. Returns interrupt request flags:
    // bit0 set = VBlank interrupt, bit1 set = LCD STAT interrupt.
    // Caller (MMU::tick) is responsible for calling requestInterrupt() on the IF register.
    uint8_t tick(uint8_t cycles);

    uint8_t readRegister(uint16_t address) const;
    void writeRegister(uint16_t address, uint8_t value);

    const uint8_t* getFramebuffer() const { return framebuffer; }

    // True once per emulated frame, right when VBlank starts. Main loop should
    // check this, blit if true, then it auto-resets.
    bool consumeFrameReady();

private:
    uint8_t* vram_;
    uint8_t* oam_;

    // Registers
    uint8_t lcdc = 0x91;
    uint8_t stat = 0x00;   // only bits 2 (LYC flag) and 3-6 (int enables) are meaningful here;
    // mode bits (0-1) are derived live from `mode` in readRegister()
    uint8_t scy = 0x00;
    uint8_t scx = 0x00;
    uint8_t lyc = 0x00;
    uint8_t bgp = 0xFC;
    uint8_t obp0 = 0xFF;
    uint8_t obp1 = 0xFF;
    uint8_t wy = 0x00;
    uint8_t wx = 0x00;

    uint8_t  ly = 0;
    uint16_t lineDot = 0;   // 0-455, position within current scanline
    uint8_t  mode = 2;      // current PPU mode 0-3

    uint8_t windowLineCounter = 0; // window has its own internal line counter, separate from LY
    bool frameReady = false;

    uint8_t framebuffer[160 * 144];   // post-palette shade (0-3) per pixel, for display
    uint8_t bgColorBuffer[160 * 144]; // raw BG/window color NUMBER (0-3) pre-palette,
    // needed for sprite-behind-BG priority checks

    void updateLYCFlag(uint8_t& interruptFlags);
    void renderScanline(uint8_t line);
    void renderBackgroundLine(uint8_t line);
    void renderWindowLine(uint8_t line);
    void renderSpritesLine(uint8_t line);
    uint8_t getTileColorNumber(uint16_t tileDataBase, bool signedAddressing,
        int tileIndex, int row, int col);
};