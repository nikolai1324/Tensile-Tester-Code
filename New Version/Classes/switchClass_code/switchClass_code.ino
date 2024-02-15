#include <Arduino.h>
//#define BUTTON_PIN  3

class Button {
	private: //declare private variables
	byte pin;
	byte state;
	byte lastReading;
	unsigned long lastDebounceTime = 0; //unsigned for vars > 0
	unsigned long debounceDelay = 50;
	public:
	Button(byte pin) { //object constructor
		this->pin = pin;
		lastReading = LOW;
		init();
	}
	void init() { //methods
		pinMode(pin, INPUT);
		update();
	}
	void update() {
		byte newReading = digitalRead(pin);
		if (newReading != lastReading) {
			lastDebounceTime = millis();
		}
		if (millis() - lastDebounceTime > debounceDelay) {
			state = newReading; //current state assigned
		}
		lastReading = newReading;
	}
	byte getState() {
		update();
		return state; //return the state
	}
	bool isPressed() {
		return (getState() == HIGH);
	}
};

/*
void setup() {
  Serial.begin(9600);
}

Button button1(BUTTON_PIN);
void loop() {
	if (button1.isPressed()) {
    Serial.print("Button was not Pressed");
	}
	else {
		Serial.print("Button was not Pressed");
		delay(1000);
	}
}
*/