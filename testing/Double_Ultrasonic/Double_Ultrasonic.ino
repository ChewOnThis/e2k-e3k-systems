/* Credit:
 * https://projecthub.arduino.cc/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-7cabe1
*/

class Ultrasonic
{
	public:
		Ultrasonic(int Trigger, int Echo)
		{
			_trigger = Trigger;
			_echo = Echo;
			pinMode(_trigger, OUTPUT);  
			pinMode(_echo, INPUT);  
		}

		float distanceCm()
		{
			digitalWrite(_trigger, LOW);  
			delayMicroseconds(2);  
			digitalWrite(_trigger, HIGH);  
			delayMicroseconds(10);  
			digitalWrite(_trigger, LOW);
			return (pulseIn(_echo, HIGH) * .0343)/2.0;
		}

	private:
		int _trigger;
		int _echo;
};

#define TRIGGER_PIN_1 33
#define ECHO_PIN_1 		39
#define TRIGGER_PIN_2 32
#define ECHO_PIN_2 		36

Ultrasonic sensor1(TRIGGER_PIN_1, ECHO_PIN_1);
Ultrasonic sensor2(TRIGGER_PIN_2, ECHO_PIN_2);



void setup() 
{  
	Serial.begin(115200);
}



void loop() 
{  
	Serial.print("Sensor1:");
	Serial.println(sensor1.distanceCm());
	Serial.print("Sensor2:");
	Serial.println(sensor2.distanceCm());
	delay(10);
}




