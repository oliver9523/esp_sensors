#include "arduino.h"

class RateLimiter {
public:
	ulong last_triggered = 0;
	ulong start_time = 0;
	ulong rate = 1000;	//default to 1000ms

	RateLimiter() {
		start_time = millis();
	}

	RateLimiter(ulong _rate) {
		start_time = millis();
		rate = _rate;
	}

	bool ready() {
		if (millis() - last_triggered > rate) {
			last_triggered = millis();
			return true;
		}
		return false;
	}
};