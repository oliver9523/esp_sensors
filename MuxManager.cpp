#include "MuxManager.h"

MuxManager::MuxManager() {
	//default to 8
	MUX_LIMIT = 8;
}

MuxManager::MuxManager(int8_t n_mux) {
	MUX_LIMIT = n_mux;
}

void MuxManager::setPins(int8_t _0, int8_t _1, int8_t _2, int8_t _3) {
	S0 = _0;
	S1 = _1;
	S2 = _2;
	S3 = _3;

	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
}

void MuxManager::setNumberOfDevices(int8_t limit) {
	MUX_LIMIT = limit;
}

bool MuxManager::setMuxAddress(int8_t index) {
	if (index >= MUX_LIMIT)
		return false;

	int8_t r0 = bitRead(index, 0);
	int8_t r1 = bitRead(index, 1);
	int8_t r2 = bitRead(index, 2);
	int8_t r3 = bitRead(index, 3);

	if (MUX_LIMIT >= 2)
		digitalWrite(S0, r0);
	
	if (MUX_LIMIT >= 4)
		digitalWrite(S1, r1);

	if (MUX_LIMIT >= 8)
		digitalWrite(S2, r2);

	if (MUX_LIMIT == 16)
		digitalWrite(S3, r3);

	//wait 5ms to allow multiplexing chip to switch
	//switching should be on the ns timescale... 
	//but without a delay I've seen the same channel being read without switching
	delay(50);	//50ms lets things settle
	ActiveAddress = index;
	return true;
}

int MuxManager::analogReadMux(int8_t A2DPin, int8_t mux_index) {
	//change the mux address
	if (setMuxAddress(mux_index)) {
		//analogRead(A2DPin);
		return analogRead(A2DPin);
	}

	//if settning mux address fails return -1
	return -1;
}

int MuxManager::digitalReadMux(int8_t Pin, int8_t mux_index) {
	//change the mux address
	if (setMuxAddress(mux_index)) {
		return digitalRead(Pin);
	}

	//if settning mux address fails return -1
	return -1;
}

int MuxManager::analogReadNext(int8_t A2DPin) {
	ActiveAddress = (ActiveAddress + 1) % MUX_LIMIT;
	//change the mux address
	if (setMuxAddress(ActiveAddress))
		return analogRead(A2DPin);
	//if settning mux address fails return -1
	return -1;
}

void MuxManager::SetEnable(bool enable) {
	//assumes active-low
	if (ENABLE == -1) return;
	digitalWrite(ENABLE, enable ? LOW : HIGH);
}

void MuxManager::SetEnablePin(int8_t pin) {
	ENABLE = pin;
	pinMode(ENABLE, OUTPUT);
}