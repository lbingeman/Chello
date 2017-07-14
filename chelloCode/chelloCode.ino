#include "Adafruit_MPR121.h"

#define pitchPinS4 A9
#define pitchPinS3 A17
#define pitchPinS2 A13
#define pitchPinS1 A12
#define accXPin A14
#define accYPin A15
#define accZPin A16
#define gyro A17



Adafruit_MPR121 stringSensor = Adafruit_MPR121();

uint16_t lastStringTouched = 0;
uint16_t currStringTouched = 0;

int pitchPins[4] = {pitchPinS1,pitchPinS2,pitchPinS3,pitchPinS4};

float prevRead[4][3] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};//x,y,z
float prevVelocity[3] = {0,0,0};
int currIndex = 0;

int measureAcceleration = 0;
int accCycles = 1;

int delayTime = 10;

#include <AcceleroMMA7361.h>
AcceleroMMA7361 accelero;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  accelero.begin(13, 12, 11, 10, accXPin, accYPin, accZPin);
  accelero.setARefVoltage(3.3);                   //sets the AREF voltage to 3.3V
  accelero.setSensitivity(LOW);                   //sets the sensitivity to +/-1.5G
  accelero.calibrate();
  pinMode(37, OUTPUT);
  
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

    //pin mode setup
    for(int i=0; i<4; i++){
      pinMode(pitchPins[i],INPUT);
    }
}

int getPitchIncrement(int string) {
  int resistanceReading = analogRead(pitchPins[string-1]);
  int pitchBendResistance = resistanceReading;
  
  if (pitchBendResistance < 15) {
    pitchBendResistance = 0;
  }

  int pitchBendAmount = map(pitchBendResistance,0,1023,0,8192*2);
//
//  if (string == 3) {
//    Serial.print(pitchBendAmount);
//    Serial.print('\t');
//    Serial.print(currStringTouched);
//    Serial.print('\t');
//    Serial.println(resistanceReading);
//  }
  
  return pitchBendAmount;
}

void getAcceleration() {
//  int xAl = analogRead(accXPin);
//  int yAl = analogRead(accYPin);
//  int zAl = analogRead(accZPin);

  
  int xAl = accelero.getXAccel();
  int yAl = accelero.getYAccel();
  int zAl = accelero.getZAccel();

  float Rx = xAl/9.81;//(DeltaVoltsRx / Sensitivity)*9.81;
  float Ry =  yAl/9.81;//(DeltaVoltsRy / Sensitivity)*9.81;
  float Rz =  zAl/9.81;//(DeltaVoltsRz / Sensitivity)*9.81;
  
  if (abs(Rx)<0.5) Rx = 0;
  if (abs(Ry)<0.5) Ry = 0;
  if (abs(Rz)<0.5) Rz = 0;
  
  
//  float VoltsRx = xAl * 3.3 / 1023; //=~ 1.89V  (we round all results to 2 decimal points)
//  float VoltsRy = yAl * 3.3 / 1023; //=~ 2.03V
//  float VoltsRz = zAl * 3.3 / 1023;
//
//  float DeltaVoltsRx = VoltsRx - 1.65;
//  float DeltaVoltsRy = VoltsRy - 1.65;
//  float DeltaVoltsRz = VoltsRz - 1.65;
//
//  float Sensitivity = 0.8;
  


  int index0 = currIndex;
  int index1 = (currIndex+1)%4;
  int index2 = (currIndex+2)%4;
  int index3 = (currIndex+3)%4;
  
  float vx = getVelocity((delayTime*accCycles)/1000.0,prevRead[index0][0],prevRead[index1][0],prevRead[index2][0],prevRead[index3][0],Rx);
  float vy = getVelocity((delayTime*accCycles)/1000.0,prevRead[index0][1],prevRead[index1][1],prevRead[index2][1],prevRead[index3][1],Ry);
  float vz = getVelocity((delayTime*accCycles)/1000.0,prevRead[index0][2],prevRead[index1][2],prevRead[index2][2],prevRead[index3][2],Rz);
  
  prevRead[index0][0] = Rx;
  prevRead[index0][1] = Ry;
  prevRead[index0][2] = Rz;

  prevVelocity[0] = vx;
  prevVelocity[1] = vy;
  prevVelocity[2] = vz;
  
  currIndex = (currIndex+1)%4;
  
  Serial.print(accelero.getOrientation());
  Serial.print('\t');
  Serial.print("A Total: ");
  Serial.print(pow(Rx*Rx+Ry*Ry+Rz*Rz,0.5));
  Serial.print('\t');
  Serial.print("AX: ");
  Serial.print(Rx);
  Serial.print('\t');
  Serial.print("RY: ");
  Serial.print(Ry);
  Serial.print('\t');
  Serial.print("AZ: ");
  Serial.println(Rz);
  
  
}

float getVelocity(float h,float f0,float f1,float f2,float f3,float f4) {
  return (h/90)*(7*f0+32*f1+12*f2+32*f3+7*f4);
}

float getSlope(float h,float f0,float f4) {
  return (f4-f0)/(h*4);
}

int getNote(int string) {
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

void playString(int string)
{
  int velocity = 100;
  int baseNote = getNote(string); 
  int pitchBend = getPitchIncrement(string);
   
  if ((currStringTouched & _BV(string)) && !(lastStringTouched & _BV(string)) ) {
    usbMIDI.sendPitchBend(8192*2,string);
    usbMIDI.sendNoteOn(baseNote, velocity, string); // Turn the note ON
  } else if (currStringTouched & _BV(string)){
    usbMIDI.sendPitchBend(pitchBend,string);
  }
  else if (!(currStringTouched & _BV(string)) && (lastStringTouched & _BV(string)) ) {
    usbMIDI.sendNoteOff(baseNote, velocity, string); // Turn the note Off
  }

  
}
  

void loop() {
  digitalWrite(37,HIGH);
  // put your main code here, to run repeatedly:
  currStringTouched = stringSensor.touched(); //get currently touched strings
  
  for (int i = 1; i <= 4; i++) {
    playString(i);
  }

  lastStringTouched = currStringTouched;

  if (measureAcceleration == 0) {
    getAcceleration();
    measureAcceleration = measureAcceleration++%accCycles;
  }
  
  delay(10);
}
