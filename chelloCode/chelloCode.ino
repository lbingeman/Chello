#include <Bow.h>
#include "Adafruit_MPR121.h"
#include <Transpose.h>

#define pitchPinS4 A9
#define pitchPinS3 A8
#define pitchPinS2 A7
#define pitchPinS1 A6
#define accXPin A16
#define accYPin A17
#define accZPin A18
#define transposeStatusRPin 5
#define transposeStatusGPin 6
#define transposeStatusBPin 7
#define transpositionPin 24
#define bowModePin 25
#define bowStatusRPin 8
#define bowStatusGPin 9
#define bowStatusBPin 10


Adafruit_MPR121 stringSensor = Adafruit_MPR121();
int stringMapping[4] = {2,3,4,5}; 

uint16_t lastStringTouched = 0;
uint16_t currStringTouched = 0;

int pitchPins[4] = {pitchPinS1,pitchPinS2,pitchPinS3,pitchPinS4};

float prevRead[3] = {0,0,0};//x,y,z
float prevVelocity[3] = {0,0,0};
int prevTime = 0;
int currIndex = 0;

int measureAcceleration = 0;
int accCycles = 1;

int delayTime = 1;

bool calibrated = false;

float xCalibration = 654*3.3/1023.0;
float yCalibration = 638*3.3/1023.0;

bool notePlayed[4] = {0,0,0,0};

Bow bow = Bow();
Transpose transpose = Transpose();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  bow.begin(accXPin,accYPin,accZPin,bowModePin,bowStatusRPin,bowStatusGPin,bowStatusBPin);
  transpose.begin(transpositionPin,transposeStatusRPin,transposeStatusGPin,transposeStatusBPin);
  
  usbMIDI.sendProgramChange(42, 1);
  usbMIDI.sendProgramChange(42, 2);
  usbMIDI.sendProgramChange(42, 3);
  usbMIDI.sendProgramChange(42, 4);

  //Adafruit Setup (check if sensor is connected and start)
  Serial.println("Connected!");
  if (!stringSensor.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring.");
    while (1);
  }
  Serial.println("MPR121 found!");
}

int getPitchIncrement(int string) {
  int resistanceReading = analogRead(pitchPins[string-1]);
  int pitchBendResistance = resistanceReading;
  
  if (pitchBendResistance < 15) {
    pitchBendResistance = 0;
  }

  int pitchBendAmount = map(pitchBendResistance,0,1023,0,8192*2);
  
  return pitchBendAmount;
}

void playString(int string)
{
  int velocity = 100;
  int baseNote = transpose.noteValue(string);
  int pitchBend = getPitchIncrement(string);

  Serial.print(string);
  Serial.print('\t');
  Serial.print(pitchBend);
  Serial.print('\t');
  
  int stringPort = stringMapping[string-1];
  bool stringTouched = (currStringTouched & _BV(stringPort));
  bool stringPreviouslyTouched = (lastStringTouched & _BV(stringPort));
  
  bool bowMoving = bow.playNote(stringTouched);
  
  if (((stringTouched && !stringPreviouslyTouched) && bowMoving) || (bowMoving && !notePlayed[string-1])) {
    usbMIDI.sendPitchBend(8192*2,string);
    usbMIDI.sendNoteOn(baseNote, velocity, string); // Turn the note ON
    Serial.print("Turned on note: ");
    Serial.println(string);
    notePlayed[string-1] = true;
  } else if (stringTouched){
    usbMIDI.sendPitchBend(pitchBend,string);
  }
  else if ((!stringTouched && stringPreviouslyTouched) || (stringTouched && !bowMoving)) {
    usbMIDI.sendNoteOff(baseNote, velocity, string); // Turn the note Off
    Serial.print("Turned off note: ");
    Serial.println(string);
    notePlayed[string-1] = false;
  }
}

void loop() {
  bow.update();
  transpose.update();
 
  // put your main code here, to run repeatedly:
  currStringTouched = stringSensor.touched(); //get currently touched strings

  for (int i = 1; i <= 4; i++) {
    playString(i);
  }

  lastStringTouched = currStringTouched;
  Serial.println();  
  delay(10);
}
