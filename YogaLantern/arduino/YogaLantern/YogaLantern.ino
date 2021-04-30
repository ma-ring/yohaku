#include "HeartBeat.h"
#include "MyHR.h"
#include "D2to40.h"
#include "MyPWM.h"
#define PIN_R 8
#define PIN_G 9
#define PIN_B 10
#define PIN_FREC_MOTER 6
#define PIN_ECG_IN A5
#define WAIT_SEC  3000  //Length of time for contact continuation judgment
#define FR_WEAK 500
#define FR_STRONG 1024
#define TIME_FIRST_TATCH_THRETH 500
int ecg[76];//ECG data buffer
double ecg_out[10];//ECG data buffer
double ecg_rms = 0; //ECG data output
double ecg_o = 0; //ECG data output
int t_cur = 0; //current Time
int t_pre = 0; //present Time
int t_wait = 0;
int t_wait0 = 0;
int src = 0; //sampling rate count
int ecg_last = 0;
bool is_ecg_ready = false;

//flag
bool isFrec1 = false;
bool isLED1 = false;
bool is_keeping_tatching = false;
bool is_tatching = false;

int t = 1;
int h2 = 0;
int stopCount = 0;
int r, g, b = 0;


MyPWM ledR;
MyPWM ledG;
MyPWM ledB;
MyPWM vib_motor;

void InitOutputVib();
void InitOutputLed();

void setup() {
  // initialize the serial communication:
  //  Serial.begin(57600);
  //  Serial.println("Serial begin");

  //Moter
  vib_motor.setup(PIN_FREC_MOTER, 0);

  //heartBeat
  HeartBeatSetup();

  //LED
  ledR.setup(PIN_R, 0);
  ledG.setup(PIN_G, 0);
  ledB.setup(PIN_B, 0);
  t_pre = micros();
  t_wait0 = millis();

  vib_motor.write(FR_STRONG);
  vib_motor.loop();
  digitalWrite(PIN_FREC_MOTER, HIGH);
  delay(3000);
  InitOutputVib();
  digitalWrite(PIN_FREC_MOTER, LOW);
}

void loop() {
  int h, s, v;
  //Wait for a bit to keep serial data from saturating

  t_cur = micros();//current time

  //250[Hz]
  if (t_cur - t_pre > 4000) {
    //update current time
    t_pre = t_cur;

    //update ecg val
    ecg_last = HeartBeatLoop();

    //tatch detection
    if (ecg_last < 900) {
      is_tatching = true;
    } else {
      is_tatching = false;
    }

    //tatch reaction
    if (is_tatching) {

      isFrec1 = false;
      isLED1 = false;

      //judge keeping tatching time buff
      t_wait = millis() - t_wait0;

      //Vibrate the motor only the first time
      if (t_wait < TIME_FIRST_TATCH_THRETH) {
        vib_motor.write(FR_WEAK);

        isFrec1 = true;
        isLED1 = false;
      } else if (t_wait < WAIT_SEC ) {
        vib_motor.write(0);
        vib_motor.stop();
        isFrec1 = false;
        isLED1 = false;
      } else {
        is_keeping_tatching = true;
      }
    } else {
      is_tatching = false;
      isFrec1 = false;
      isLED1 = false;
      t_wait0 = millis();
      InitOutputVib();
      InitOutputLed();
    }

    //process of ECG
    fifo(ecg, ecg_last);

    //Whether all filter buffers are filled with ecg waveform data
    if (is_ecg_ready) {
      ecg_o = filterBP(ecg);
      ecg_out[src % 10] = ecg_o;
      for (int j = 1; j < 10; j++) {
        ecg_rms += pow(ecg_out[j], 2);
      }
      ecg_rms /= 10;
      ecg_rms = sqrt(ecg_rms);
      //Serial.println((float)ecg_last);
      //Serial.print(" ::: ");

      if (is_tatching && ( t_wait > WAIT_SEC ) ) {
        h = ecg_rms * 4;
        if (h > 90) {
          s = 255;
          v = 255;
          if (h2 > 100) {
            h2 = 0;
          } else {
            h2 += 10;
          }
          //HSV2RGB(h , s, v, r, g, b);
          r = 5;
          g = 3;
          b = 0;
          ledR.write(r * h2);
          ledG.write(g * h2);
          ledB.write(b );
          vib_motor.write( h2 );
          isFrec1 = true;
          isLED1 = true;
        } else {
          h = 0;
          isLED1 = true;
          ledR.write(r * h2);
          ledG.write(g * h2);
          ledB.write(b);
          vib_motor.write( h2 );
          isFrec1 = true;
        }
      }
    }
    src++;
    if (src > BL - 1) {
      src = 0;
      is_ecg_ready = true;
    }
  }
  //viblate motor
  if (isFrec1 ) {
    vib_motor.loop();
  } else {
    InitOutputVib();
  }
  if (isLED1  ) {
    ledR.loop();
    ledG.loop();
    ledB.loop();
  } else {
    InitOutputLed();
  }
}

void InitOutputVib() {
  vib_motor.write(0);
  vib_motor.stop();
}
void InitOutputLed() {
  ledR.write(0); ledG.write(0); ledB.write(0);
  ledR.stop(); ledG.stop(); ledB.stop();
}
