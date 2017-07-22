#ifndef Bow_H
#define Bow_H
 
#include <Arduino.h>
#include <Bounce.h>

class Bow {
public:
	Bow();
	~Bow();
	void begin(int _pinX,int _pinY,int _pinZ,int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin);
	void update();
	bool playNote(bool stringPressed);
private:
	//Pins
	int pinX;
	int pinY;
	int pinZ;
	Bounce *modePin;
	int modeStatusPins[3];

	//Status variables
	int yAccValue;
	int constantCount;
	int mode;
	bool moving;
	int movingCount;

	void updateAccelerometer();
	bool updateModeButton();
	void updateModeStatusLight();
	void setLights(int pins[3],bool red,bool green,bool blue);
};

#endif