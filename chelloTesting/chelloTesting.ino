#define pitchPin A9

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //pin mode setup
  pinMode(pitchPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int resistanceReading = analogRead(pitchPin);
  Serial.println(resistanceReading);
  delay(10);
}
