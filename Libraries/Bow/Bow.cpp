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

/**
  	* @desc initializes Bow class
  	* @param int _pinX - Teensy pin corresponding to Acceleration in X-Direction
	* @param int _pinY - Teensy pin corresponding to Acceleration in Y-Direction
    * @param int _pinZ - Teensy pin corresponding to Acceleration in Z-Direction
    * @param int _modePin - Teensy pin corresponding to Bow Mode 
    * @param int _modeStatusRPin - Teensy pin corresponding to Red Status Light
    * @param int _modeStatusGPin - Teensy pin corresponding to Blue Status Light
    * @param int _modeStatusBPin - Teensy pin corresponding to Green Status Light
*/
void Bow::begin(int _pinX,int _pinY,int _pinZ,int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin){
	pinX = _pinX;
	pinY = _pinY;
	pinZ = _pinZ;

	//initialize mode button with 5ms debounce time
	modePin = new Bounce(_modePin,debounceTime);

	//initialize status pins
	modeStatusPins[0] = _modeStatusRPin;
	modeStatusPins[1] = _modeStatusGPin;
	modeStatusPins[2] = _modeStatusBPin;

	yAccValue = 0;
	constantCount = 0;
	mode = 1;
	moving = false;
	movingCount = 0;

	//initialize Bow Mode Status Pins as Digital Outputs
	pinMode(modeStatusPins[0],OUTPUT);
	pinMode(modeStatusPins[1],OUTPUT);
	pinMode(modeStatusPins[2],OUTPUT);

	//initialize Bow Mode Button as Digital Input
	pinMode(_modePin,INPUT);

	//Update light status
	updateModeStatusLight();
}

/**
  	* @desc update status of all class properties in Bow
*/
void Bow::update(){
	updateAccelerometer();
	if(updateModeButton()){
		updateModeStatusLight();
	}
}

/**
  	* @desc get status of Bow Mode Pin
  	* @return bool - true if Bow Mode has changed, false if same
*/
bool Bow::updateModeButton(){
	if (modePin->update()) {
		if(modePin->read() == HIGH) {
			mode = (mode+1)%bowModes;
			return true;
		}
	}
	return false;
}

/**
  	* @desc set Mode Status lights based on the class variable mode
*/
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

/**
  	* @desc set Mode Status lights based RGB value
  	* @param int pins[3] - array of pins corresponding to light Pins on Teensy
  	* @param bool red - status of red light
  	* @param bool green - status of green light
  	* @param bool blue - status of blue light
*/
void Bow::setLights(int pins[3],bool red,bool green,bool blue){
	if(red && green && blue) {
		Serial.println("Error: Overloaded lights");
		return;
	}

	digitalWrite(pins[0],red);
	digitalWrite(pins[1],green);
	digitalWrite(pins[2],blue);
}

/**
  	* @desc update accelerometer reading
*/
void Bow::updateAccelerometer(){
	int pinYReading = analogRead(pinY);

	//Has the value of the accelerometer changed above the threshold of moving and is the threshold of number of times seen moving greater than the threshold?
	if(abs(pinYReading-yAccValue) > movingThreshold && movingCount >= movingCountThreshold) {
		//Set the bow to moving
		yAccValue = pinYReading;
		constantCount = 0;
		movingCount++;
		moving = true;
	} else if(abs(pinYReading-yAccValue) > movingThreshold){ //If the change in accelerometer is greater than changing threshold, increase count of number of moves detected
		movingCount++;
	} else {
		//Accelerometer readings has not signifigantly changed
		constantCount++; 
		if (constantCount >= stoppingThreshold) { //if the bow has not moved for a signifigant number of readings, updated bowMoving to false
			moving = false;
			movingCount = 0;
		}
	}
}

/**
  	* @desc output whether the bow should play a note or note
  	* @param bool stringPressed - is the string current being played
  	* @return bool - true for note should be played, false for note should not be played
*/
bool Bow::playNote(bool stringPressed){
	if (stringPressed) {
		if(mode != 0) { //if bow is not on classical bowing mode - ignore readings of accelerometer and return true
			return true;
		} else {
			return moving;
		}
	} else {
		return false;
	}
}