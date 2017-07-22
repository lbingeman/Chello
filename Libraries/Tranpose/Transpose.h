#ifndef Transpose_H
#define Transpose_H
 
#include <Arduino.h>
#include <Bounce.h>

class Transpose {
public:
	Transpose();
	~Transpose();
	void begin(int _modePin,int _modeStatusRPin,int _modeStatusGPin,int _modeStatusBPin);
	void update();
	int noteValue(int string);
private:
	//Pins
	Bounce *modePin;
	int modeStatusPins[3];

	//Status variables
	int mode;

	bool updateModeButton();
	void updateModeStatusLight();
	void setLights(int pins[3],bool red,bool green,bool blue);
	int violin(int string);
	int viola(int string);
	int cello(int string);
	int doubleBase(int string);
};

#endif