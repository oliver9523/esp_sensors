
#pragma once
// MuxManager.h

#ifndef _MUX_MANAGER_h
#define _MUX_MANAGER_h

#include "arduino.h"

//https://assets.nexperia.com/documents/data-sheet/74HC_HCT4051.pdf

class MuxManager
{
public:
	int8_t MUX_LIMIT;
	int8_t ActiveAddress;

	//mux address pins
	int8_t S0;
	int8_t S1;
	int8_t S2;
	int8_t S3;
	
	//mux chip enable pin
	int8_t ENABLE = -1;

	MuxManager();
	MuxManager(int8_t n_mux);

	void setPins(int8_t _0, int8_t _1 = -1, int8_t _2 = -1, int8_t _3 = -1);
	bool setMuxAddress(int8_t index);
	int analogReadMux(int8_t A2DPin, int8_t mux_index);
	int digitalReadMux(int8_t A2DPin, int8_t mux_index);
	int analogReadNext(int8_t A2DPin);
	void setNumberOfDevices(int8_t limit);
	void SetEnable(bool enable);
	void SetEnablePin(int8_t pin);

private:

};

#endif