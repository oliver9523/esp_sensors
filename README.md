# esp_sensors
Classes and helpers for a sensor suite 

These files are to help manage multiple sensors attached to an ESP8266 with a multiplexer so that the single ADC on the ESP8266 can address multiple analogue sensors

The sensor management approach used here puts the MQTT responsibility on the sensor. 

**Why?**
Rather than filling the `.ino` with loads of functions and calls to them in the `loop()` and publish calls all over the place we make the sensor class do the work.

In this following example we create a sensor, configure it with mqtt settings, then we just have to call `update()` in the `loop()` function
```
BinarySensor switch_input;

void setup() {
  switch_input.Setup(mqtt_client);
  switch_input.Configure('0/garage/sensors/door', SW_INPUT_PIN, 1);
  switch_input.setRisingFallingMessage("opened", "closed");
  switch_input.onRising(turn_on);
  switch_input.onFalling(turn_off);
  switch_input.Settings.publish_on_change = true;
}

void turn_on() {
	//do something...
}

void turn_off() {
	//also do something...
}

void loop() {
	switch_input.update();
}

```

As we set `switch_input.Settings.publish_on_change = true;` this means the sensor class will publish the message "opened" with a rising digital input and "closed" when a falling digital edge is detected. These messages are published to the topic `0/garage/sensors/door`.
We can also set a callback using the `.onRising()` and `.onFalling()` functions.
