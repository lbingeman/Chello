#include <Bow.h>
#include <Transpose.h>
#include "Adafruit_MPR121.h"

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

  //Adafruit Setup (check if sensor is connected and start)
  Serial.println("Connected!");
  if (!stringSensor.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring.");
    while (1);
  }
  Serial.println("MPR121 found!");
}

/**
  * @desc calculates amount of pitch bend that should be applied on the MIDI note based on the parameters recieved by teensy
  * @param int string - string on Chello that is being played (1 - lowest pitch string, 4 - highest pitch string)
  * @return int - pitch increment for current string (range 0 - 16384)
*/
int getPitchIncrement(int string) {
  int resistanceReading = analogRead(pitchPins[string-1]); //Get Analog resistance reading from sensor
  int pitchBendResistance = resistanceReading;
  
  if (string == 4) {
    //Adjust fourth string due to error in wiring caused flipped readings when sensor is depressed
    if(resistanceReading >= 15) {
      pitchBendResistance = map(pitchBendResistance,0,1023,1023,0);
    }
  }

  //Minimum threshold for going from open string to closed string (pitch range 1). Added to avoid oscillation 
  if (pitchBendResistance < 15) {
    pitchBendResistance = 0;
  }
  
  int pitchBendAmount = 0;
  int singleSemitonPitchBend = (8192*2)/14;//set range of pitch bend for each semitone 


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

/**
  * @desc plays a MIDI note based on the parameters recieved by teensy
  * @param int string - string on Chello that is being played (1 - lowest pitch string, 4 - highest pitch string)
  * @param uint16_t _currStringTouched - integer corresponding to the output from the Adafruit Capacitive touch board
  * @param uint16_t _lastStringTouched - integer corresponding to the output from the Adafruit Capacitive touch board read on the previous loop
  * @param bool _notePlayed - pointer to an bool[4] array which corresponds to whether a note is currently being played on the string
*/
void playString(int string, uint16_t _currStringTouched, uint16_t _lastStringTouched, bool *_notePlayed)
{
  //set note velocity
  int velocity = 100;

  //set the base note (this is the note that will be pitch bent from). This is based on string & instrument
  int baseNote = transpose.noteValue(string);
  //set the pitch bend (based on the position of the finger on the fingerboard)
  int pitchBend = getPitchIncrement(string);

  //get position of capacitive touch sensor corresponding to the current string
  int stringPort = stringMapping[string-1];

  //boolean check if the string is touched
  bool stringTouched = (_currStringTouched & _BV(stringPort));

  //boolean check if string was touched on previous loop
  bool stringPreviouslyTouched = (_lastStringTouched & _BV(stringPort));

  //boolean check if the bow is currently moving and a note should be played
  bool bowMoving = bow.playNote(stringTouched);

  //turn MIDI signal on if a new string is pressed and the bow is moving
  if (((stringTouched && !stringPreviouslyTouched) && bowMoving) || (bowMoving && !_notePlayed[string-1])) {
    usbMIDI.sendPitchBend(pitchBend,string);
    usbMIDI.sendNoteOn(baseNote, velocity, string); // Turn the note ON
    _notePlayed[string-1] = true;
  } else if (stringTouched){ //update pitchbend
    usbMIDI.sendPitchBend(pitchBend,string);
  }
  else if ((!stringTouched && stringPreviouslyTouched) || (stringTouched && !bowMoving)) { //turn note off is string no longer touched or bow stops moving
    usbMIDI.sendNoteOff(baseNote, velocity, string); // Turn the note Off
    _notePlayed[string-1] = false;
  }
}

void loop() {
  bow.update(); //update bow information (bow mode status, bow motion)
  transpose.update(); //update transpotion information (transposition status)

  uint16_t currStringTouched = stringSensor.touched(); //get currently touched strings
  
  for (int i = 1; i <= 4; i++) { //play/update for each string
    playString(i,currStringTouched,lastStringTouched,notePlayed);
  }

  lastStringTouched = currStringTouched; //set staus variable of last touched string
  delay(10); //delay to ensure MIDI Synthesizer is not overloaded by signals
}
