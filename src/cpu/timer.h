#pragma once
#include <cstdint> 

class timer {
private:
	
	uint8_t TIMA = { 0 };
	uint8_t TMA = { 0 };
	uint8_t TAC = { 0 };

	uint16_t counter = { 0 };

	bool handleOverflow(uint8_t old_bit, uint8_t N);
	uint8_t getSelectedBit();

public:
	bool tick(uint8_t cycles);

	uint8_t getTIMA();
	void setTIMA(uint8_t value);

	uint8_t getTMA() ;
	void setTMA(uint8_t value);

	uint8_t getTAC();
	void setTAC(uint8_t value);

	uint8_t getDIV();

	bool handleDIV();
	
};