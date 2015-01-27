void setup(){
 pinMode(5, OUTPUT); // PWM disable
 pinMode(6, OUTPUT); // reverse
 pinMode(7, OUTPUT); // forward
 digitalWrite(5, LOW); 
  digitalWrite(6, LOW); 
   digitalWrite(7, LOW); 
 Serial.begin(9600);
}

void loop(){
 digitalWrite(6, HIGH);
Serial.println("6 HIGH (reverse)");

for(int i=100;i<180;i+=1){
 analogWrite(5, i);
 Serial.println(i);
 delay(100);
}
analogWrite(5, 0);

digitalWrite(6,LOW);
Serial.println("6 LOW");
delay(5000);

 digitalWrite(7, HIGH);
Serial.println("7 HIGH (forward)");
analogWrite(5, 160);
delay(300);
for(int i=100;i<180;i+=2){
 analogWrite(5, i);
  Serial.println(i);
 delay(100);
}
analogWrite(5, 0);

digitalWrite(7,LOW);
Serial.println("7 LOW");
delay(5000);
}
