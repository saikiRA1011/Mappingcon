#include <DHT.h>
#include<Wire.h>

// MPU-6050のアドレス、レジスタ設定値
#define MPU6050_WHO_AM_I    0x75  // Read Only
#define MPU6050_PWR_MGMT_1  0x6B  // Read and Write
#define MPU6050_AX  0x3B
#define MPU6050_ADDR 0x68

#define dataPin 7
#define RCLK 8
#define SRCLK 9

#define leftMoter 10
#define rightMoter 11
#define drive 12
#define back 13

#define echo_pin 5
#define trig_pin 4
#define DHT_pin 3
#define DHT_type DHT11
// #define vs 8

// offset
#define AX_OFFSET -473
#define AY_OFFSET -1196
#define AZ_OFFSET 3403
#define GX_OFFSET 30
#define GY_OFFSET 19
#define GZ_OFFSET 18

DHT dht(DHT_pin,DHT_type);
/* 温度など計測値 */
float tempC=15.0;


char ok=false;

float offset[6];

void setOffset(){
  for(int i=0;i<100;i++){
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_AX);
    Wire.endTransmission();
    Wire.requestFrom(MPU6050_ADDR,14);
    while (Wire.available() < 14);
    short gomi;
    gomi=(Wire.read()<<8)|Wire.read();
    gomi=(Wire.read()<<8)|Wire.read();
    gomi=(Wire.read()<<8)|Wire.read();
    gomi=(Wire.read()<<8)|Wire.read();
    gomi=(Wire.read()<<8)|Wire.read();
    gomi+=(Wire.read()<<8)|Wire.read();
    gomi+=(Wire.read()<<8)|Wire.read();
  }
  for(int i=0;i<1000;i++){
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_AX);
    Wire.endTransmission();
    Wire.requestFrom(MPU6050_ADDR,14);
    while (Wire.available() < 14);
    short gomi;
    offset[0]+=(Wire.read()<<8)|Wire.read();
    offset[1]+=(Wire.read()<<8)|Wire.read();
    offset[2]+=(Wire.read()<<8)|Wire.read();
    gomi=(Wire.read()<<8)|Wire.read();
    offset[3]+=(Wire.read()<<8)|Wire.read();
    offset[4]+=(Wire.read()<<8)|Wire.read();
    offset[5]+=(Wire.read()<<8)|Wire.read();
  }
  for(int i=0;i<6;i++){
    offset[i]/=1000.0;
  }
}

// ドットマトリクスに使う
char i=-1;
char j=0;

void setup() {
  pinMode(dataPin,OUTPUT);
  pinMode(RCLK,OUTPUT);
  pinMode(SRCLK,OUTPUT);
  digitalWrite(RCLK,HIGH);
  
  Wire.begin();
  Serial.begin(9600);
  pinMode(echo_pin,INPUT);
  pinMode(trig_pin,OUTPUT);
  pinMode(DHT_pin,INPUT);
  pinMode(drive,OUTPUT);
  pinMode(back,OUTPUT);
  pinMode(leftMoter,OUTPUT);
  pinMode(rightMoter,OUTPUT);
//  pinMode(vs,OUTPUT);
  dht.begin();

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_WHO_AM_I);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission();

  //LPF設定
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x03);
  Wire.endTransmission();

  setOffset();

  i=0;
}

unsigned long tempTime=0;
unsigned long writeTimer=0;

// 模様を決める(上から順番)
unsigned char rowData[10][8] = {
  {0x18,0x24,0x42,0x81,0x81,0x42,0x24,0x18},  // ○
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},  // ブレーキ =
  {0x18,0x3C,0x18,0x18,0x18,0x3C,0x18,0x18},  // ゆっくり前進 小さい矢印
  {0x18,0x18,0x3C,0x18,0x18,0x18,0x3C,0x18},  // ゆっくり後退
  {0x0F,0x07,0x07,0x09,0x10,0x20,0x40,0x80},  // 右前
  {0xF0,0xE0,0xE0,0x90,0x08,0x04,0x02,0x01},  // 左前
  {0x80,0x40,0x20,0x10,0x09,0x07,0x07,0x0F},  // 右後
  {0x01,0x02,0x04,0x08,0x90,0xE0,0xE0,0xF0},  // 左後
  {0x18,0x3C,0x7E,0xFF,0x3C,0x3C,0x3C,0x3C},  // 前
  {0x3C,0x3C,0x3C,0x3C,0xFF,0x7E,0x3C,0x18}   // 後ろ
};
unsigned char colData[10][8] ={
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C},
  {0x04,0x08,0x10,0x20,0x04,0x08,0x10,0x20},
  {0x04,0x08,0x10,0x20,0x04,0x08,0x10,0x20},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
  {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80}
};

void loop() {
  if(millis()-writeTimer<10){
    if (Serial.available() > 0) {
      char x=Serial.read();
      if(0<=x && x<=8)
        i=x+1;
    }
  }
  
  else{
  String s="";
  if(abs(millis()-tempTime)>2000){
    float newTemp=dht.readTemperature();
    tempTime=millis();
    
    if(!isnan(newTemp)){
      tempC=newTemp;
    }
  }
  s+=String(tempC)+",";

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_AX);
  Wire.endTransmission();
  Wire.requestFrom(MPU6050_ADDR,14);
  while (Wire.available() < 14);
  float ax,ay,az,gx,gy,gz;
  short gomi;
  ax=(Wire.read()<<8)|Wire.read();
  ay=(Wire.read()<<8)|Wire.read();
  az=(Wire.read()<<8)|Wire.read();
  gomi=(Wire.read()<<8)|Wire.read();
  gx=(Wire.read()<<8)|Wire.read();
  gy=(Wire.read()<<8)|Wire.read();
  gz=(Wire.read()<<8)|Wire.read();

  s+=String(ax-offset[0]); s+=",";
  s+=String(ay-offset[1]); s+=",";
  s+=String(az-offset[2]); s+=",";
  s+=String(gx-offset[3]); s+=",";
  s+=String(gy-offset[4]); s+=",";
  s+=String(gz-offset[5]); s+=",";
//  Serial.print(ax);Serial.print(",");
//  Serial.print(ay);Serial.print(",");
//  Serial.print(az);Serial.print(",");
//  Serial.print(gx);Serial.print(",");
//  Serial.print(gy);Serial.print(",");
//  Serial.print(gz);Serial.print(",");
  
  digitalWrite(trig_pin,LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin,LOW);
  double Duration=pulseIn(echo_pin,HIGH);

  s+=String(Duration);
  Serial.println(s);

  writeTimer=millis();
  }

  if(i==1){
    digitalWrite(drive,HIGH);
    digitalWrite(back,HIGH);
    analogWrite(leftMoter,0);
    analogWrite(rightMoter,0);
  }
  else if(i==2){
    digitalWrite(drive,HIGH);
    digitalWrite(back,LOW);
    analogWrite(leftMoter,50);
    analogWrite(rightMoter,50);
  }
  else if(i==3){
    digitalWrite(drive,LOW);
    digitalWrite(back,HIGH);
    analogWrite(leftMoter,50);
    analogWrite(rightMoter,50);
  }
  else if(i==4){
    digitalWrite(drive,HIGH);
    digitalWrite(back,LOW);
    analogWrite(leftMoter,0);
    analogWrite(rightMoter,255);
  }
  else if(i==5){
    digitalWrite(drive,HIGH);
    digitalWrite(back,LOW);
    analogWrite(leftMoter,255);
    analogWrite(rightMoter,0);
  }
  else if(i==6){
    digitalWrite(drive,LOW);
    digitalWrite(back,HIGH);
    analogWrite(leftMoter,0);
    analogWrite(rightMoter,255);
  }
  else if(i==7){
    digitalWrite(drive,LOW);
    digitalWrite(back,HIGH);
    analogWrite(leftMoter,255);
    analogWrite(rightMoter,0);
  }
  else if(i==8){
    digitalWrite(drive,HIGH);
    digitalWrite(back,LOW);
    analogWrite(leftMoter,200);
    analogWrite(rightMoter,200);
  }
  else if(i==9){
    digitalWrite(drive,LOW);
    digitalWrite(back,HIGH);
    analogWrite(leftMoter,200);
    analogWrite(rightMoter,200);
  }
  
  if(-1<i){
    digitalWrite(RCLK,LOW);
    shiftOut(dataPin,SRCLK,MSBFIRST,rowData[i][j]);
    shiftOut(dataPin,SRCLK,MSBFIRST,0xFF^colData[i][j]);
    digitalWrite(RCLK,HIGH);
  }
  j++;
  j%=8;
}
