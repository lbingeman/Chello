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

void Transpose::begin(int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin){
	modePin = new Bounce(_modePin,debounceTime);

	modeStatusPins[0] = _modeStatusRPin;
	modeStatusPins[1] = _modeStatusGPin;
	modeStatusPins[2] = _modeStatusBPin;

	mode = 2;

	pinMode(modeStatusPins[0],OUTPUT);
	pinMode(modeStatusPins[1],OUTPUT);
	pinMode(modeStatusPins[2],OUTPUT);
	pinMode(_modePin,INPUT);
	updateModeStatusLight();
}

void Transpose::update(){
	// Read y value from pin
	if(updateModeButton()){
		updateModeStatusLight();
	}
}

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

void Transpose::setLights(int pins[3],bool red,bool green,bool blue){
	if(red && green && blue) {
		Serial.println("Error: Overloaded lights");
		return;
	}
	digitalWrite(pins[0],red);
	digitalWrite(pins[1],green);
	digitalWrite(pins[2],blue);
}

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

