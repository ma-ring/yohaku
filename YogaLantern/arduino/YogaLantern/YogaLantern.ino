
#include "HeartBeat.h"
#include "MyHR.h"
#include "D2to40.h"
#include "MyPWM.h"
#define PIN_R 8
#define PIN_G 9
#define PIN_B 10
#define PIN_FREC_MOTER 13

int ecg[76];//ECG data buffer
double ecg_out[10];//ECG data buffer
double ecg_rms = 0; //ECG data output
double ecg_o = 0; //ECG data output
int t_cur = 0; //current Time
int t_pre = 0; //present Time
int wait_sec = 3000;
int t_wait = 0;
int t_wait0 = 0;
#define fr_weak 500
int src = 0; //sampling rate count
int ecg_last = 0;
bool ecg_standby = false;
bool flag_tatch = false;
bool isFrec1 = false;
bool isLED1 = false;
int stopCount = 0;
int h;

MyPWM ledR;
MyPWM ledG;
MyPWM ledB;
MyPWM frec_Moter;

void setup() {
  // put your setup code here, to run once:
  // initialize the serial communication:
  Serial.begin(57600);
  Serial.println("Serial begin");
  //analogReference(EXTERNAL);
  frec_Moter.setup(PIN_FREC_MOTER, 0);
  //LED
  //pinMode(13,OUTPUT);

  ledR.setup(PIN_R, 0);
  ledG.setup(PIN_G, 0);
  ledB.setup(PIN_B, 0);
  t_wait0 = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  int  s, v;
  //Wait for a bit to keep serial data from saturating
  t_cur = micros();
  if (t_cur - t_pre > 4000) {
    t_pre = t_cur;
    ecg_last = analogRead(A0);
    if (ecg_last < 900) {
      flag_tatch = true;
      isFrec1 = true;
      isLED1 = false;
      t_wait = millis() - t_wait0;
      if (t_wait < 500) {
        frec_Moter.write(fr_weak);
        //                                Serial.print("status: wait=now, val<600  :");
        //                               Serial.println(analogRead(10));
      } else if (t_wait < wait_sec ) {
        //        frec_Moter.write(0);
        //                             Serial.print("status: wait=end, val<600  :");
        //                           Serial.println(analogRead(10));
        frec_Moter.stop();
        isFrec1 = false;
        isLED1 = false;
      }
    } else {
      flag_tatch = false;
      isFrec1 = false;
      isLED1 = false;
      t_wait0 = millis();
      frec_Moter.stop();
      //                   Serial.print("status: wait=no, val>600  :");
      //             Serial.println(analogRead(10));
    }
    fifo(ecg, ecg_last);
    if (ecg_standby) {
      ecg_o = filterBP(ecg);
      ecg_out[src % 10] = ecg_o;
      for (int j = 1; j < 10; j++) {
        ecg_rms += pow(ecg_out[j], 2);
      }
      ecg_rms /= 10;
      ecg_rms = sqrt(ecg_rms);
      //                    Serial.println((float)ecg_last);
      //      Serial.print(" ::: ");
      if (flag_tatch && t_wait > wait_sec) {
        h = ecg_rms * 4;
        //        if (h > 80) {
        s = 255;
        v = 255;
        int r, g, b = 0;
        HSV2RGB(h , s, v, r, g, b);
        ledR.write(r * 2);
        ledG.write(g * 2);
        ledB.write(b * 2);
        isLED1 = true;
        //        Serial.println(t_cur - t_pre);
        //        } else {
        //        h = 0;
        //        isLED1 = false;
        //        ledR.write(0);
        //        ledG.write(0);
        //        ledB.write(0);
        //        }
        frec_Moter.write(h);
        Serial.println((float)h);
      }
      //      Serial.println((double)ecg_rms);
    }
    src++;
    if (src > BL - 1) {
      src = 0;
      ecg_standby = true;
    }
  }
  if (isFrec1) {
    frec_Moter.loop();
  }
  else {
    frec_Moter.stop();
  }
  if (isLED1) {
    ledR.loop();
    ledG.loop();
    ledB.loop();
  }
  else {
    ledR.stop(); ledG.stop(); ledB.stop();
  }
}
