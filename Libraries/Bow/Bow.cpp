#include "Bow.h" //include the declaration for this class
const int stoppingThreshold = 10;
const int movingThreshold = 1;
const int movingCountThreshold = 2;

const int debounceTime = 5;
const int bowModes = 3;

// Bow Mode 0:Regular Bowing 
// Bow Mode 1:Regular Pluck
// Bow Mode 2:Special Bowing

Bow::Bow(){

}

Bow::~Bow(){
	delete modePin;
}

void Bow::begin(int _pinX,int _pinY,int _pinZ,int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin){
	pinX = _pinX;
	pinY = _pinY;
	pinZ = _pinZ;
	modePin = new Bounce(_modePin,debounceTime);

	modeStatusPins[0] = _modeStatusRPin;
	modeStatusPins[1] = _modeStatusGPin;
	modeStatusPins[2] = _modeStatusBPin;

	yAccValue = 0;
	constantCount = 0;
	mode = 1;
	moving = false;
	movingCount = 0;

	pinMode(modeStatusPins[0],OUTPUT);
	pinMode(modeStatusPins[1],OUTPUT);
	pinMode(modeStatusPins[2],OUTPUT);
	pinMode(_modePin,INPUT);
	updateModeStatusLight();
}

void Bow::update(){
	// Read y value from pin
	updateAccelerometer();
	if(updateModeButton()){
		updateModeStatusLight();
	}
}

bool Bow::updateModeButton(){
	if (modePin->update()) {
		if(modePin->read() == HIGH) {
			mode = (mode+1)%bowModes;
			Serial.print("Button Mode: ");
			Serial.println(mode);
			return true;
		}
	}
	return false;
}

void Bow::updateModeStatusLight(){
	switch(mode){
		case 0:
			setLights(modeStatusPins,1,0,0);
			break;
		case 1:
			setLights(modeStatusPins,0,1,0);
			break;
		case 2:
			setLights(modeStatusPins,0,0,1);
			break;
	}	
}

void Bow::setLights(int pins[3],bool red,bool green,bool blue){
	if(red && green && blue) {
		Serial.println("Error: Overloaded lights");
		return;
	}
	digitalWrite(pins[0],red);
	digitalWrite(pins[1],green);
	digitalWrite(pins[2],blue);
}

void Bow::updateAccelerometer(){
	int pinYReading = analogRead(pinY);

	if(abs(pinYReading-yAccValue) > movingThreshold && movingCount >= movingCountThreshold) {
		yAccValue = pinYReading;
		constantCount = 0;
		movingCount++;
		moving = true;
	} else if(abs(pinYReading-yAccValue) > movingThreshold){
		movingCount++;
	} else {
		constantCount++;
		if (constantCount >= stoppingThreshold) {
			moving = false;
			movingCount = 0;
		}
	}
}

bool Bow::playNote(bool stringPressed){
	if (stringPressed) {
		if(mode != 0) {
			return true;
		} else {
			return moving;
		}
	} else {
		return false;
	}
}