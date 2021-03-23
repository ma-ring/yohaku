#include "BLEDevice.h"
#include "Free_Fonts.h"
#include "SPI.h"
#include "TFT_eSPI.h"

#define BLE_MODE_TEXT 'A'
#define BLE_MODE_DRAW_S 'B'
#define BLE_MODE_DRAW 'C'
#define BLE_MODE_DRAW_E 'D'

static String DEVICE_NAME = "TranslatorMask";
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
BLECharacteristic *pCharacteristic;   // キャラクタリスティック
bool deviceConnected = false;           // デバイスの接続状態
bool bInAlarm  = false;                 // デバイス異常判定

TFT_eSPI tft = TFT_eSPI();
#define FIRST_SCREEN 5
#define SECOND_SCREEN 21

String displayStr = "";
bool isDrawing = false;
bool isDraw = false;
int pos_x, pos_y, pre_x, pre_y;

//SET UP BLE /////////////////////////////////
//callback for server 
class functionServerCallBacks : public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    Serial.println("BLE CONNECTED");
  }
  void onDisconnect(BLEServer* pServer){
    
    Serial.println("BLE DISCONNECTED");
  }
};
//callback for characteristic
class functionCharaCallBacks : public BLECharacteristicCallbacks{
  void onRead(BLECharacteristic* pCharacteristic){
    Serial.println("read");
  }
  void onWrite(BLECharacteristic* pCharacteristic){
    std::string str = pCharacteristic->getValue();
    char head = str[0];
    str =  str.erase(0,1);

    if(head == BLE_MODE_TEXT){
       displayStr = str.c_str();
//       for(int i = 0; i < str.length(); i++){
//         Serial.print(str[i]);
//       }
//       Serial.println("");
    }
    else if(head == BLE_MODE_DRAW_S){
      
      isDrawing = true;
      int token = str.find(",");
      String str_y = (str.substr(token + 1)).c_str();;
      String str_x = (str.erase(token)).c_str();
      pre_x = str_x.toInt();
      pre_y = str_y.toInt();
      pos_y = pre_x;
      pos_x = pre_y;
     
    }else if(head == BLE_MODE_DRAW){
       isDraw = true;
       int token = str.find(",");
       String str_y = (str.substr(token + 1)).c_str();;
       String str_x = (str.erase(token)).c_str();
       pos_x = str_x.toInt();
       pos_y = str_y.toInt();
       Serial.println("update");
       if(!isDrawing){
         pre_x = pos_x;
         pre_y = pos_y;
         isDrawing = true;
       }
       
    }else if(head == BLE_MODE_DRAW_E){
      
       isDrawing = false;
    }
  }
  
};
//setup
void setupBLE(){
  BLEDevice::init("TRANSLATOR MASK");
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

//TFT////////////////////////////////////////
void setupTFT(){
  pinMode(FIRST_SCREEN, OUTPUT);
  digitalWrite(FIRST_SCREEN, HIGH);
  pinMode(SECOND_SCREEN, OUTPUT);
  digitalWrite(SECOND_SCREEN, HIGH);
  digitalWrite(FIRST_SCREEN, LOW);
  digitalWrite(SECOND_SCREEN, LOW);
  
  tft.begin();
  tft.setRotation(1);
  tft.setFreeFont(FF18);
  tft.setTextSize(30);
  digitalWrite(FIRST_SCREEN, HIGH);
  digitalWrite(SECOND_SCREEN, HIGH);
  
}



//////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 
  setupTFT();
  setupBLE();
}

void loop() {
  // put your main code here, to run repeatedly:


  if(isDraw && isDrawing){
    digitalWrite(FIRST_SCREEN, LOW);
    tft.drawLine(pre_x, pre_y, pos_x, pos_y, TFT_WHITE);
    pre_x = pos_x;
    pre_y = pos_y;
    isDraw = false;
  }else{
    digitalWrite(FIRST_SCREEN, LOW);
    //tft.fillScreen(TFT_BLACK);
    //tft.drawString("TEST WRITING",10,10);
    tft.drawString(displayStr,10,50);
    digitalWrite(FIRST_SCREEN, HIGH);
  
    digitalWrite(FIRST_SCREEN, LOW);
    //tft.fillScreen(TFT_BLACK);
    //tft.drawString("TEST WRITING",10,10);
    tft.drawString(displayStr,10,50);
    digitalWrite(FIRST_SCREEN, HIGH);
  
    delay(1000);
  }

  
  
}
