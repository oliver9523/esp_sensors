#include "helpers.h"
#include "arduino.h"
#include "MuxManager.h"
#include <PubSubClient.h>

class TimedTrigger {
public:
	STATE CurrentState = STATE::OFF;
	STATE TargetState = STATE::OFF;
	ulong trigger_duration = SECONDS(5);
	ulong idle_duration = SECONDS(60);
	ulong last_triggered = 0;
	bool active_high = true;
	int8_t pin;

	bool trigger_once = false;
	ulong single_trigger_duration = 0;

	MuxManager mux_manager;
	PubSubClient mqtt_client;

	void Setup(MuxManager& _mux_manager, PubSubClient& _mqtt_client) {
		mux_manager = _mux_manager;
		mqtt_client = _mqtt_client;
	}

	void Setup(PubSubClient& _mqtt_client) {
		mqtt_client = _mqtt_client;
	}

	void Setup(MuxManager& _mux_manager) {
		mux_manager = mux_manager;
	}

	TimedTrigger() {
		last_triggered = millis();
	}

	void Configure(int8_t _pin, bool _active_high, ulong _trigger_duration, ulong _idle_duration) {
		pin = _pin;
		active_high = _active_high;
		trigger_duration = _trigger_duration;
		idle_duration = _idle_duration;
		pinMode(pin, OUTPUT);
	}

	void Update() {
		if (trigger_once) {
			if (elapsed() > single_trigger_duration) {
				trigger_once = false;
				Off();
				return;
			}
			return;
		}

		if (elapsed() > idle_duration&& TargetState == ON && CurrentState == OFF) {
			On();
			return;
		}

		if (elapsed() > trigger_duration&& TargetState == ON && CurrentState == ON) {
			Off();
			return;
		}
	}

	ulong elapsed() {
		return millis() - last_triggered;
	}

	void Trigger() {
		TargetState = STATE::ON;
		On();
	}

	void TriggerOnce(ulong duration) {
		single_trigger_duration = duration;
		trigger_once = true;
		TargetState = STATE::ON;
		On();
	}

	void Idle() {
		TargetState = STATE::OFF;
		Off();
	}

	void On() {
		digitalWrite(pin, active_high ? HIGH : LOW);
		last_triggered = millis();
		CurrentState = STATE::ON;
		Serial.println("Trigger ON");
	}

	void Off() {
		digitalWrite(pin, active_high ? LOW : HIGH);
		CurrentState = STATE::OFF;
		last_triggered = millis();
		Serial.println("Trigger OFF");
	}

};
