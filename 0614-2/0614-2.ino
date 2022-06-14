#include<SoftwareSerial.h>
SoftwareSerial BT(10,11);
enum car_status
{
  SETTING,
  READY,
  TARGET_FOUND,
  HEADING,
  NEAR_TARGET,
  TARGET_GET,
  PARKING,
  FAIL
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
enum system_status
{
  FIDING,
  STANDBY,
};
int i;
int Rx=10;
int Tx=11;
int Sensor[5]={21, 20, 19, 18, 17};
bool waiting=false;
bool camera_ready=false;
bool arm_ready=false;
bool on_route=false;
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
int LBD=560,RBD=720;
String tp,cont;
char tmp[100]={0};
car_status a_status=READY;
system_status s_status=STANDBY;
double last_tu = 0;
bool In_Interval()
{
  if(x+w/2>=LBD && x+w/2<=RBD)return true;
  else return false;
}
int Sensor_hit()
{
  int cnt = 0;
  for (int i = 0; i < 5; i++)
  {
    if (digitalRead(Sensor[i]) == HIGH)
    {
      cnt++;
    }
  }
  return cnt;
}
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
  for (int i = 0; i < 250; i++)//back
  {
    digitalWrite(SERVO_CAM, HIGH);
    delayMicroseconds(2480);
    digitalWrite(SERVO_CAM, LOW);
    delayMicroseconds(520);
  }
}
void turn_arm(arm_status s)
{
  if(s==OPEN)
  {
    for (int i = 0; i < 250; i++)
    {
      digitalWrite(SERVO_ARM, HIGH);
      delayMicroseconds(1500);
      digitalWrite(SERVO_ARM, LOW);
      delayMicroseconds(1500);
    }
  }
  else if (s==CLOSE)
  {
    for (int i = 0; i < 250; i++)
    {
      float high = 2150;
      float low = 3000 - high;
      digitalWrite(SERVO_ARM, HIGH);
      delayMicroseconds(high);
      digitalWrite(SERVO_ARM, LOW);
      delayMicroseconds(low);
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
    
    
    //MotorCheck();
    if(In_Interval())
    {
      a_status=HEADING;
    }
    else 
    {
      //MotorCheck()
      //double tu=(((x+w/2)-640)*240)/640.0;
      double tu = (( (x + w / 2) / 640.0 ) - 1) * 240;
      Serial.println(tu);

      if (last_tu != 0) //not the initial status
      {
        if (tu*last_tu < 0 && LBD != 560) //prevent in nearby, go back and forth
        {
          MotorWrite(100, 100);
          delay(100);
          MotorWrite(0, 0);
          delay(500);
          Serial.println("ready");
          waiting=true;
          last_tu = 0;
          return;
        }
      }
      last_tu = tu;
      if(tu>0)tu=max(tu,100);
      else if(tu<0)tu=min(tu,-100);
      Serial.println(tu);
      MotorWrite(-tu,tu);
      delay(100);
      MotorWrite(0,0);
      delay(500);
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
    if(not In_Interval())
    {
      a_status=TARGET_FOUND;
    }
    else if(w>=170)
    {
      a_status=NEAR_TARGET;
    }
    else
    {
      if(w<=100)
      {
        LBD=560;
        RBD=680;
        MotorWrite(-150, -150);
        delay(350);
      }
      if(w>=140)
      {
        turn_arm(OPEN);
        LBD = 600;
        RBD = 680;
        MotorWrite(-100, -100);
        delay(350);
      }
      else
      {
        LBD = 560;
        RBD = 720;
        MotorWrite(-100,-100);
        delay(500);
      }
      MotorWrite(0,0);
      Serial.println("ready");
      waiting=true;
    }
  }
  
}
void trailer()
{
  turn_arm(OPEN);
  MotorWrite(-100,-100);
  delay(1300);
  MotorWrite(0,0);
  turn_arm(CLOSE);
  a_status=TARGET_GET;
  Serial.println("Gotcha");
  turn_camera(FRONT);
  MotorWrite(100,-100);
  delay(500);
}
/*void back_home()
{
  MotorWrite(100,100);
  delay(1000);
  MotorWrite(0,0);
  a_status=READY;
  s_status=STANDBY;
}*/

//=======================================BACK HOME CODE==========================================//
void adjust() {
  //循跡演算法
  int cnt = 0;

  double total = 0;
  double average = 0;
  double offset = 0;

  for (int i = 0; i < 5; i++) {
    if (digitalRead(Sensor[i]) == HIGH) {
      cnt++;
      total += i;
    }
  }
  if (cnt == 0)
  {
    MotorWrite(-120, 140);
  }
  else {
    average = total / cnt; // calculate the current average
    if (cnt <= 4) {
      double V_sR = 200; // the velocity of going straight (CHANGE HERE!)
      double V_sL = 200;
      double K_p = 30; // P control term (CHANGE HERE!)

      offset = average - 2.0; // the ideal average is 2.0

      MotorWrite(min(V_sR + K_p * offset , 255), min(V_sL - K_p * offset, 255));
    }
    else {
      MotorWrite(0,0);
      if(a_status == TARGET_GET)
      {
        delay(100);
        if(Sensor_hit()==5)
        {
          turn_arm(OPEN);
          while(Sensor_hit()==5)
          {
            MotorWrite(150,150);
          }
          MotorWrite(0,0);
          delay(5000);
          a_status=PARKING;
        }
      }
      else
      {
        turn_arm(CLOSE);
        a_status=READY;
        s_status=STANDBY;
        Serial.println("home");
      }
    }
  }
}

void back_home()
{
  if(s_status != STANDBY)
  {
    if(on_route)
    {
      adjust();
    }
    else
    {
      MotorWrite(150,-150);
      delay(200);
      MotorWrite(150,150);
      while(Sensor_hit()==0){}
      on_route=true;
    }
  }
}
 
//======================================================BACK HOME CODE========================================================================//

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
  
  turn_camera(FRONT);
  delay(2000);
  turn_camera(BACK);
  
  turn_arm(OPEN);
  delay(1000);
  turn_arm(CLOSE);
  Serial.println("Setup finish");
  
  
}
void loop() {
  while(s_status == STANDBY)
  {
    MotorWrite(0.0,0.0);
    if (BT.available() == 0) continue;
    String ss=BT.readString();
    s_status = FIDING;
    if(ss=="black_tea")
    {
      Serial.println("black_tea");
    }
    else if(ss=="green_tea")
    {
      Serial.println("green_tea");
    }
    else if(ss=="soy") 
    {
      Serial.println("soy");
    }
    else if(ss=="chocolate") 
    {
      Serial.println("chocolate");
    }
    else  s_status = STANDBY;
  }
  
  if (Serial.available() > 0) {
    
    MotorWrite(0,0);
    String data = Serial.readStringUntil('\n');
    Serial.print("Input:");
    Serial.println(data);
    Serial.println(waiting);
    if (data == "fail")
    {
      MotorWrite(-150,-150);
      delay(3000);
      a_status = TARGET_GET;
    }
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
          x=10*x-(data[i]-'0');
          i++;
        }
      }
      else
      {
        i=2;
        x=0;
         while(data[i]!=0)
        {
          x=10*x+(data[i]-'0');
          i++;
        }
      }
      Serial.print("x=");
      Serial.println(x);
    }
    else if(data[0]=='h')
    {
      if(data[2]=='-')
      {
        i=3;
        h=0;
         while(data[i]!=0)
        {
          h=10*h-(data[i]-'0');
          i++;
        }
      }
      else
      {
        i=2;
        h=0;
         while(data[i]!=0)
        {
          h=10*h+(data[i]-'0');
          i++;
        }
      }
      if(a_status==READY)
      {
        if(not camera_ready)
        {
          turn_camera(FRONT);
          turn_camera(BACK);
          camera_ready=true;
        }
        if(not arm_ready)
        {
           turn_arm(CLOSE);
           arm_ready=true;
        }
        a_status=TARGET_FOUND;
      }
      Serial.print("h=");
      Serial.println(h);
      Serial.println("received");
      waiting=false;// input complete
    }
    else if(data[0]=='w')
    {
      if(data[2]=='-')
      {
        i=3;
        w=0;
         while(data[i]!=0)
        {
          w=10*w-(data[i]-'0');
          i++;
        }
      }
      else
      {
        i=2;
        w=0;
         while(data[i]!=0)
        {
          w=10*w+(data[i]-'0');
          i++;
        }
      }
      Serial.print("w=");
      Serial.println(w);
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
      camera_ready=false;
      arm_ready=false;
      on_route=false;
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
    case PARKING:
    case FAIL:
      back_home();
      waiting=false;
      break;
  }
}
