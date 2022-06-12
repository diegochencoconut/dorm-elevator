#include<SoftwareSerial.h>
SoftwareSerial BT(10,11);
enum car_status
{
  READY,
  TARGET_FOUND,
  HEADING,
  NEAR_TARGET,
  TARGET_GET
};
enum camera_status
{
  FRONT=0,
  RIGHT=90,
  BACK=180
};
enum arm_status
{
  OPEN=0,
  CLOSE=90
};
int i;
int Rx=10;
int Tx=11;
int Sensor[5]={21, 20, 19, 18, 17};
bool waiting=false;
bool camera_ready=false;
// Motor
int PWMA = 2;
int AIN1 = 4;
int AIN2 = 3;
int STBY = 5;
int BIN1 = 6;
int BIN2 = 7;
int PWMB = 8;
int SERVO_CAM=38;
int SERVO_ARM=39;
int x=0,w=0,h=0,dist;
char label[15]={0};
const int LBD=600,RBD=680;
String tp,cont;
char tmp[100]={0};
car_status a_status=READY;
void MotorWrite(double vR, double vL){
    
    int Left_forward = (vR >= 0)? HIGH : LOW;
    int Right_forward = (vL >= 0)? HIGH : LOW;
    
    digitalWrite(STBY, HIGH);
    analogWrite(PWMA, abs(vR));
    analogWrite(PWMB, abs(vL));
    
    digitalWrite(AIN1, Left_forward);
    digitalWrite(AIN2, (1 - Left_forward));
    digitalWrite(BIN1, (1 - Right_forward));
    digitalWrite(BIN2, Right_forward);
}
void MotorCheck(){
  
  // go right
    MotorWrite(-100.0, 100.0);
    delay (100);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (500);
    
  // go left
    MotorWrite(100.0, -100.0);
    delay (100);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (500);
}
void turn_camera(camera_status s)
{
  if(s==FRONT)
  {
     for (int i = 0; i < 250; i++)// front
    {
      digitalWrite(SERVO_CAM, HIGH);
      delayMicroseconds(520);
      digitalWrite(SERVO_CAM, LOW);
      delayMicroseconds(2480);
    }
  }
  else if(s==BACK)
  {
    for (int i = 0; i < 250; i++)//back
    {
      digitalWrite(SERVO_CAM, HIGH);
      delayMicroseconds(2480);
      digitalWrite(SERVO_CAM, LOW);
      delayMicroseconds(520);
    }
  }
}
void looking()
{
  
  if(waiting)
  {
    return;
  }
  else 
  {
    if(not camera_ready)
    {
      turn_camera(FRONT);
      turn_camera(BACK);
      camera_ready=true;
    }
    if(x+w/2>=LBD && x+w/2<=RBD)
    {
      a_status=HEADING;
      return;
    }
    else 
    {
      MotorWrite(((x+w/2)-640)/640*120,-((x+w/2)-640)/640*120);
      delay(100);
      MotorWrite(0,0);
      Serial.println("ready");
      waiting=true;
    }
    
    
  }
  
}
void heading()
{
  if(waiting)
  {
    return;
  }
  else
  {
    if(h>=300)
    {
      a_status=NEAR_TARGET;
    }
    else
    {
      MotorWrite(-100,-100);
      delay(500);
      MotorWrite(0,0);
      Serial.println("ready");
      waiting=true;
    }
  }
  
}
void trailer()
{
  a_status=TARGET_GET;
}
void back_home()
{
  turn_camera(FRONT);
  MotorWrite(0,0);
  a_status=READY;
}
void setup() {
  for(int i = 0; i < 5; i++){
    pinMode(Sensor[i], INPUT); //set sensor -> input
  }
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(Rx,INPUT);
  pinMode(Tx,OUTPUT);
  pinMode(SERVO_CAM, OUTPUT);
  pinMode(SERVO_ARM, OUTPUT);
  MotorCheck();
  for (int i = 0; i < 250; i++)// front
  {
    digitalWrite(SERVO_CAM, HIGH);
    delayMicroseconds(520);
    digitalWrite(SERVO_CAM, LOW);
    delayMicroseconds(2480);
  }
  delay(2000);
  for (int i = 0; i < 250; i++)//back
  {
    digitalWrite(SERVO_CAM, HIGH);
    delayMicroseconds(2480);
    digitalWrite(SERVO_CAM, LOW);
    delayMicroseconds(520);
  }
  for (int i = 0; i < 250; i++)
  {
    digitalWrite(SERVO_ARM, HIGH);
    delayMicroseconds(1500);
    digitalWrite(SERVO_ARM, LOW);
    delayMicroseconds(1500);
  }
  delay(1000);

  //open
  for (int i = 0; i < 250; i++)
  {
    float high = 2150;
    float low = 3000 - high;
    digitalWrite(SERVO_ARM, HIGH);
    delayMicroseconds(high);
    digitalWrite(SERVO_ARM, LOW);
    delayMicroseconds(low);
  }
  Serial.println("Setup finish");
  
  
}
void loop() {
  if (Serial.available() > 0) {
    
    MotorWrite(0,0);
    String data = Serial.readStringUntil('\n');
    Serial.print("Input:");
    Serial.println(data);
    Serial.println(waiting);
    /*
    Serial.print("You sent me: ");
    Serial.println(data);
    //*/
    Serial.println(data[0]);
    Serial.println(data.length());
    if(data[0]=='l')
    {
      i=6;
      while(data[i]!=0)
      {
        label[i-6]=data[i];
        i++;
      }
      
      Serial.println(label);
    }
    else if(data[0]=='x')
    {
      if(data[2]=='-')
      {
        i=3;
        x=0;
         while(data[i]!=0)
        {
          x=10*x-data[i];
          i++;
        }
      }
      else
      {
        i=2;
        x=0;
         while(data[i]!=0)
        {
          x=10*x+data[i];
          i++;
        }
      }
    }
    else if(data[0]=='h')
    {
      if(data[2]=='-')
      {
        i=3;
        h=0;
         while(data[i]!=0)
        {
          h=10*h-data[i];
          i++;
        }
      }
      else
      {
        i=2;
        h=0;
         while(data[i]!=0)
        {
          h=10*h+data[i];
          i++;
        }
      }
      if(a_status==READY)
      {
        a_status=TARGET_FOUND;
      }
      Serial.println("INPUT FINISHED.");
      waiting=false;// input complete
      Serial.println(waiting);
    }
    else if(data[0]=='w')
    {
      if(data[2]=='-')
      {
        i=3;
        w=0;
         while(data[i]!=0)
        {
          w=10*w-data[i];
          i++;
        }
      }
      else
      {
        i=2;
        w=0;
         while(data[i]!=0)
        {
          w=10*w+data[i];
          i++;
        }
      }
    }
  }
  switch(a_status)
  {
    case READY:
      MotorWrite(0,0);
      if(not waiting)
      {
        Serial.println("ready");
        waiting=true;
      }
      break;
    case TARGET_FOUND:
      looking();
      break;
    case HEADING:
      heading();
      break;
    case NEAR_TARGET:
      trailer();
      break;
    case TARGET_GET:
      back_home();
      break;
  }
}
