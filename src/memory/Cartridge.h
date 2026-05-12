#pragma once
#include <cstdint>
#include <vector>

#define ROM_SIZE 0x8000


class Cartridge
{
private:
	uint8_t checksum = 0;

public:
	std::vector<uint8_t> romByteVector;
	
	void loadROM(const char* filePath);

};