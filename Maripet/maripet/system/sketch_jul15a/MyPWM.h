#include <Arduino.h>
class MyPWM{
private:
  int pin;
  int highTime;
  int nextHighTime;
  const int itv = 2000;
  const int res = 1000;
  unsigned long preTime;
  bool flagHIGH = true;
  
public:
  void write(int pwm){
      nextHighTime = itv * pwm / res; 
  }
  void loop(){
    unsigned long currenttime = micros();
    if(flagHIGH){
      if(currenttime - preTime > highTime){
         digitalWrite(pin, LOW);
         flagHIGH = false;
         preTime = currenttime;
      }
    }
    else{
      if(currenttime - preTime > itv - highTime){
        digitalWrite(pin, HIGH);
        flagHIGH = true;
        preTime = currenttime;

        if(highTime != nextHighTime){ highTime = nextHighTime;}
      }
    }
  }
  void setup(int _pin, int pwm){
    pin = _pin;
    pinMode(pin, OUTPUT);
    highTime = itv * pwm / res; 
    write(pwm);
    //Serial.println(nextHighTime);
  }
};
