#include "Cartridge.h"
#include <fstream>
#include <iostream>
#include <cstdint>
#include <iterator>



void Cartridge::loadROM(const char* filePath){
	std::ifstream file(filePath, std::ios::binary);
	std::cout << "Loading ROM from file: " << filePath << std::endl;

	if (!file) {
		std::cerr << "Error opening file: " << filePath << std::endl;
	}
	
	romByteVector.resize(ROM_SIZE);
	romByteVector.assign(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
	

	//header checksum
	//if byte at $014D != lower 8 bits of checksum, rom locks and cartridge wont work
	std::cout << "Calculating header checksum..." << std::endl;
	for (uint16_t address = 0x0134; address <= 0x014C; address++) {
		checksum = checksum - romByteVector[address] - 1;
	}
	std::cout << "checksum passed" << std::endl;


}