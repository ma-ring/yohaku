#include <Arduino.h>
class MyPWM {
  private:
    int pin;
    int highTime = 0;
    int nextHighTime;
    int pastTime = 0;
    const int itv = 200;
    const int res = 1024;
    unsigned long preTime;
    bool flagHIGH = true;

  public:
    void write(int pwm) {
      pwm = max(0, pwm);
      pwm = min(res, pwm);
      nextHighTime = ( itv * pwm ) / res;
    }
    void loop() {
      unsigned long currenttime = micros();
      pastTime = currenttime - preTime;
      if ( pastTime > itv) {
        flagHIGH = true;
        preTime = micros();
      } else if ( pastTime > nextHighTime ) {
        flagHIGH = false;
      }
      
      if (flagHIGH) {
        digitalWrite(pin , HIGH);
      } else {
        digitalWrite(pin , LOW);
      }
    }

    void setup(int _pin, int pwm) {
      pin = _pin;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      highTime = itv * pwm / res;
      write(pwm);
      preTime = micros();
      //Serial.println(nextHighTime);
    }

    void stop() {
      digitalWrite(pin, LOW);
      write(0);
      flagHIGH = false;
    }

};
