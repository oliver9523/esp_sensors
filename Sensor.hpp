#include "arduino.h"
#include "helpers.h"

#include "MuxManager.h"
#include <PubSubClient.h>

#include "helpers.h"


struct SensorSetting {
	//Used to serialise and deserialise the sensor settings to read/write to EEPROM

	//generic
	ulong publish_rate;			//8 bytes
	int8_t pin;					//1 byte
	int8_t address;				//1 byte
	SENSOR_TYPE	sensor_type;	//4 bytes
	char name[64];				//64 bytes
	char topic[64];				//64 bytes
	MAPPINGS mapping;			//4 bytes

	//analog
	float upper_threshold;		//4 bytes
	float lower_threshold;		//4 bytes
	float dead_zone;			//4 bytes

	//digital
	char rising_message[64];	//64 bytes
	char falling_message[64];	//64 bytes
	bool publish_on_change;		//1 byte


	SensorSetting()
	{
		//default init

		//generic sensor
		publish_rate = SECONDS(60);
		pin = A0;
		address = 0;
		sensor_type = SENSOR_TYPE::NOTSET;
		mapping = MAPPINGS::NO_MAPPING;

		String("sensor name not set").toCharArray(name, sizeof(name) - 1);
		String("mqtt topic not set").toCharArray(topic, sizeof(topic) - 1);
		//String("mqtt message not set").toCharArray(message, sizeof(message) - 1);
		String("rising").toCharArray(rising_message, sizeof(rising_message) - 1);
		String("falling").toCharArray(falling_message, sizeof(falling_message) - 1);

		//analog sensor
		upper_threshold = -1;
		lower_threshold = -1;
		dead_zone = -1;

		//digital
		publish_on_change = true;
	}
};


class Sensor {
public:

	SensorSetting Settings;

	ulong since_last_published = 0;
	String message = "";
	String sensor_error_topic = "devices/sensor/error";

	MuxManager mux_manager;
	PubSubClient mqtt_client;
	//MuxManager* mm = new MuxManager;
	//float latest_reading = 0;
	uint64 read_count = 0;

	bool ready_to_save = false;

	Sensor() {

	}

	void Setup(MuxManager& mux_manager, PubSubClient& mqtt_client) {
		this->mux_manager = mux_manager;
		this->mqtt_client = mqtt_client;
	}

	void Setup(PubSubClient& mqtt_client) {
		this->mqtt_client = mqtt_client;
	}

	void Setup(MuxManager& mux_manager) {
		//mm = nullptr;
		//if (mm == nullptr)
		this->mux_manager = mux_manager;
	}

	void Configure(int8_t _pin, SENSOR_TYPE _type) {
		Settings.pin = _pin;
		Settings.address = -1;
		Settings.sensor_type = _type;

		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
	}

	void Configure(int8_t _pin, int8_t _address, SENSOR_TYPE _type) {
		Settings.pin = _pin;
		Settings.address = _address;
		Settings.sensor_type = _type;

		//add a random time offset so several sensors don't try to public at the same time
		since_last_published = random(1, Settings.publish_rate);
	}

	void ConfigureMQTTSettings(String _topic, ulong _publish_rate) {
		Settings.publish_rate = _max(_publish_rate, 100);	//limit to 100ms
		_topic.toCharArray(Settings.topic, 64);
	}

	bool publish(bool now = false) {
		//topic has to be set to publish and rate has to be > 0
		if (String(Settings.topic).length() == 0 || Settings.publish_rate <= 0)
			return false;

		if (now)
			return mqtt_client.publish(Settings.topic, message.c_str(), true);

		if (millis() - since_last_published > Settings.publish_rate) {
			since_last_published = millis();
			return publish(true);
		}
	}

	bool sensor_error(String error) {
		return mqtt_client.publish(sensor_error_topic.c_str(), error.c_str(), true);
	}

	//todo
	//void load(SensorSetting _settings) {
	//	Settings = _settings;
	//}
};
