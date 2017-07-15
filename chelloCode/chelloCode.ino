#include <Bow.h>
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

Bow bow = Bow();

#include <AcceleroMMA7361.h>
AcceleroMMA7361 accelero;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  bow.begin(accXPin,accYPin,accZPin,12,13,14,15);
  
  accelero.begin(13, 12, 11, 10, accXPin, accYPin, accZPin);
  accelero.setARefVoltage(3.3);                   //sets the AREF voltage to 3.3V
  accelero.setSensitivity(HIGH);                   //sets the sensitivity to +/-6.0G
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
//    for(int i=0; i<4; i++){
//      pinMode(pitchPins[i],INPUT);
//    }
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

void getAcceleration() {
  int xAl = accelero.getXAccel();
  int yAl = accelero.getYAccel();
  int zAl = accelero.getZAccel();

  int yRead = analogRead(accYPin);
  int xRead = analogRead(accXPin);

  float yVolts =  ((((yRead) * (3.3) / (1023))-yCalibration)/0.206)*9.81;
  
  float Rx = xAl/9.81;//(DeltaVoltsRx / Sensitivity)*9.81;
  float Ry =  yAl/9.81;//(DeltaVoltsRy / Sensitivity)*9.81;
  float Rz =  zAl/9.81;//(DeltaVoltsRz / Sensitivity)*9.81;
  
  int currentTime = millis();
  float deltaT = (currentTime-prevTime)/1000.0;
  
  float vx = prevVelocity[0]+trapozoidRule(deltaT,prevRead[0],Rx);
  float vy = prevVelocity[1]+trapozoidRule(deltaT,prevRead[1],Ry);
  
  prevRead[0] = Rx;
  prevRead[1] = Ry;
  prevRead[2] = Rz;

  prevVelocity[0] = vx;
  prevVelocity[1] = vy;

  prevTime = currentTime;
}

float trapozoidRule(float dt,float f0,float f1) {
  return (dt/2)*(f0+f1);
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
    Serial.print("Turned on note: ");
    Serial.println(string);
  } else if (currStringTouched & _BV(string)){
    usbMIDI.sendPitchBend(pitchBend,string);
  }
  else if (!(currStringTouched & _BV(string)) && (lastStringTouched & _BV(string)) ) {
    usbMIDI.sendNoteOff(baseNote, velocity, string); // Turn the note Off
    Serial.print("Turned off note: ");
    Serial.println(string);
  }

  
}
  
void calibrate(){
    float xSum = 0;
  float ySum = 0;
  
  for (int i=0; i<10000;i++){
    int yRead = analogRead(accYPin);
    int xRead = analogRead(accXPin);

    xSum += xRead;
    ySum += yRead;
    Serial.println(yRead);
    delay(10);
  }

  xCalibration = (xSum/10000);
  yCalibration = (ySum/10000);

  Serial.print("Calibrated accelerator with: ");
  Serial.print(xCalibration);
  Serial.print(" ");
  Serial.println(yCalibration);
}

void loop() {
  bow.update();
 
  digitalWrite(37,HIGH);
  // put your main code here, to run repeatedly:
  currStringTouched = stringSensor.touched(); //get currently touched strings
  
  for (int i = 1; i <= 4; i++) {
    playString(i);
  }

  lastStringTouched = currStringTouched;
  
  delay(10);
}
