#include "arduino.h"
#include "Sensor.hpp"

class BinarySensor : public Sensor {
public:
	int latest_reading = 0;
	bool first_update = true;
	bool changed;
	//bool publish_on_change = true;
	//can't write string to eeprom
	//String rising = "rising";
	//String falling = "falling";

	BinarySensor() {
		message = "idle";
		Settings.sensor_type = SENSOR_TYPE::DIGITAL;
	}

	void Configure(String _topic, int8_t _pin, ulong _publish_rate) {
		Settings.publish_rate = _publish_rate;
		//topic = _topic;
		_topic.toCharArray(Settings.topic, 64);
		Settings.pin = _pin;
		Settings.address = -1;

		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
	}

	void Configure(String _topic, int8_t _pin, int8_t _address, ulong _publish_rate) {
		Settings.publish_rate = _publish_rate;
		//topic = _topic;
		_topic.toCharArray(Settings.topic, 64);
		Settings.pin = _pin;
		Settings.address = _address;

		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
	}

	void setRisingFallingMessage(String _rising, String _falling) {
		//rising = _rising;
		//falling = _falling;
		_rising.toCharArray(Settings.rising_message, 64);
		_falling.toCharArray(Settings.falling_message, 64);
	}

	void onChange(void (*callback)(void)) {
		onchange_callbackfn = callback;
	}

	void onRising(void (*callback)(void)) {
		rising_callbackfn = callback;
	}

	void onFalling(void (*callback)(void)) {
		falling_callbackfn = callback;
	}

	void update() {
		if (Settings.pin == -1) return;

		int this_reading;
		if (Settings.address != -1) {
			this_reading = mux_manager.digitalReadMux(Settings.pin, Settings.address);
		}
		else {
			this_reading = digitalRead(Settings.pin);
		}

		//if it's the first update we don't know if it's a true rising/falling edge
		if (first_update) {
			latest_reading = this_reading;
			first_update = false;
			return;
		}

		//compare previous reading against
		changed = (latest_reading != this_reading);

		if (changed) {
			if (this_reading > latest_reading) {			//rising edge
				message = String(Settings.rising_message);
				trigger_onrising();
			}
			else if (latest_reading > this_reading) {		//falling edge
				message = String(Settings.falling_message);
				trigger_onfalling();
			}
			trigger_onchange();
			if (Settings.publish_on_change) {
				//todo - might need to rate limit this...
				publish(true);
			}
		}

		latest_reading = this_reading;
	}

	void trigger_onchange() {
		if (onchange_callbackfn)
			(*onchange_callbackfn)();
	}

	void trigger_onrising() {
		if (rising_callbackfn)
			(*rising_callbackfn)();
	}

	void trigger_onfalling() {
		if (falling_callbackfn)
			(*falling_callbackfn)();
	}

	bool update_and_publish() {
		if (Settings.pin == -1) return false;
		update();
		publish();
	}

	String info() {
		String Output;
		Output += String("Type: Binary - ");
		Output += String("current = ");
		Output += String(latest_reading);
		Output += String(" (changed = ");
		Output += String(changed);
		Output += String(")");
		return Output;
	}

private:
	void (*onchange_callbackfn)(void);
	void (*rising_callbackfn)(void);
	void (*falling_callbackfn)(void);
};
