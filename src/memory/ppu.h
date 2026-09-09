#pragma once
#include <cstdint>

uint8_t lcdc = 0x91;   // LCD Control
uint8_t stat = 0x00;   // LCD Status
uint8_t scy = 0x00;   // BG scroll Y
uint8_t scx = 0x00;   // BG scroll X
uint8_t ly = 0x00;   // current scanline (read-only to CPU)
uint8_t lyc = 0x00;   // LY compare
uint8_t bgp = 0xFC;   // BG palette
uint8_t obp0 = 0xFF;   // OBJ palette 0
uint8_t obp1 = 0xFF;   // OBJ palette 1
uint8_t wy = 0x00;   // window Y
uint8_t wx = 0x00;   // window X

uint8_t framebuffer[160 * 144]; // 2-bit shade index per pixel, 0-3
