#include<SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
// 引入 SPI 程式庫 與 MFRC522 程式庫
#define RST_PIN 13
#define SDA_PIN 9
// 設定重設腳位 與 SPI 介面裝置選擇腳位
MFRC522 *mfrc522;
SoftwareSerial BT(10, 11);
int Rx = 10;
int Tx = 11;
int Sensor[5] = {21, 20, 19, 18, 17}; //right----->left
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
int num_of_step = 0;
enum running_mode
{
  STARTED,
  ROUTE_LEFT,
  ROUTE_RIGHT,
  STOPPED
};
enum turning_mode
{
  LEFT = 11,
  RIGHT = 12,
  RETURN = 13,
  STRAIGHT = 14,
  NA = 15
};
running_mode cur_mode = STOPPED;
turning_mode* turn_mode_input;
turning_mode turn_mode_cur = NA;
//===============================for debug
bool debug = false;
//===========================================

double last_offset = 0;
double sum_offset = 0;
int cnt_step = 0;
int cnt_step_input = 0;
int turning = 0;
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
void MotorWrite(double vR, double vL) {

  int Left_forward = (vR >= 0) ? HIGH : LOW;
  int Right_forward = (vL >= 0) ? HIGH : LOW;

  digitalWrite(STBY, HIGH);
  analogWrite(PWMA, abs(vR));
  analogWrite(PWMB, abs(vL));

  digitalWrite(AIN1, Left_forward);
  digitalWrite(AIN2, (1 - Left_forward));
  digitalWrite(BIN1, (1 - Right_forward));
  digitalWrite(BIN2, Right_forward);
}
void blind_walking()
{
  MotorWrite(100, 100);
  if (digitalRead(Sensor[4]) == HIGH)
  {
    delay(500);
    MotorWrite(-100, 100);
    delay(300);
    cur_mode = ROUTE_LEFT;
  }
  else if (digitalRead(Sensor[0]) == HIGH)
  {
    delay(500);
    MotorWrite(100, -100);
    delay(300);
    cur_mode = ROUTE_RIGHT;
  }
}
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
    if (cur_mode == ROUTE_RIGHT)MotorWrite(70, -60);
    else if (cur_mode == ROUTE_LEFT)MotorWrite(-60, 70);
    else MotorWrite(70, -50);
  }
  else {
    average = total / cnt; // calculate the current average
    if (cnt <= 3) {
      if (turning == 1)
      {
        cnt_step++;//the car has turned -> next step
        turn_mode_cur = turn_mode_input[min(cnt_step, num_of_step - 1)];
      }
      turning = 0;
      double V_sR = 100; // the velocity of going straight (CHANGE HERE!)
      double V_sL = 100;
      double K_p = 15; // P control term (CHANGE HERE!)
      double K_i = 0;  // I control term (CHANGE HERE!)
      double K_d = 0;  // D control term (CHANGE HERE!)

      offset = average - 2.0; // the ideal average is 2.0
      double diff = offset - last_offset; // the differential term

      MotorWrite(min(V_sR + K_p * offset - K_d * diff + K_i * sum_offset, 255), min(V_sL - K_p * offset + K_d * diff - K_i * sum_offset, 255));
    }
    else {
      MotorWrite(100, 100);
      delay(700);
      if (cur_mode == ROUTE_LEFT)
      {
        MotorWrite(120, -120);
        delay(330);
        cur_mode=STOPPED;
      }
      else if (cur_mode == ROUTE_RIGHT)
      {
        MotorWrite(-120, 120);
        delay(330);
        cur_mode=STOPPED;
      }
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(Sensor[i], INPUT); //set sensor -> input
  }
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(Rx, INPUT);
  pinMode(Tx, OUTPUT);
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
  // put your main code here, to run repeatedly:
  Serial.print(Sensor_hit());
  Serial.println(Sensor[4]);
  while (BT.available())
  {
    String ss = BT.readStringUntil('\n');
    Serial.println(ss);
    if (ss == "start")
    {
      cur_mode = STARTED;
    }
  }
  switch (cur_mode)
  {
    case STARTED:
      blind_walking();
      break;
    case STOPPED:
      MotorWrite(0.0, 0.0);
      break;
    default:
      adjust();
  }

  if (!mfrc522->PICC_IsNewCardPresent()) {
    goto FuncEnd;
  } //PICC_IsNewCardPresent()：是否感應到新的卡片?
  if (!mfrc522->PICC_ReadCardSerial()) {
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
  /*MotorWrite(0,0);
    delay(100);
    MotorWrite(100,100);
    delay(100);*/
FuncEnd:; // goto 跳到這.
}
