// Your sketch must #include this library, and the Wire library.
// (Wire is a standard library included with Arduino.):
#include <Arduino.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include "MyFilter.h"
#include "MyBLE.h"
#include "MyLedBlink.h"
#include "D_notch.h"
#include "D4to20.h"
#include "D2to40.h"
#define DEBUG

//pin設定
//const int P_PWM_LED = A17;//GPIO_27
const int P_PWM_LED = 16;//GPIO_16
const int P_ECG_IN = 33;//GPIO_33
//PWM設定
uint8_t ch1 = 0;//channel設定 0~15のchannelがあるらしい
bool on_flag = true;
//LED設定
static uint8_t brightness = 0;// 初期の明るさを指定
static int diff = 1;// 明るさの変動具合を設定（高いほど急に変わる）
//気圧センサ設定
#define LEN_PRESSURE 50
MyFilter pressure_sensor;
bool is_ready_pres = false;
//ECGセンサ設定
#define LEN_ECG 101
MyFilter ecg_sensor;
bool is_ready_ecg = false;
// You will need to create an SFE_BMP180 object, here called "pressure":
SFE_BMP180 pressure;

//timer
#define TIMER_CH 0//timer ch 
#define DIVIDER 80//Frequency division ratio. 1tic = F_clock / DIVIDER. Default F_clock is 80[MHz].
#define SAMPLING_PERIOD 4000//[usec]
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int cout_for_debug = 0;
volatile double ecg_tmp = 0;
volatile double ecg_filtered = 0;
volatile double ecg_val = 0;
void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  ecg_tmp = analogRead(P_ECG_IN);
  ecg_sensor.updateFifo(ecg_tmp);
  // ecg_filtered = ecg_sensor.filter(D_NOTCH, sizeof(D_NOTCH) / sizeof(double));
  ecg_filtered = ecg_sensor.filter(D_4to20, sizeof(D_4to20) / sizeof(double));

  ecg_val = ecg_sensor.MaxValue(ecg_filtered * ecg_filtered);

  //
  setEcg(ecg_val);
#ifdef DEBUG

  //  ecg_filtered = ecg_sensor.filter(D_NOTCH, sizeof(D_NOTCH));
  //  Serial.print("size of d_notch = ");
  //  Serial.print(sizeof(D_NOTCH));
  //    Serial.print("  ecg_tmp = ");
  //    Serial.println(ecg_tmp);
  //  Serial.print("  ecg_f = ");
  //  Serial.println(ecg_filtered);

  Serial.print("  ecg_val = ");
  Serial.println(ecg_val);
#endif

  cout_for_debug++;
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void setup()
{
  Serial.begin(500000);
  Serial.println("REBOOT");

  //ECG setup
  pinMode(P_ECG_IN, INPUT);
  ecg_sensor.mySetup(LEN_ECG, BAND_PASS_ECG);

  //PWM setup
  pinMode(P_PWM_LED, OUTPUT);
  //pwmの設定。最初の引数がchannel,次が周波数,最後が解像度（ここでは8bit = 256段階）
  Serial.print("PWM Fr= ");
  Serial.println(ledcSetup(ch1, 6400, 12));
  //ピンをチャンネルに接続
  ledcAttachPin(P_PWM_LED, ch1);

  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
  }
  pressure_sensor.mySetup(LEN_PRESSURE, MOVING_AVERAGE);

  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(TIMER_CH, DIVIDER, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, SAMPLING_PERIOD, true);

  // Start an alarm
  timerAlarmEnable(timer);


  //Countdown just before the start of the loop.
  Serial.println("ready to start.");
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("0");
}

void loop()
{
  char status;
  double T, P;

  // Start a temperature measurement:
  is_ready_pres = GetTempAndPress(T, P, pressure, pressure_sensor);
  //LED
  if (updateBLE(ecg_val)) {

  } else {
    brightness = blinkLed();
    //Serial.print("LED = ");
    //Serial.println(brightness);
  }
  // チャネル0に明るさを設定
  ledcWrite(ch1, brightness);

  //Timer
  xSemaphoreTake(timerSemaphore, 0);
}

int blinkLED() {
  sortedVal;
  return 0;
}

bool GetTempAndPress(double temp, double p, SFE_BMP180 p_bmp180, MyFilter myPres) {
  // You must first get a temperature measurement to perform a pressure reading.
  char status;
  status = p_bmp180.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    WaitCalcBMP180((int)status);
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.
    status = p_bmp180.getTemperature(temp);
    if (status != 0)
    {
      status = p_bmp180.startPressure(3);
      if (status != 0)
      {
        //Wait for the measurement to complete :
        WaitCalcBMP180((int)status);
        status = p_bmp180.getPressure(p, temp);
        p = p * 100; //raw data is [mb](=100[Pa])
        if (status != 0)
        {
          // Print out the measurement:
          myPres.updateFifo(p);
          double pressure_val = (p - myPres.movingAverage());
          //Serial.print("ac pressure: ");
          //Serial.print((pressure_val) * 100, 2);
          //Serial.println(" Pa ");

        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  if (is_ready_pres == false) {
    is_ready_pres = myPres.checkReady();
  }
  return is_ready_pres;
}

void WaitCalcBMP180(int wait_time) {
  int tmp_count = 0;
  unsigned long tmp_time = millis();
  while (tmp_count < wait_time) {
    tmp_count = millis() - tmp_time;
  }
}
