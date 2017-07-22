#include "Transpose.h" //include the declaration for this class

const int debounceTime = 5;
const int transposeModes = 4;

// Transpose Mode 0:Violin
// Transpose Mode 1:Viola
// Transpose Mode 2:Cello
// Transpose Mode 3:Double Bass

Transpose::Transpose(){

}

Transpose::~Transpose(){
	delete modePin;
}

/**
  	* @desc initializes Transpotion class
    * @param int _modePin - Teensy pin corresponding to Transpotion Mode 
    * @param int _modeStatusRPin - Teensy pin corresponding to Red Status Light
    * @param int _modeStatusGPin - Teensy pin corresponding to Blue Status Light
    * @param int _modeStatusBPin - Teensy pin corresponding to Green Status Light
*/
void Transpose::begin(int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin){
	//initialize mode button with 5ms debounce time
	modePin = new Bounce(_modePin,debounceTime);

	//initialize status pins
	modeStatusPins[0] = _modeStatusRPin;
	modeStatusPins[1] = _modeStatusGPin;
	modeStatusPins[2] = _modeStatusBPin;

	//set initial mode to Cello
	mode = 2;

	//initialize Mode Status Pins as Digital Outputs
	pinMode(modeStatusPins[0],OUTPUT);
	pinMode(modeStatusPins[1],OUTPUT);
	pinMode(modeStatusPins[2],OUTPUT);

	//initialize Transposition Mode Button as Digital Input
	pinMode(_modePin,INPUT);

	//Update light status
	updateModeStatusLight();
}

/**
  	* @desc update status of all class properties in Transpose
*/
void Transpose::update(){
	// Read y value from pin
	if(updateModeButton()){
		updateModeStatusLight();
	}
}

/**
  	* @desc get status of Transposition Mode Pin
  	* @return bool - true if Transposition Mode has changed, false if same
*/
bool Transpose::updateModeButton(){
	if (modePin->update()) {
		if(modePin->read() == HIGH) {
			mode = (mode+1)%transposeModes;
			Serial.print("Button Mode: ");
			Serial.println(mode);
			return true;
		}
	}
	return false;
}

/**
  	* @desc set Mode Status lights based on the class variable mode
*/
void Transpose::updateModeStatusLight(){
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
		case 3:
			setLights(modeStatusPins,1,1,0);
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
void Transpose::setLights(int pins[3],bool red,bool green,bool blue){
	if(red && green && blue) {
		Serial.println("Error: Overloaded lights");
		return;
	}
	digitalWrite(pins[0],red);
	digitalWrite(pins[1],green);
	digitalWrite(pins[2],blue);
}

/**
  	* @desc output what note value should be played
  	* @param int string - current string (1 - lowest string, 4 - highest string)
  	* @return int - MIDI note value that should be played
*/
int Transpose::noteValue(int string){
	switch(mode) {
		case 0:
			return violin(string);
		case 1:
			return viola(string);
		case 2:
			return cello(string);
		case 3:
			return doubleBase(string);
		default:
			return 0;
	}
}

/**
  	* @desc output what note value should be played if mode is violin
  	* @param int string - current string (1 - lowest string, 4 - highest string)
  	* @return int - MIDI note value that should be played
*/
int Transpose::violin(int string){
	switch (string) {
	    case 1:
	      return 55+6;
	      break;
	    case 2:
	      return 62+6;
	      break;
	    case 3:
	      return 69+6;
	      break;
	    case 4:
	      return 76+6;
	      break;
	    default:
	      return 0;
  	}
}

/**
  	* @desc output what note value should be played if mode is viola
  	* @param int string - current string (1 - lowest string, 4 - highest string)
  	* @return int - MIDI note value that should be played
*/
int Transpose::viola(int string){
	switch (string) {
	    case 1:
	      return 48+6;
	      break;
	    case 2:
	      return 55+6;
	      break;
	    case 3:
	      return 62+6;
	      break;
	    case 4:
	      return 69+6;
	      break;
	    default:
	      return 0;
  	}
}

/**
  	* @desc output what note value should be played if mode is cello
  	* @param int string - current string (1 - lowest string, 4 - highest string)
  	* @return int - MIDI note value that should be played
*/
int Transpose::cello(int string){
	switch (string) {
	    case 1:
	      return 36+6;
	      break;
	    case 2:
	      return 43+6;
	      break;
	    case 3:
	      return 50+6;
	      break;
	    case 4:
	      return 57+6;
	      break;
	    default:
	      return 0;
  	}
}

/**
  	* @desc output what note value should be played if mode is double base
  	* @param int string - current string (1 - lowest string, 4 - highest string)
  	* @return int - MIDI note value that should be played
*/
int Transpose::doubleBase(int string){
	switch (string) {
	    case 1:
	      return 24+6;
	      break;
	    case 2:
	      return 31+6;
	      break;
	    case 3:
	      return 38+6;
	      break;
	    case 4:
	      return 45+6;
	      break;
	    default:
	      return 0;
  	}
}

