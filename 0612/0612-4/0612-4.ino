#include<SoftwareSerial.h>
SoftwareSerial BT(10,11);
enum car_status
{
  READY,
  TARGET_FOUND,
  HEADING,
  NEAR_TARGET,
  TARGET_GET,
  FAIL,
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
  FINDING,
  STANDBY,
};
int i;
int Rx=10;
int Tx=11;
int Sensor[5]={21, 20, 19, 18, 17};
bool waiting=false;
bool camera_ready=false;
bool arm_ready=false;
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
    if(not camera_ready)
    {
      turn_camera(FRONT);
      turn_camera(BACK);
      camera_ready=true;
    }
    if(not arm_ready)
    {
       turn_camera(BACK);
       arm_ready=true;
    }
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
    else if(h>=240)
    {
      a_status=NEAR_TARGET;
    }
    else
    {
      if(h<=150)
      {
        LBD=560;
        RBD=680;
        MotorWrite(-150, -150);
        delay(350);
      }
      if(h>=200)
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
  turn_arm(CLOSE);
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

void looking_home()
{
  
  if(waiting)
  {
    return;
  }
  else 
  {
    if(not camera_ready)
    {
      turn_camera(BACK);
      turn_camera(FRONT);
      camera_ready=true;
    }
    if(not arm_ready)
    {
       turn_camera(FRONT);
       arm_ready=true;
    }
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
          MotorWrite(-100, -100);
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
      if(tu>0)tu=min(-tu,-100);
      else if(tu<0)tu=max(-tu,100);
      Serial.println(tu);
      MotorWrite(tu,-tu);
      delay(100);
      MotorWrite(0,0);
      delay(500);
      Serial.println("ready");
      waiting=true;
    }  
    
  }
  
}
void heading_home()
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
    else if(h>=190)
    {
      a_status=NEAR_TARGET;
      turn_arm(OPEN);
    }
    else
    {
      if(h<=150)
      {
        LBD=560;
        RBD=680;
        MotorWrite(150, 150);
        delay(350);
      }
      if(h>=170)
      {
        LBD = 600;
        RBD = 680;
        MotorWrite(100, 100);
        delay(350);
      }
      else
      {
        LBD = 560;
        RBD = 720;
        MotorWrite(100, 100);
        delay(500);
      }
      MotorWrite(0,0);
      Serial.println("ready");
      waiting=true;
    }
  }
  
}
void trailer_home()
{
  MotorWrite(100,100);
  delay(1300);
  MotorWrite(0,0);
  turn_arm(OPEN);
  a_status=TARGET_GET;
  Serial.println("Gotcha");
  turn_camera(BACK);
}

void back_home()
{
  turn_camera(FRONT);
  while(s_status != STANDBY)
  {
    if (Serial.available() > 0) {
      
      MotorWrite(0,0);
      String data = Serial.readStringUntil('\n');
      Serial.print("Input:");
      Serial.println(data);
      Serial.println(waiting);

      if (data == "fail")
        a_status = TARGET_GET;
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
        break;
      case TARGET_FOUND:
        looking_home();
        break;
      case HEADING:
        heading_home();
        break;
      case NEAR_TARGET:
        trailer_home();
        break;
      case TARGET_GET:
        s_status = STANDBY;
        turn_camera(BACK);
        turn_arm(CLOSE);
        return;
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
    s_status = FINDING;
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
      a_status = TARGET_GET;
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
      a_status = READY;
      Serial.println("ready");
      back_home();
      a_status = READY;
      waiting=false;
      camera_ready=false;
      arm_ready=false;
      break;
  }
}
