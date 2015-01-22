/* 	unit test for the limit switches
	@author Marien Wolthuis
					*/
					
#define fwd 5
#define rev 6
#define up 10
#define down 11
bool state = true;

void setup(){
	pinMode(fwd, OUTPUT); // fwd
	//pinMode(rev, INPUT); // rev
	//pinMode(up, INPUT); // up
	//pinMode(down, INPUT); // down
	
	pinMode(8, INPUT);
	
	Serial.begin(9600);
}

void loop(){
	int _fwd = digitalRead(fwd);
	//int _rev = digitalRead(rev);
	//int _up = digitalRead(up);
	//int _down = digitalRead(down);
	
	digitalWrite(fwd, state);
	state = !state;
	
//	Serial.print("Forward state: "); Serial.println(_fwd);
	Serial.print("Inverter output: "); Serial.println(digitalRead(8));
	//Serial.print("Reverse state: "); Serial.println(_rev);
	//Serial.print("Up state: "); Serial.println(_up);
	//Serial.print("Down state: "); Serial.println(_down);
	Serial.println("\n");
	
	delay(3000);
}