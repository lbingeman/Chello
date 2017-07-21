#include <Bow.h>
#include "Adafruit_MPR121.h"
#include <Transpose.h>

//Port definitions
#define pitchPinS4 A8
#define pitchPinS3 A9
#define pitchPinS2 A7
#define pitchPinS1 A6
#define accXPin A17
#define accYPin A16
#define accZPin A18
#define transposeStatusRPin 5
#define transposeStatusGPin 6
#define transposeStatusBPin 7
#define transpositionPin 24
#define bowModePin 25
#define bowStatusRPin 8
#define bowStatusGPin 9
#define bowStatusBPin 10

// Capacitive Touch Sensor
Adafruit_MPR121 stringSensor = Adafruit_MPR121();
int stringMapping[4] = {2,3,5,4}; 

// Capacitive Touch Status Variables
uint16_t lastStringTouched = 0;

// Resistive Sensor Pin Mapping
int pitchPins[4] = {pitchPinS1,pitchPinS2,pitchPinS3,pitchPinS4};

bool notePlayed[4] = {0,0,0,0};

Bow bow = Bow(); //Controls bow effects 
Transpose transpose = Transpose(); //Controls base note mapped to each string

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //Initialize Bow (controlling effect of bow) and Transpose objects (base note for each string)
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
  int resistanceReading = analogRead(pitchPins[string-1]); //Get Analog resistance reading from sensor
  int pitchBendResistance = resistanceReading;
  
  if (string == 4) {
    //Adjust fourth string due to error in wiring caused flipped readings when sensor is depressed
    if(resistanceReading >= 15) {
      pitchBendResistance = map(pitchBendResistance,0,1023,1023,0);
    }
  }
  
  if (pitchBendResistance < 15) {
    pitchBendResistance = 0;
  }
  
  int pitchBendAmount = 0;
  int singleSemitonPitchBend = (8192*2)/14;


  //Pitchbend mapping for string 4 (HotPot Sensor) 
  if (string == 4) {
    if (pitchBendResistance <= 145){ //Pitch Range 1 (Pitchbend -7 semitone to Pitchbend -6 semitone)
      pitchBendAmount = map(pitchBendResistance,0,145,0,singleSemitonPitchBend);
   } else if  (pitchBendResistance <= 280) { //Pitch Range 2 (Pitchbend -6 semitone to Pitchbend -5 semitone)
      pitchBendAmount = map(pitchBendResistance,145,280,singleSemitonPitchBend,singleSemitonPitchBend*2);
    } else if (pitchBendResistance <= 350) { //Pitch Range 3 (Pitchbend -5 semitone to Pitchbend -4 semitone)
      pitchBendAmount = map(pitchBendResistance,280,350,singleSemitonPitchBend*2,singleSemitonPitchBend*3);
    } else if (pitchBendResistance <= 410) { //Pitch Range 4 (Pitchbend -4 semitone to Pitchbend -3 semitone)
      pitchBendAmount = map(pitchBendResistance,350,410,singleSemitonPitchBend*3,singleSemitonPitchBend*4);
    } else if (pitchBendResistance <= 470) { //Pitch Range 5 (Pitchbend -3 semitone to Pitchbend -2 semitone)
      pitchBendAmount = map(pitchBendResistance,410,470,singleSemitonPitchBend*4,singleSemitonPitchBend*5);
    } else if (pitchBendResistance <= 540) { //Pitch Range 6 (Pitchbend -2 semitone to Pitchbend -1 semitone)
      pitchBendAmount = map(pitchBendResistance,470,540,singleSemitonPitchBend*5,singleSemitonPitchBend*6);
    } else if (pitchBendResistance <= 580) { //Pitch Range 7 (Pitchbend -1 semitone to Pitchbend 0 semitone)
      pitchBendAmount = map(pitchBendResistance,540,580,singleSemitonPitchBend*6,singleSemitonPitchBend*7);
    } else if (pitchBendResistance <= 620) { //Pitch Range 8 (Pitchbend 0 semitone to Pitchbend +1 semitone)
      pitchBendAmount = map(pitchBendResistance,580,620,singleSemitonPitchBend*7,singleSemitonPitchBend*8);
    } else if (pitchBendResistance <= 655) { //Pitch Range 9 (Pitchbend +1 semitone to Pitchbend +2 semitone)
      pitchBendAmount = map(pitchBendResistance,620,655,singleSemitonPitchBend*8,singleSemitonPitchBend*9);
    } else if (pitchBendResistance <= 680) { //Pitch Range 10 (Pitchbend +2 semitone to Pitchbend +3 semitone)
      pitchBendAmount = map(pitchBendResistance,655,680,singleSemitonPitchBend*9,singleSemitonPitchBend*10);
    } else if (pitchBendResistance <= 710) { //Pitch Range 11 (Pitchbend +3 semitone to Pitchbend +4 semitone)
      pitchBendAmount = map(pitchBendResistance,680,710,singleSemitonPitchBend*10,singleSemitonPitchBend*11);
    } else if (pitchBendResistance <= 740) { //Pitch Range 12 (Pitchbend +4 semitone to Pitchbend +5 semitone)
      pitchBendAmount = map(pitchBendResistance,710,740,singleSemitonPitchBend*11,singleSemitonPitchBend*12);
    } else if (pitchBendResistance <= 770) { //Pitch Range 13 (Pitchbend +5 semitone to Pitchbend +6 semitone)
      pitchBendAmount = map(pitchBendResistance,740,770,singleSemitonPitchBend*12,singleSemitonPitchBend*13);
    } else { //Pitch Range 14 (Pitchbend +6 semitone to Pitchbend +7 semitone)
      pitchBendAmount = map(pitchBendResistance,770,1023,singleSemitonPitchBend*13,singleSemitonPitchBend*14);
    } 
  } else { //SoftPot Sensor
   if (pitchBendResistance <= 48){ //Pitch Range 1 (Pitchbend -7 semitone to Pitchbend -6 semitone)
      pitchBendAmount = map(pitchBendResistance,0,48,0,singleSemitonPitchBend);
   } else if  (pitchBendResistance <= 95) { //Pitch Range 2 (Pitchbend -6 semitone to Pitchbend -5 semitone)
      pitchBendAmount = map(pitchBendResistance,48,95,singleSemitonPitchBend,singleSemitonPitchBend*2);
    } else if (pitchBendResistance <= 125) { //Pitch Range 3 (Pitchbend -5 semitone to Pitchbend -4 semitone)
      pitchBendAmount = map(pitchBendResistance,95,125,singleSemitonPitchBend*2,singleSemitonPitchBend*3);
    } else if (pitchBendResistance <= 158) { //Pitch Range 4 (Pitchbend -4 semitone to Pitchbend -3 semitone)
      pitchBendAmount = map(pitchBendResistance,125,158,singleSemitonPitchBend*3,singleSemitonPitchBend*4);
    } else if (pitchBendResistance <= 190) { //Pitch Range 5 (Pitchbend -3 semitone to Pitchbend -2 semitone)
      pitchBendAmount = map(pitchBendResistance,158,190,singleSemitonPitchBend*4,singleSemitonPitchBend*5);
    } else if (pitchBendResistance <= 230) { //Pitch Range 6 (Pitchbend -2 semitone to Pitchbend -1 semitone)
      pitchBendAmount = map(pitchBendResistance,190,230,singleSemitonPitchBend*5,singleSemitonPitchBend*6);
    } else if (pitchBendResistance <= 265) { //Pitch Range 7 (Pitchbend -1 semitone to Pitchbend 0 semitone)
      pitchBendAmount = map(pitchBendResistance,230,265,singleSemitonPitchBend*6,singleSemitonPitchBend*7);
    } else if (pitchBendResistance <= 290) { //Pitch Range 8 (Pitchbend 0 semitone to Pitchbend +1 semitone)
      pitchBendAmount = map(pitchBendResistance,265,290,singleSemitonPitchBend*7,singleSemitonPitchBend*8);
    } else if (pitchBendResistance <= 320) { //Pitch Range 9 (Pitchbend +1 semitone to Pitchbend +2 semitone)
      pitchBendAmount = map(pitchBendResistance,290,320,singleSemitonPitchBend*8,singleSemitonPitchBend*9);
    } else if (pitchBendResistance <= 345) { //Pitch Range 10 (Pitchbend +2 semitone to Pitchbend +3 semitone)
      pitchBendAmount = map(pitchBendResistance,320,345,singleSemitonPitchBend*9,singleSemitonPitchBend*10);
    } else if (pitchBendResistance <= 380) { //Pitch Range 11 (Pitchbend +3 semitone to Pitchbend +4 semitone)
      pitchBendAmount = map(pitchBendResistance,345,380,singleSemitonPitchBend*10,singleSemitonPitchBend*11);
    } else if (pitchBendResistance <= 415) { //Pitch Range 12 (Pitchbend +4 semitone to Pitchbend +5 semitone)
      pitchBendAmount = map(pitchBendResistance,380,415,singleSemitonPitchBend*11,singleSemitonPitchBend*12);
    } else if (pitchBendResistance <= 460) { //Pitch Range 13 (Pitchbend +5 semitone to Pitchbend +6 semitone)
      pitchBendAmount = map(pitchBendResistance,415,460,singleSemitonPitchBend*12,singleSemitonPitchBend*13);
    } else { //Pitch Range 14 (Pitchbend +6 semitone to Pitchbend +7 semitone)
      pitchBendAmount = map(pitchBendResistance,460,1023,singleSemitonPitchBend*13,singleSemitonPitchBend*14);
    }
  }
  
  return pitchBendAmount;
}

void playString(int string, uint16_t _currStringTouched, uint16_t _lastStringTouched, bool *_notePlayed)
{
  int velocity = 100;
  int baseNote = transpose.noteValue(string);
  int pitchBend = getPitchIncrement(string);

  int stringPort = stringMapping[string-1];
  bool stringTouched = (_currStringTouched & _BV(stringPort));
  bool stringPreviouslyTouched = (_lastStringTouched & _BV(stringPort));
  
  bool bowMoving = bow.playNote(stringTouched);
  
  if (((stringTouched && !stringPreviouslyTouched) && bowMoving) || (bowMoving && !_notePlayed[string-1])) {
    usbMIDI.sendPitchBend(pitchBend,string);
    usbMIDI.sendNoteOn(baseNote, velocity, string); // Turn the note ON
    _notePlayed[string-1] = true;
  } else if (stringTouched){
    usbMIDI.sendPitchBend(pitchBend,string);
  }
  else if ((!stringTouched && stringPreviouslyTouched) || (stringTouched && !bowMoving)) {
    usbMIDI.sendNoteOff(baseNote, velocity, string); // Turn the note Off
    _notePlayed[string-1] = false;
  }
}

void loop() {
  bow.update();
  transpose.update();

  uint16_t currStringTouched = stringSensor.touched(); //get currently touched strings
  
  for (int i = 1; i <= 4; i++) {
    playString(i,currStringTouched,lastStringTouched,notePlayed);
  }

  lastStringTouched = currStringTouched;
  delay(10);
}
