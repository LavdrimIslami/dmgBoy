#include "Cartridge.h"
#include <fstream>
#include <iostream>
#include <cstdint>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

void Cartridge::loadROM(const char* filePath){
	std::ifstream file(filePath, std::ios::binary);
	std::cout << "Loading ROM from file: " << filePath << std::endl;

	if (!file) {
		std::cerr << "Error opening file: " << filePath << std::endl;
		return;
	}	
	
	romByteVector.assign(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());


	//entry point
	this->header.entryPoint[0] = romByteVector[0x0100];
	this->header.entryPoint[1] = romByteVector[0x0101];
	this->header.entryPoint[2] = romByteVector[0x0102];
	this->header.entryPoint[3] = romByteVector[0x0103];

	//nintendo logo
	for (int i = 0; i < 48; i++) {
		this->header.nintendoLogo[i] = romByteVector[0x0104 + i];
	}

	//CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D
	//00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99
	//BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E


	//title
	for (int i = 0; i < 16; i++) {
		this->header.title[i] = romByteVector[0x0134 + i];
	}
	//CGB flag
	this->header.cgbFlag = romByteVector[0x0143];

	//manufacturer code
	//okay so its 4 bytes at 0x013F - 0x0142. but only on new carts
	//on old carts its part of title field 
	//new carts, title is 11 bytes and this is the next 4
	for (auto i = 0; i < 4; i++) {
		if (this->header.cgbFlag == 0x80 || this->header.cgbFlag == 0xC0) {
			this->header.manufacturerCode[i] = romByteVector[0x013F + i];
		}
		else {
			break;
		}
	}

	//new licensee code
	this->header.newLicenseeCode[0] = romByteVector[0x0144];
	this->header.newLicenseeCode[1] = romByteVector[0x0145];


	//sgb flag
	this->header.sgbFlag = romByteVector[0x0146];

	//cart type
	this->header.cartridgeType = romByteVector[0x0147];

	//romsize
	this->header.romSize = romByteVector[0x0148];

	//ramsize
	this->header.ramSize = romByteVector[0x0149];

	numRomBanks = 2 << header.romSize;
	
	switch (this->header.ramSize) {
		
	case 0x00:
		numRamBanks = 0;
		break;
	case 0x01:
		numRamBanks = 0;
		break;
	case 0x02:
		numRamBanks = 1;
		break;
	case 0x03:
		numRamBanks = 4;
		break;
	case 0x04:
		numRamBanks = 16;
		break;
	case 0x05:
		numRamBanks = 8;
		break;
	default:
		break;
	}

	ramByteVector.resize(numRamBanks * 0x2000, 0);

	//destcode
	this->header.destinationCode = romByteVector[0x014A];

	//oldlicensee
	this->header.oldLicenseeCode = romByteVector[0x014B];

	//mask rom version num
	this->header.maskROMVersionNumber = romByteVector[0x014C];


	//header checksum
	//if byte at $014D != lower 8 bits of checksum, rom locks and cartridge wont work
	std::cout << "Calculating header checksum..." << std::endl;
	std::cout<<"Rom Banks: "<< numRomBanks << std::endl;

	this->header.headerChecksum = 0;

	for (uint16_t address = 0x0134; address <= 0x014C; address++) {
		this->header.headerChecksum = this->header.headerChecksum - romByteVector[address] - 1;
	}
	 
	(this->header.headerChecksum == romByteVector[0x014D]) ? std::cout << "Checksum passed" << std::endl : std::cout << "Checksum failed" << std::endl;
	
}


const uint8_t Cartridge::read(uint16_t address) {
	if (address >= 0x0000 && address <= 0x3FFF) {
		if (modeSelect == 0x00) {
			return romByteVector[address];
		}
		else if (modeSelect == 0x01) {
			auto x = (bankReg2 << 5) & (numRomBanks - 1);
			return romByteVector[x * 0x4000 + address];
		}
	}
	if (address >= 0x4000 && address <= 0x7FFF) {
		uint8_t lowBits = (romBankLow == 0) ? 1 : romBankLow;
		auto bank = lowBits | bankReg2 << 5;
		bank &= (numRomBanks - 1);
		return romByteVector[bank * 0x4000 + (address - 0x4000)];
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnabled && numRamBanks > 0) {
			uint8_t ramBank = (modeSelect ? bankReg2 : 0) & (numRamBanks - 1);
			return ramByteVector[ramBank * 0x2000 + (address - 0xA000)];
		}
		else {
			return 0xFF;
			}
		}
	else{
		return 0xFF;
	}

	return 0xFF;
}

void Cartridge::write(uint16_t address, uint8_t value) {
	if (address >= 0x0000 && address <= 0x1FFF) {
		ramEnabled = ((value & 0x0F) == 0x0A);
	}
	else if (address >= 0x2000 && address <= 0x3FFF) {
		romBankLow = value & 0x1F;
	}
	else if (address >= 0x4000 && address <= 0x5FFF) {
		bankReg2 = value & 0x03;
	}
	else if (address >= 0x6000 && address <= 0x7FFF) {
		modeSelect = value & 0x01;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (ramEnabled && numRamBanks > 0) {
			uint8_t ramBank = (modeSelect ? bankReg2 : 0) & (numRamBanks - 1);
			ramByteVector[ramBank * 0x2000 + (address - 0xA000)] = value;
		}
	}

}


std::string Cartridge::getCartridgeType() const{
	const std::unordered_map<uint8_t, std::string> cartridge_types = {
	{0x00, "ROM ONLY"},
	{0x01, "MBC1"},
	{0x02, "MBC1+RAM"},
	{0x03, "MBC1+RAM+BATTERY"},
	{0x05, "MBC2"},
	{0x06, "MBC2+BATTERY"},
	{0x08, "ROM+RAM"},
	{0x09, "ROM+RAM+BATTERY"},
	{0x0B, "MMM01"},
	{0x0C, "MMM01+RAM"},
	{0x0D, "MMM01+RAM+BATTERY"},
	{0x0F, "MBC3+TIMER+BATTERY"},
	{0x10, "MBC3+TIMER+RAM+BATTERY"},
	{0x11, "MBC3"},
	{0x12, "MBC3+RAM"},
	{0x13, "MBC3+RAM+BATTERY"},
	{0x19, "MBC5"},
	{0x1A, "MBC5+RAM"},
	{0x1B, "MBC5+RAM+BATTERY"},
	{0x1C, "MBC5+RUMBLE"},
	{0x1D, "MBC5+RUMBLE+RAM"},
	{0x1E, "MBC5+RUMBLE+RAM+BATTERY"},
	{0x20, "MBC6"},
	{0x22, "MBC7+SENSOR+RUMBLE+RAM+BATTERY"},
	{0xFC, "POCKET CAMERA"},
	{0xFD, "BANDAI TAMA5"},
	{0xFE, "HuC3"},
	{0xFF, "HuC1+RAM+BATTERY"}};

	auto x = cartridge_types.find(this->header.cartridgeType);

	if (x != cartridge_types.end()) {
		std::cout << "Cartridge Type: " << x->second << "\n";
		return x->second;
	}
	else { return "bad cart type";}
}

std::string Cartridge::getTitle() const {	
	std::string s;
	for (auto i : this->header.title) {
		if (i != '\0') {
			s += (i);
		}
	}
	std::cout << "TITLE = " << s << std::endl;
	return s;
}

//Cartridge::Cartridge():ramByteVector()
//{
//    // set appropriate RAM size for your cartridge
//    const size_t ramSize = 8192; // replace with correct size or a member variable
//    ramByteVector.resize(ramSize);
//}

