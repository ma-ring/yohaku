#include "Wheel.h"
#include "Arduino.h"

#define pinR0 2 //D2
#define pinR1 1 //D1
#define pinL0 3 //D3
#define pinL1 0 //D0
#define pinR0_pwm D6
#define pinR1_pwm D7
#define pinL0_pwm D9
#define pinL1_pwm D8
#define PWM_DIG HIGH
#define TURN 200

void Wheel::PWM_Write(int pinR, int pinL, int pwmspeed) {
  //Serial.println(pwmspeed);
  digitalWrite(pinR, HIGH);
  digitalWrite(pinL, HIGH);
  delayMicroseconds(SPEED_MAX - pwmspeed);
  digitalWrite(pinR, LOW);
  digitalWrite(pinL, LOW);
  delayMicroseconds(pwmspeed);
}

void Wheel::Setup(int r0, int r1, int l0, int l1, int r_pwm, int l_pwm) {
  //  pinR0 = r0;
  //  pinL0 = l0;
  //  pinR1 = r1;
  //  pinL1 = l1;
  //  pinR_pwm = r_pwm;
  //  pinL_pwm = l_pwm;
  //  pinMode(pinR0, OUTPUT);
  //  pinMode(pinL0, OUTPUT);
  //  pinMode(pinR1, OUTPUT);
  //  pinMode(pinL1, OUTPUT);
  //  pinMode(pinR_pwm, OUTPUT);
  //  pinMode(pinL_pwm, OUTPUT);
}
void Wheel::setPWM(bool state) {
  mode_pwm = state;
}
void Wheel::Setup() {
  //pinR0 = D1;
  //  pinL0 = D2;
  //  pinR1 = D0;
  //  pinL1 = D3;
  if (mode_pwm) {
    pinMode(pinR0_pwm, OUTPUT);
    pinMode(pinR1_pwm, OUTPUT);
    pinMode(pinL0_pwm, OUTPUT);
    pinMode(pinL1_pwm, OUTPUT);
    digitalWrite(pinR0_pwm,LOW);
    digitalWrite(pinR1_pwm,LOW);
    digitalWrite(pinL0_pwm,LOW);
    digitalWrite(pinL1_pwm,LOW);
  }
  else {
    pinMode(pinR0, OUTPUT);
    pinMode(pinL0, OUTPUT);
    pinMode(pinR1, OUTPUT);
    pinMode(pinL1, OUTPUT);
  }

}
void Wheel::Go() {
  //  Stop();
  //  delay(1000);
  if (mode_pwm) {
    // Serial.println("GO");
    digitalWrite(pinR0_pwm, PWM_DIG);
    digitalWrite(pinL0_pwm, PWM_DIG);
    PWM_Write(pinR1_pwm, pinL1_pwm, pwm_speed);
  }
  else {
    digitalWrite(pinR0, HIGH);
    digitalWrite(pinR1, LOW);
    digitalWrite(pinL0, HIGH);
    digitalWrite(pinL1, LOW);
  }
}
void Wheel::setMoveMode(int mode) {
  if (moveMode != mode) {
    fadecount = 0;
    //Stop();
    // Serial.println("MODE");
    turnCount = 0;
    //delay(500);
  }

  if (fadecount < STOP_FADE) {
    fadecount++;
    float rate = (STOP_FADE - fadecount) / STOP_FADE;
    Speed(SPEED_MAX * rate);
  } 
  else {
    switch (mode) {
      case MOTER_GO: {
          Go();
          break;
        }
      case MOTER_RIGHT: {
          Right();
          break;
        }
      case MOTER_LEFT: {
          Left();
          break;
        }
      case MOTER_STOP: {
          Stop();
          break;
        }
      case MOTER_BACK: {
          Back();
          break;
        }
    }
  }

  moveMode = mode;

}
void Wheel::Stop() {

  if (mode_pwm) {
    digitalWrite(pinR0_pwm, HIGH);
    digitalWrite(pinR1_pwm, HIGH);
    digitalWrite(pinL0_pwm, HIGH);
    digitalWrite(pinL1_pwm, HIGH);
  }
  else {
    digitalWrite(pinR0, LOW);
    digitalWrite(pinR1, LOW);
    digitalWrite(pinL0, LOW);
    digitalWrite(pinL1, LOW);
  }


}

void Wheel::Right() {
  //  Stop();
  //  delay(1000);
  if (turnCount < TURN) {
    if (mode_pwm) {
      digitalWrite(pinR0_pwm, PWM_DIG);
      digitalWrite(pinL1_pwm, PWM_DIG);
      PWM_Write(pinR1_pwm, pinL0_pwm, pwm_speed);
    } else {
      digitalWrite(pinR0, HIGH);
      digitalWrite(pinR1, LOW);
      digitalWrite(pinL0, LOW);
      digitalWrite(pinL1, HIGH);
    }
    turnCount++;
  }
  else {
    mode = MOTER_GO;
  }


}

void Wheel::Left() {
  //  Stop();
  //  delay(1000);
  if (turnCount < TURN) {
    if (mode_pwm) {
      digitalWrite(pinR1_pwm, PWM_DIG);
      digitalWrite(pinL0_pwm, PWM_DIG);
      PWM_Write(pinR0_pwm, pinL1_pwm, pwm_speed);
    }
    else {
      digitalWrite(pinR0, LOW);
      digitalWrite(pinR1, HIGH);
      digitalWrite(pinL0, HIGH);
      digitalWrite(pinL1, LOW);
    }
    turnCount++;
  } else {
    mode = MOTER_GO;
  }


}

void Wheel::Speed(int pwm) {
  pwm_speed = pwm;
  if (pwm_speed > SPEED_MAX) pwm_speed = SPEED_MAX;
  if (pwm_speed < 0) pwm_speed = 0;
}
void Wheel::Back() {
  //  Stop();
  //  delay(1000);
  if (mode_pwm) {
    digitalWrite(pinR1_pwm, PWM_DIG);
    digitalWrite(pinL1_pwm, PWM_DIG);
    PWM_Write(pinR0_pwm, pinL0_pwm, pwm_speed);
  }
  else {
    digitalWrite(pinR0, LOW);
    digitalWrite(pinR1, HIGH);
    digitalWrite(pinL0, LOW);
    digitalWrite(pinL1, HIGH);
  }
}

void Wheel::Loop() {
  setMoveMode(mode);
}
