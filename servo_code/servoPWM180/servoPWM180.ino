int myservopin = 39;
void setup() {
  pinMode(myservopin, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 250; i++)// front
  {
    digitalWrite(myservopin, HIGH);
    delayMicroseconds(520);
    digitalWrite(myservopin, LOW);
    delayMicroseconds(2480);
  }
  delay(2000);
  for (int i = 0; i < 250; i++)//back
  {
    digitalWrite(myservopin, HIGH);
    delayMicroseconds(2480);
    digitalWrite(myservopin, LOW);
    delayMicroseconds(520);
  }
  delay(5000);
}
