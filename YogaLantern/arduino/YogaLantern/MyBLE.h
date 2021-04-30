#include "BLEDevice.h"

int heartRate = 1000; //0-1023
int sortedVal = 0; //0-4096 -> 0-120

static std::string DEVICE_NAME = "YogaLantern";
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
BLECharacteristic *pCharacteristic;   // キャラクタリスティック
bool deviceConnected = false;           // デバイスの接続状態
bool bInAlarm  = false;                 // デバイス異常判定

//SET UP BLE /////////////////////////////////
//callback for server 
class functionServerCallBacks : public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    Serial.println("BLE CONNECTED");
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer){
    Serial.println("BLE DISCONNECTED");
    deviceConnected = false;
  }
};
//callback for characteristic
class functionCharaCallBacks : public BLECharacteristicCallbacks{
  void onRead(BLECharacteristic* pCharacteristic){
    pCharacteristic->setValue(heartRate);
  }
  void onWrite(BLECharacteristic* pCharacteristic){
    std::string str = pCharacteristic->getValue();
    String val(str.c_str());
    sortedVal = val.toInt();
    Serial.println(sortedVal);
  }
  
};
//setup
void setupBLE(){
  Serial.begin(9600);
  BLEDevice::init(DEVICE_NAME);
  //Create server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new functionServerCallBacks());
  //Create service
  BLEService *pService = pServer->createService(serviceUUID);
  //Create characteristics
  pCharacteristic = pService->createCharacteristic(charUUID, 
                              BLECharacteristic::PROPERTY_READ   |
                              BLECharacteristic::PROPERTY_WRITE  |
                              BLECharacteristic::PROPERTY_NOTIFY |
                              BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic->setCallbacks(new functionCharaCallBacks);
 
  //Start service
  pService->start();
  //Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void updateBLE(int val){
  if(deviceConnected){
    //
  }
  else{
    //
    sortedVal = 120 * val/ 1023;
  }
}
