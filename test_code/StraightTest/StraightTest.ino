#include<SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
// 引入 SPI 程式庫 與 MFRC522 程式庫
#define RST_PIN 13
#define SDA_PIN 9
// 設定重設腳位 與 SPI 介面裝置選擇腳位
MFRC522 *mfrc522;
SoftwareSerial BT(10,11);
int Rx=10;
int Tx=11;
int Sensor[5]={21, 20, 19, 18, 17};
bool started = 0;
bool stopped = 0;
bool right_turn = 0;
bool node_hit = 0;
// Motor
int PWMA = 2;
int AIN1 = 4;
int AIN2 = 3;
int STBY = 5;
int BIN1 = 6;
int BIN2 = 7;
int PWMB = 8;
int num_of_step=0;
enum running_mode
{
  STARTED,
  STOPPED,
};
enum turning_mode
{
  LEFT=11,
  RIGHT=12,
  RETURN=13,
  STRAIGHT=14,
  NA=15
};
running_mode cur_mode=STOPPED;
turning_mode* turn_mode_input;
turning_mode turn_mode_cur=NA;
//===============================for debug
bool debug = false;
//===========================================

double last_offset = 0;
double sum_offset = 0;
int cnt_step = 0;
int cnt_step_input=0;
int turning=0;

bool ValidCommand(String ss)
{
  bool x=true;
  for(int i=0;i<ss.length();i++)
  {
    if((ss[i]!='f')&&(ss[i]!='b')&&(ss[i]!='r')&&(ss[i]!='l'))
    {
      x=false;
      break;
    }
  }
  return x;
}
void StringToCommands(String ss)
{
    cnt_step=0;
    num_of_step=ss.length();
    if(turn_mode_input!=NULL)delete [] turn_mode_input;
    turn_mode_input=new turning_mode[num_of_step];
    for(int i=0;i<ss.length();i++)
    {
        switch(ss[i])
        {
            case 'f':
                turn_mode_input[i]=STRAIGHT;
                break;
            case 'b':
                turn_mode_input[i]=RETURN;
                break;
            case 'r':
                turn_mode_input[i]=RIGHT;
                break;
            case 'l':
                turn_mode_input[i]=LEFT;
                break;
            default:
                turn_mode_input[i]=NA;
                break;
        }
    }
}

void adjust(){
  //循跡演算法
  int cnt = 0; 
  
  double total = 0;
  double average = 0;
  double offset = 0;
  if(turn_mode_cur==NA)turn_mode_cur=turn_mode_input[0];
  for(int i = 0; i < 5; i++){
    if(digitalRead(Sensor[i]) == HIGH){
      cnt++;
      total += i;
    }
  }
  if(cnt == 0)
  {
    Serial.println(turn_mode_cur);
    if(turn_mode_cur==LEFT)MotorWrite(70, -60);
    else if(turn_mode_cur==RIGHT)MotorWrite(-60, 70);
    else MotorWrite(-60, 70);
  }
  else{
    average = total / cnt; // calculate the current average
    if (cnt <= 3){
      if(turning==1)
      {
        cnt_step++;//the car has turned -> next step
        turn_mode_cur = turn_mode_input[min(cnt_step,num_of_step-1)];
      }
      turning = 0;
      double V_s = 100; // the velocity of going straight (CHANGE HERE!)
      double K_p = 30; // P control term (CHANGE HERE!)
      double K_i = 0;  // I control term (CHANGE HERE!)
      double K_d = 0;  // D control term (CHANGE HERE!)

      offset = average - 2.0; // the ideal average is 2.0
      double diff = offset - last_offset; // the differential term

      MotorWrite(V_s + K_p * offset - K_d * diff + K_i * sum_offset, V_s - K_p * offset + K_d * diff - K_i * sum_offset);
    }
    else {
      //BT.print("Nd");
      Serial.println("Nd");
      Serial.println(cnt_step);
      delay(600); // keep going until reach the RFID.
      if(cnt_step>=num_of_step)
      {
        turn_mode_cur=NA;
        cur_mode=STOPPED;
        Serial.print("You Should STOP!!!! ");
        Serial.print(cnt_step);
        Serial.print(">=");
        Serial.println(num_of_step);
      }
      if(turn_mode_cur==LEFT)
      {
        MotorWrite(80, -80);
        delay(300);
      }
      else if(turn_mode_cur==RIGHT)
      {
        MotorWrite(-80, 80);
        delay(300);
      }
      else if(turn_mode_cur==RETURN)
      {
        MotorWrite(-80,80);
        delay(750);
      }
      else if(turn_mode_cur==STRAIGHT)
      {
        MotorWrite(90,90);
        delay(350);
      }
      turning=1;
    }
    
    last_offset = offset;
    sum_offset += offset;
  }
}

void SensorCheck(){
  Serial.println("Hi");

  for(int i = 0; i < 5; i++){
    Serial.print(i);
    Serial.print(' ');
    Serial.print(digitalRead(Sensor[i]));
    Serial.print(' ');
  }
  Serial.println(' ');
}
bool Sensor_node_hit()
{
  int cnt=0;
  for(int i=0;i<5;i++)
  {
    if(digitalRead(Sensor[i])==HIGH)
    {
      cnt++;
    }
  }
  if(cnt==5)return true;
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
  // go forward
    MotorWrite(100.0, 100.0);
    delay (2000);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (2000);

  // go backward
    MotorWrite(-100.0, -100.0);
    delay (2000);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (2000);

  // go right
    MotorWrite(-100.0, 0.0);
    delay (2000);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (2000);
    
  // go left
    MotorWrite(100.0, 0.0);
    delay (2000);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay (2000);

    delay (5000);
}
void setup() {
  for(int i = 0; i < 5; i++){
    pinMode(Sensor[i], INPUT); //set sensor -> input
  }
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(Rx,INPUT);
  pinMode(Tx,OUTPUT);
  MotorWrite(100, 100);
  SPI.begin();
  mfrc522 = new MFRC522(SDA_PIN, RST_PIN);
  // 請系統去要一塊記憶體空間，後面呼叫它的建構函式
  // 將(SS, RST) 當成參數傳進去初始化。
  mfrc522->PCD_Init();
  /* 初始化MFRC522讀卡機 PCD_Init 模組。 -> 表示：
  透過記憶體位置，找到 mfrc522 這物件，再翻其內容。*/
  Serial.println(F("Read UID on a MIFARE PICC:"));
}

void loop() { 
  while(BT.available())
  {
    //MotorWrite(0.0,0.0);
    String ss=BT.readStringUntil('\n');
    Serial.println(ss);
    if(ss=="start")
    {
      cur_mode=STARTED;
      cnt_step=0;
    }
    else if(ss=="stop")cur_mode=STOPPED;
    else if(ValidCommand(ss))
    {
        StringToCommands(ss);
    }
  }
  switch(cur_mode)
  {
    case STARTED:
      MotorWrite(110,100);
      delay(5000);
      cur_mode=STOPPED;
      break;
    case STOPPED:
      MotorWrite(0.0,0.0);
      break;
  }
  
  if(!mfrc522->PICC_IsNewCardPresent()) {
    goto FuncEnd;
   } //PICC_IsNewCardPresent()：是否感應到新的卡片?
   if(!mfrc522->PICC_ReadCardSerial()) {
    goto FuncEnd;
   } //PICC_ReadCardSerial()：是否成功讀取資料?
   Serial.println(F("**Card Detected:**"));
   mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid)); //讀出 UID
   for (byte i = 0; i < mfrc522->uid.size; i++) {
    BT.print(mfrc522->uid.uidByte[i] < 0x10 ? "0" : "" );
    BT.print(mfrc522->uid.uidByte[i], HEX);
   }
   BT.print('\n');
   mfrc522->PICC_HaltA(); // 讓同一張卡片進入停止模式 (只顯示一次)
   mfrc522->PCD_StopCrypto1(); // 停止 
   MotorWrite(0,0);
   delay(100);
   MotorWrite(10,100);
   delay(100);
   FuncEnd:; // goto 跳到這.
}
