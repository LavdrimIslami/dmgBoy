#pragma once
#include <cstdint>
#include <vector>
#include <string>


class Cartridge
{
private:
	std::vector<uint8_t> romByteVector;

	struct headerBytes {
		uint8_t entryPoint[4];
		uint8_t nintendoLogo[48];
		uint8_t title[16];
		uint8_t manufacturerCode[4];
		uint8_t cgbFlag;
		uint8_t newLicenseeCode[2];
		uint8_t sgbFlag;
		uint8_t cartridgeType;
		uint8_t romSize;
		uint8_t ramSize;
		uint8_t destinationCode;
		uint8_t oldLicenseeCode;
		uint8_t maskROMVersionNumber;
		uint8_t headerChecksum;
		uint16_t globalChecksum;
	};

	headerBytes header;


public:
	void loadROM(const char* filePath);
	
	std::string getCartridgeType() const;
	std::string getTitle() const;

};