#include "BleMobile.h"
#include "BleCore.h"

 #define BLINK_PIN  D4

//main
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("APP SETUP");

  //ble
  setupBleCore();
  setupBleMobile();

  pinMode(BLINK_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  loopBleMobile();
  loopBleCore();

  digitalWrite(BLINK_PIN, HIGH);
  delay(1000);
  digitalWrite(BLINK_PIN, LOW);
  delay(1000);
  
}
