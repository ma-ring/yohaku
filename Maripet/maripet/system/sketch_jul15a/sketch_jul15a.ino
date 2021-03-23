#include "MyPWM.h"
#define pin0 D1
#define pin1 D2
#define pin2 D0
#define pin3 D4
int t = 1;
int mtime =0;
MyPWM ledR;
MyPWM ledG;
MyPWM ledB;
Ticker ticker;
unsigned long preTime;
bool state = true;
void callback(){
    digitalWrite(pin0,HIGH);
    digitalWrite(pin1,HIGH);
    digitalWrite(pin2,HIGH);
    digitalWrite(pin3,HIGH);
    delayMicroseconds(t);
    //delay(1000);
    digitalWrite(pin0,LOW);
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,LOW);
    digitalWrite(pin3,LOW);
    delayMicroseconds(2000 - t);
    //delay(1000);
   
}

void setup(){
  //Serial.begin(9600);
//  pinMode(pin0,OUTPUT);
//  pinMode(pin1,OUTPUT);
//  pinMode(pin2,OUTPUT);  
//  pinMode(pin3,OUTPUT);
  pinMode(13,OUTPUT);
  ledR.setup(pin0,10);
  ledG.setup(pin1,50);
  ledB.setup(pin2,800);
  //delay(3000);
  //ticker.attach_us(callback,2000);
}

void loop(){
 ledR.loop();
 ledG.loop();
 ledB.loop();
}
