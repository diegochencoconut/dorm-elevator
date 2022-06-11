int myservopin = 39;
void setup() {
  pinMode(myservopin, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:

  //close
  for (int i = 0; i < 250; i++)
  {
    digitalWrite(myservopin, HIGH);
    delayMicroseconds(1500);
    digitalWrite(myservopin, LOW);
    delayMicroseconds(1500);
  }
  delay(1000);

  //open
  for (int i = 0; i < 250; i++)
  {
    float high = 2150;
    float low = 3000 - high;
    digitalWrite(myservopin, HIGH);
    delayMicroseconds(high);
    digitalWrite(myservopin, LOW);
    delayMicroseconds(low);
  }
  delay(5000);
}
