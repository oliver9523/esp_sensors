#include "arduino.h"
#include "Sensor.hpp"

class AnalogSensor : public Sensor {
public:
#define smooth_window 10

	//calibrated each time
	int noise_floor = 0;
	float latest_reading = 0;
	float smoothed_readings[smooth_window];
	float max_reading = 0;
	float min_reading = MAXFLOAT;
	float (*converter)(float in);

	//save/load
	//float upper_threshold = -1;
	//float lower_threshold = -1;
	//float dead_zone = 0;

	//===============================
	//  Converting ADC to voltage
	//===============================
	bool scale_to_volts = true;
	float adc_resolution = 1024;	//10-bit adc
	float adc_ref_voltage = 3.3;	//Wemos D1	0-3.3v, ESP8266	0-1v
	float reading_to_volts;


	void SetConversion(float (*fn)(float in)) {
		converter = fn;
	}

	float apply_conversion(float val) {
		return (*converter)(val);
	}



	AnalogSensor() {
		Settings.sensor_type = SENSOR_TYPE::ANALOG;
		reading_to_volts = adc_ref_voltage / adc_resolution;
		for (byte i = 0; i < smooth_window; i++)
			smoothed_readings[i] = 0;
	}

	void Configure(int8_t _pin) {
		Settings.pin = _pin;
		Settings.address = -1;
		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
		pinMode(Settings.pin, INPUT);
	}

	void Configure(int8_t _pin, ulong _address) {
		Settings.pin = _pin;
		Settings.address = _address;
		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
		pinMode(Settings.pin, INPUT);
	}

	void setThresholds(float _lower, float _upper, float _deadzone) {
		Settings.upper_threshold = _upper;
		Settings.lower_threshold = _lower;
		Settings.dead_zone = _deadzone;
	}

	void onUpperThreshold(void (*callback)(void)) {
		upper_callbackfn = callback;
	}

	void onLowerThreshold(void (*callback)(void)) {
		lower_callbackfn = callback;
	}

	void trigger_onupper() {
		if (current_threshold_zone == TRIGGER_ZONE::UPPER)
			return;
		if (upper_callbackfn)
			(*upper_callbackfn)();
	}

	void trigger_onlower() {
		if (current_threshold_zone == TRIGGER_ZONE::LOWER)
			return;
		if (lower_callbackfn)
			(*lower_callbackfn)();
	}

	void check_thresholds(float value) {
		//TODO - rethink this - it got stuck ON when the upper threshold was changed

		if (Settings.upper_threshold == -1 || Settings.lower_threshold == -1)
			return;

		if ((Settings.upper_threshold + Settings.dead_zone) < value) {
			//in upper
			trigger_onupper();
			current_threshold_zone = TRIGGER_ZONE::UPPER;
		}

		if (((Settings.lower_threshold + Settings.dead_zone) < value) && (value <= (Settings.upper_threshold - Settings.dead_zone))) {
			//in middle
			current_threshold_zone = TRIGGER_ZONE::NONE;
		}

		if (value < (Settings.lower_threshold - Settings.dead_zone)) {
			//in lower
			trigger_onlower();
			current_threshold_zone = TRIGGER_ZONE::LOWER;
		}
	}

	void auto_thresh(float percentage) {
		//fn expects it in the range 0-50
		if (percentage < 1)
			percentage *= 100;

		//50% won't work as there won't be a gap between thresholds
		if (percentage > 49)
			return;

		float range = max_reading - min_reading;
		float auto_level_offset = range * (percentage / 100);

		Settings.upper_threshold = max_reading - auto_level_offset;
		Settings.lower_threshold = min_reading + auto_level_offset;

		Settings.dead_zone = auto_level_offset / 2;
	}

	void reset_limits() {
		max_reading = 0;
		min_reading = MAXFLOAT;
	}

	void update_limits() {
		max_reading = _max(latest_reading, max_reading);
		min_reading = _min(latest_reading, min_reading);
	}

	void update() {
		if (Settings.pin == -1) return;

		if (Settings.address != -1) {
			latest_reading = (float)mux_manager.analogReadMux(Settings.pin, Settings.address);
		}
		else {
			latest_reading = (float)analogRead(Settings.pin);
		}

		if (scale_to_volts)
			latest_reading *= reading_to_volts;

		read_count++;

		//offset the ADC noise to make 0v = 0
		//latest_reading = _max(0, latest_reading - noise_floor);
		//latest_reading = map(latest_reading, 0, 1024-noise_floor, 0, 1023);

		//size_t idx = read_count % smooth_window;
		smoothed_readings[read_count % smooth_window] = latest_reading;

		//message = String(latest_reading);
		message = String(get_smoothed_converted());
		check_thresholds(get_smoothed_converted());
		update_limits();
	}

	bool update_and_publish() {
		if (Settings.pin == -1) return false;
		update();
		publish();
	}

	MAPPINGS str2enum(String _str) {
		if (_str == "none")	return MAPPINGS::NO_MAPPING;
		if (_str == "tmp36")		return MAPPINGS::TMP36;
		if (_str == "battery")		return MAPPINGS::BATTERY;
		if (_str == "norm3v")		return MAPPINGS::NORM_3V;
		if (_str == "inv_norm3v")	return MAPPINGS::INV_NORM_3V;
	}

	String enum2str(MAPPINGS _mapping) {
		switch (_mapping)
		{
		case NO_MAPPING:
			return "none";
		case TMP36:
			return "tmp36";
		case BATTERY:
			return "battery";
		case NORM_3V:
			return "norm3v";
		case INV_NORM_3V:
			return "inv_norm3v";
		default:
			return "none";
		}
	}

	String info() {
		String Output;
		Output += "Name : " + String(Settings.name) + "<br>";
		Output += "Address : " + String(Settings.address) + "<br>";
		Output += "Type : Analog <br>";
		Output += "Mapping fn : " + enum2str(Settings.mapping) + "<br>";
		Output += "--------- <br>";
		Output += "Voltage = " + String(latest_reading) + "<br>";

		Output += "Mapped = " + String(get_smoothed_converted());
		Output += " (min = " + String(min_reading) + ", max = " + String(max_reading) + ", low_t = " + String(Settings.lower_threshold);
		Output += ", upper_t = " + String(Settings.upper_threshold) + ", deadzone = " + String(Settings.dead_zone) + ") <br>";

		Output += "State : ";
		if (current_threshold_zone == TRIGGER_ZONE::UPPER)
			Output += String("Upper");
		else if (current_threshold_zone == TRIGGER_ZONE::LOWER)
			Output += String("Lower");
		else if (current_threshold_zone == TRIGGER_ZONE::NONE)
			Output += String("None");
		Output += "<br>";

		Output += "Topic : " + String(Settings.topic) + "<br>";
		Output += "Publish rate : " + String(Settings.publish_rate) + "<br>";

		return Output;
	}

	float smoothed_reading() {
		float sum = 0;
		for (byte i = 0; i < smooth_window; i++)
			sum += smoothed_readings[i];
		return sum / smooth_window;
	}

	float get_smoothed_converted() {
		if (converter == nullptr)
			return smoothed_reading();
		return apply_conversion(smoothed_reading());
	}

private:
	void (*lower_callbackfn)(void);
	void (*upper_callbackfn)(void);
	void (*test_callbackfn)(int);
	TRIGGER_ZONE current_threshold_zone = TRIGGER_ZONE::NONE;
};