/* Credit:
 * https://arduino.stackexchange.com/questions/36942/managing-2-ultrasonic-sensors-without-blocking-the-other
*/

volatile unsigned long LastPulseTimeA;
volatile unsigned long LastPulseTimeB;
int durationA;
int durationB;
//unsigned long startTime;
#define trigPinA 4
#define echoPinA 2
#define trigPinB 5
#define echoPinB 3




void setup() {
  Serial.begin (115200);
  pinMode(trigPinA, OUTPUT);
  pinMode(echoPinA, INPUT);
  pinMode(trigPinB, OUTPUT);
  pinMode(echoPinB, INPUT);  
	attachInterrupt(digitalPinToInterrupt(echoPinA), EchoPinA_ISR, CHANGE);  // Pin 2 interrupt on any change
	attachInterrupt(digitalPinToInterrupt(echoPinB),EchoPinB_ISR, CHANGE);  // Pin3 interrupt on any change
}



void loop(){
  digitalWrite(trigPinA, LOW);
  digitalWrite(trigPinB, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinA, HIGH);
  digitalWrite(trigPinB, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinA, LOW);
  digitalWrite(trigPinB, LOW);
  Serial.print("SensorA:");
  // Serial.print(LastPulseTimeA);
  // Serial.print('\t');
  float valueA = (LastPulseTimeA/2) / 29.1;
  if (valueA > 200.0) valueA = 200.0;
  Serial.print(valueA,1);
  // Serial.println("cm");
	Serial.println();
  Serial.print("SensorB:");
  // Serial.print(LastPulseTimeB);
  // Serial.print('\t');
  float valueB = (LastPulseTimeB/2) / 29.1;
  if (valueB > 200.0) valueB = 200.0;
  Serial.print(valueB,1);
  // Serial.println("cm");
	Serial.println();

  delay(100);
}



void EchoPinA_ISR() {
    static unsigned long startTimeA;

    if (digitalRead(echoPinA)) // Gone HIGH
        startTimeA = micros();
    else  // Gone LOW
        LastPulseTimeA = micros() - startTimeA;
}



void EchoPinB_ISR() {
    static unsigned long startTimeB;

    if (digitalRead(echoPinB)) // Gone HIGH
        startTimeB = micros();
    else  // Gone LOW
        LastPulseTimeB = micros() - startTimeB;
}




