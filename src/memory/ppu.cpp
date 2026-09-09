#include "ppu.h"

PPU::PPU(uint8_t* vram, uint8_t* oam) : vram_(vram), oam_(oam) {
    for (auto& px : framebuffer) px = 0;
    for (auto& c : bgColorBuffer) c = 0;
}

bool PPU::consumeFrameReady() {
    bool r = frameReady;
    frameReady = false;
    return r;
}

void PPU::updateLYCFlag(uint8_t& interruptFlags) {
    if (ly == lyc) {
        stat |= 0x04;
        if (stat & 0x40) interruptFlags |= 0x02;
    }
    else {
        stat &= ~0x04;
    }
}

uint8_t PPU::tick(uint8_t cycles) {
    uint8_t interruptFlags = 0;

    if (!(lcdc & 0x80)) {
        // LCD disabled: real hardware halts the PPU entirely. We don't advance
        // ly/lineDot or render while off.
        return 0;
    }

    lineDot += cycles;

    // In practice a single CPU instruction is far shorter than a scanline (456
    // cycles), so this loop runs 0-1 times per call. It's written as a loop for
    // safety, but note: if it ever ran more than once in a single tick(), any
    // mode-transition interrupts within the skipped line(s) would be missed.
    while (lineDot >= 456) {
        lineDot -= 456;
        ly++;
        if (ly == 154) {
            ly = 0;
            windowLineCounter = 0;
        }
        updateLYCFlag(interruptFlags);
    }

    uint8_t newMode;
    if (ly >= 144) {
        newMode = 1; // VBlank
    }
    else if (lineDot < 80) {
        newMode = 2; // OAM scan
    }
    else if (lineDot < 252) {
        newMode = 3; // Pixel transfer (80+172 fixed length — see note below)
    }
    else {
        newMode = 0; // HBlank
    }

    if (newMode != mode) {
        mode = newMode;
        switch (mode) {
        case 2:
            if (stat & 0x20) interruptFlags |= 0x02;
            break;
        case 3:
            // Not true pixel-FIFO timing — we render the whole scanline at
            // once here, at the moment pixel transfer begins. Good enough
            // for background/window/sprites to be visually correct; not
            // accurate for games that race the PPU mid-scanline.
            renderScanline(ly);
            break;
        case 0:
            if (stat & 0x08) interruptFlags |= 0x02;
            break;
        case 1:
            interruptFlags |= 0x01; // VBlank interrupt always fires, unconditionally
            if (stat & 0x10) interruptFlags |= 0x02;
            frameReady = true;
            break;
        }
    }

    return interruptFlags;
}

uint8_t PPU::readRegister(uint16_t address) const {
    switch (address) {
    case 0xFF40: return lcdc;
    case 0xFF41: return 0x80 | (stat & 0x7C) | (mode & 0x03);
    case 0xFF42: return scy;
    case 0xFF43: return scx;
    case 0xFF44: return ly;
    case 0xFF45: return lyc;
    case 0xFF47: return bgp;
    case 0xFF48: return obp0;
    case 0xFF49: return obp1;
    case 0xFF4A: return wy;
    case 0xFF4B: return wx;
    default: return 0xFF;
    }
}

void PPU::writeRegister(uint16_t address, uint8_t value) {
    switch (address) {
    case 0xFF40: {
        bool wasEnabled = lcdc & 0x80;
        lcdc = value;
        std::cout << "LCDC write: 0x" << std::hex << (int)value << std::dec << std::endl;
        bool nowEnabled = lcdc & 0x80;
        if (wasEnabled && !nowEnabled) {
            ly = 0;
            lineDot = 0;
            mode = 0;
            windowLineCounter = 0;
            for (auto& px : framebuffer) px = 0; // blank the screen
        }
        break;
    }
    case 0xFF41:
        stat = (stat & 0x04) | (value & 0x78); // keep LYC flag, take int-enable bits
        break;
    case 0xFF42: scy = value; break;
    case 0xFF43: scx = value; break;
    case 0xFF44: /* LY is read-only, writes ignored */ break;
    case 0xFF45: lyc = value; break;
    case 0xFF47: bgp = value; break;
    case 0xFF48: obp0 = value; break;
    case 0xFF49: obp1 = value; break;
    case 0xFF4A: wy = value; break;
    case 0xFF4B: wx = value; break;
    default: break;
    }
}

uint8_t PPU::getTileColorNumber(uint16_t tileDataBase, bool signedAddressing,
    int tileIndex, int row, int col) {
    uint16_t tileAddr;
    if (signedAddressing) {
        tileAddr = tileDataBase + (int8_t)tileIndex * 16;
    }
    else {
        tileAddr = tileDataBase + (uint8_t)tileIndex * 16;
    }
    tileAddr += row * 2;

    uint8_t lowByte = vram_[tileAddr - 0x8000];
    uint8_t highByte = vram_[tileAddr + 1 - 0x8000];

    int bit = 7 - col;
    uint8_t lo = (lowByte >> bit) & 1;
    uint8_t hi = (highByte >> bit) & 1;
    return (hi << 1) | lo;
}

void PPU::renderScanline(uint8_t line) {
    renderBackgroundLine(line);
    renderWindowLine(line);
    renderSpritesLine(line);
}

void PPU::renderBackgroundLine(uint8_t line) {
    if (!(lcdc & 0x01)) {
        for (int x = 0; x < 160; x++) {
            framebuffer[line * 160 + x] = 0;
            bgColorBuffer[line * 160 + x] = 0;
        }
        return;
    }

    bool signedAddressing = !(lcdc & 0x10);
    uint16_t tileDataBase = signedAddressing ? 0x9000 : 0x8000;
    uint16_t bgMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;

    uint8_t y = line + scy; // wraps naturally as uint8_t across the 256x256 BG map
    int tileRow = y / 8;
    int rowInTile = y % 8;

    for (int x = 0; x < 160; x++) {
        uint8_t bgX = x + scx;
        int tileCol = bgX / 8;
        int colInTile = bgX % 8;

        uint16_t mapAddr = bgMapBase + (tileRow * 32) + tileCol;
        uint8_t tileIndex = vram_[mapAddr - 0x8000];

        uint8_t colorNum = getTileColorNumber(tileDataBase, signedAddressing, tileIndex, rowInTile, colInTile);
        uint8_t shade = (bgp >> (colorNum * 2)) & 0x03;

        framebuffer[line * 160 + x] = shade;
        bgColorBuffer[line * 160 + x] = colorNum;
    }
}

void PPU::renderWindowLine(uint8_t line) {
    if (!(lcdc & 0x20)) return;
    if (wy > line) return;
    if (wx > 166) return;

    bool signedAddressing = !(lcdc & 0x10);
    uint16_t tileDataBase = signedAddressing ? 0x9000 : 0x8000;
    uint16_t winMapBase = (lcdc & 0x40) ? 0x9C00 : 0x9800;

    int winY = windowLineCounter;
    int tileRow = winY / 8;
    int rowInTile = winY % 8;
    int startX = wx - 7;

    bool drewAnyPixel = false;

    for (int x = 0; x < 160; x++) {
        int winX = x - startX;
        if (winX < 0) continue;
        drewAnyPixel = true;

        int tileCol = winX / 8;
        int colInTile = winX % 8;

        uint16_t mapAddr = winMapBase + (tileRow * 32) + tileCol;
        uint8_t tileIndex = vram_[mapAddr - 0x8000];

        uint8_t colorNum = getTileColorNumber(tileDataBase, signedAddressing, tileIndex, rowInTile, colInTile);
        uint8_t shade = (bgp >> (colorNum * 2)) & 0x03;

        framebuffer[line * 160 + x] = shade;
        bgColorBuffer[line * 160 + x] = colorNum;
    }

    if (drewAnyPixel) windowLineCounter++;
}

void PPU::renderSpritesLine(uint8_t line) {
    if (!(lcdc & 0x02)) return;

    int spriteHeight = (lcdc & 0x04) ? 16 : 8;

    struct Candidate { int oamIndex; int x; int y; uint8_t tile; uint8_t attr; };
    Candidate candidates[10];
    int count = 0;

    for (int i = 0; i < 40 && count < 10; i++) {
        // IMPORTANT: use `int`, not uint8_t, for this subtraction. OAM Y/X are
        // stored offset by +16/+8, so a sprite near the top-left of the screen
        // has a small oam byte value; subtracting in uint8_t wraps around
        // (e.g. 0 - 16 becomes 240) instead of going negative like it should.
        int spriteY = (int)oam_[i * 4 + 0] - 16;
        int spriteX = (int)oam_[i * 4 + 1] - 8;
        uint8_t tile = oam_[i * 4 + 2];
        uint8_t attr = oam_[i * 4 + 3];

        int rowInSprite = (int)line - spriteY;
        if (rowInSprite < 0 || rowInSprite >= spriteHeight) continue;

        candidates[count++] = { i, spriteX, spriteY, tile, attr };
    }

    for (int x = 0; x < 160; x++) {
        int bestIdx = -1;
        for (int c = 0; c < count; c++) {
            int colInSprite = x - candidates[c].x;
            if (colInSprite < 0 || colInSprite >= 8) continue;

            if (bestIdx == -1 ||
                candidates[c].x < candidates[bestIdx].x ||
                (candidates[c].x == candidates[bestIdx].x && candidates[c].oamIndex < candidates[bestIdx].oamIndex)) {
                bestIdx = c;
            }
        }
        if (bestIdx == -1) continue;

        const Candidate& s = candidates[bestIdx];
        int rowInSprite = (int)line - s.y;
        int colInSprite = x - s.x;

        bool flipY = s.attr & 0x40;
        bool flipX = s.attr & 0x20;
        bool behindBG = s.attr & 0x80;
        uint8_t palette = (s.attr & 0x10) ? obp1 : obp0;

        int actualRow = flipY ? (spriteHeight - 1 - rowInSprite) : rowInSprite;
        int actualCol = flipX ? (7 - colInSprite) : colInSprite;

        uint8_t tileIndex = s.tile;
        if (spriteHeight == 16) {
            tileIndex &= 0xFE; // bit0 ignored for 8x16 sprites
            if (actualRow >= 8) { tileIndex += 1; actualRow -= 8; }
        }

        // Sprites always use unsigned $8000 addressing, regardless of LCDC bit4
        // (that bit only affects BG/window tile addressing).
        uint8_t colorNum = getTileColorNumber(0x8000, false, tileIndex, actualRow, actualCol);
        if (colorNum == 0) continue; // color 0 is always transparent for sprites

        if (behindBG && bgColorBuffer[line * 160 + x] != 0) continue;

        uint8_t shade = (palette >> (colorNum * 2)) & 0x03;
        framebuffer[line * 160 + x] = shade;
    }
}