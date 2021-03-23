#include <nRF5x_BLE_API.h>
#include "Wheel.h"

//whieel
Wheel wheel;
int movestate;
//acc
#define PIN_ACC_X D1
#define PIN_ACC_Y D0
#define PIN_ACC_Z D3
const int balanced= 255;
//BLE
BLE ble;
Ticker ticker;
bool isDebug = false;
bool isConnected = false;
#define BLINK_PIN_MOBILE 13
bool state = false;
//menber
#define OUTER_NAME "OUTER"
static uint16_t handle = BLE_CONN_HANDLE_INVALID;
uint8_t device_is_hrm = 0;
static uint8_t characteristic_is_fond = 0;
static uint8_t descriptor_is_found = 0;
//6d 28 e8 bf - 20 84 -4e df-9f fe-d0 6a 42 f5 2b 4f
static uint8_t service_uuid_arry[] ={0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
//static uint8_t service_uuid_arry[] ={0x6d, 0x28, 0xe8, 0xbf, 0x20, 0x84, 0x4e, 0xdf, 0x9f,0xfe, 0xd0, 0x6a, 0x42, 0xf5, 0x2b, 0x4f};
UUID service_uuid(service_uuid_arry);
//fe742b4b-7ff2-4e2c-a1d8-01dd20286d17
static uint8_t chars_uuid_arry[] ={0x71, 0x3D, 0x00, 0x03, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
//static uint8_t chars_uuid_arry[] ={0xfe,0x74,0x2b,0x4b,0x7f,0xf2,0x4e,0x2c,0xa1,0xd8,0x01,0xdd,0x20,0x28,0x6d,0x17};
UUID chars_uuid1(chars_uuid_arry);
static DiscoveredCharacteristic            chars_hrm;
static DiscoveredCharacteristicDescriptor  desc_of_chars_hrm(NULL,GattAttribute::INVALID_HANDLE,GattAttribute::INVALID_HANDLE,UUID::ShortUUIDBytes_t(0));

//methods
void connectcallback(const Gap::ConnectionCallbackParams_t *params);
void disconnection(const Gap::DisconnectionCallbackParams_t *paramss);
void scanCallBack(const Gap::AdvertisementCallbackParams_t *params);
void discoveredServiceCallBack(const DiscoveredService *service) ;
void discoveryTerminationCallback(Gap::Handle_t connectionHandle);
void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) ;
void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) ;
void hvxCallback(const GattHVXCallbackParams *params);
void onDataWriteCallback(const GattWriteCallbackParams *params);
void onDataReadCallback(const GattReadCallbackParams *params);
uint32_t ble_advdata_parser(uint8_t type, uint8_t advdata_len,  uint8_t *p_advdata, uint8_t *len,uint8_t *p_field_data);
void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) ;

//-------------------------------------------------------------------------------------------------------------------------------------------
void callback(){
  //ble
  ble.waitForEvent();
//  uint8_t sendData[1] = {1};
//  // ble.gattServer().write(handle,sendData,1,false);
//  if(isConnected){
//     uint8_t temp[] = "abcde";
//  //ble.writeValue(device.connected_handle, device.service.chars[0].chars.value_handle, 5, temp);
////  ble.gattClient().write(GattClient::GATT_OP_WRITE_CMD, 
////                          handle,
////                          desc_of_chars_hrm.getAttributeHandle()
////                          ,1,sendData);
//// Serial.println("write");
////  }
 
}

void setup() {
  if(isDebug){
     Serial.begin(9600);
     Serial.println("CORE START");
  }
  movestate =  MOTER_GO;
  // put your setup code here, to run once:
  //駆動部
  //wheel.Setup(1,2,0,3,4,5);
  wheel.setPWM(true);
  wheel.Speed(500);
  wheel.Setup();
  //wheel.mode = MOTER_GO;
  //wheel.setPWM(true);
  //wheel.Speed(1000);
  //wheel.Speed(255);
  
  //BLE
  ble.init();
  //callback
  ble.onConnection(connectcallback);
  ble.onDisconnection(disconnection);
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
  ble.gattClient().onHVX(hvxCallback);
  ble.gattClient().onDataWrite(onDataWriteCallback);
  ble.gattClient().onDataRead(onDataReadCallback);

  //scan
  ble.setScanParams(1000, 200, 0, false);
  ble.setActiveScan(true);
  ble.startScan(scanCallBack);
  ticker.attach(callback, 1);
 
  pinMode(BLINK_PIN_MOBILE, OUTPUT);
  //digitalWrite(BLINK_PIN_MOBILE, HIGH);
  //ACC
  pinMode(PIN_ACC_X, INPUT);
  pinMode(PIN_ACC_Y, INPUT);
  pinMode(PIN_ACC_Z, INPUT);
}

void loop() {
  // puxt your main code here, to run repeatedly:
  int accZ = analogRead(PIN_ACC_Z);
  wheel.Loop();
  wheel.Speed(2000 - abs(accZ-255) * 2000 / 255);
}


//-----------------------------------------------------------------------------------------------------------------------------
void connectcallback( const Gap::ConnectionCallbackParams_t *params)
{
  //Serial.println("Connection callback Mobile");
  isConnected  = true;
  //デバイスの検索
  //discoveredServiceCallBack：Serviceを探す
  //discoveredCharacteristicCallBack:Characteristicを探してUUIDをと取得
  handle = params->handle;
  ble.gattClient().launchServiceDiscovery(params->handle, NULL, discoveredCharacteristicCallBack, service_uuid);
}
void disconnection(const Gap::DisconnectionCallbackParams_t *paramss)
{
  //リセット  
  //Serial.println("Disconnected, start to scanning");
  device_is_hrm = 0;
  characteristic_is_fond = 0;
  descriptor_is_found = 0;
  digitalWrite(BLINK_PIN_MOBILE,LOW);
  if(isDebug){
    Serial.println("disconnected");
  }
  ble.startScan(scanCallBack);
} 
//周辺危機を探索
void scanCallBack(const Gap::AdvertisementCallbackParams_t *params)
{
  uint8_t index;
 
  uint8_t len;
  uint8_t adv_name[31];
  if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
       //Serial.print("Complete name is : ");
      //Serial.println((const char*)adv_name);  

    if(memcmp(OUTER_NAME, adv_name, len) == 0x00) {
 
      ble.stopScan();
      device_is_hrm = 1;
      ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
      //Serial.println("Got device, stop scan Mobile");
      digitalWrite(BLINK_PIN_MOBILE, HIGH);
    }
  }

}
void discoveredServiceCallBack(const DiscoveredService *service) {
  //Serial.println("\r\n----Service Discovered");

  if(service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
    //Serial.println(service->getUUID().getShortUUID(), HEX);
  }
  else {
    uint8_t index;
    const uint8_t *uuid = service->getUUID().getBaseUUID();
    for(index=0; index<16; index++) {
      //Serial.print(uuid[index], HEX);
      //Serial.print(" ");
    }
    //Serial.println(" ");
  }
}
//ペアリング後の処理　Notifyを受け取る準備
void discoveryTerminationCallback(Gap::Handle_t connectionHandle){
  //Serial.println("\r\n----CdiscoveryTermination");
 if(characteristic_is_fond == 1) {
  //Serial.println("\r\n----Characteristic Discovered rry");
    ble.gattClient().discoverCharacteristicDescriptors(chars_hrm, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
  }
}
//ペリフェラル側のNotifyにCCCD(0x290)2があるかのチェック
void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) {
 if(params->descriptor.getUUID().getShortUUID() == 0x2902) {
    // Save characteristic info (CCCDを使う＝notifyでCentralに通知することを許可=advertiser）
    descriptor_is_found = 1;
    desc_of_chars_hrm = params->descriptor;
  }
}
//ペリフェラルから通知を受け取る準備
void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) {
  if(descriptor_is_found) {
    //Serial.println("Open HRM notify");
    //valueを送ってNotifyを許可
    uint16_t value = 0x0001;
    ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_hrm.getConnectionHandle(), desc_of_chars_hrm.getAttributeHandle(), 2, (uint8_t *)&value);
  }
  
}
//Notify通知かindicateが来た時の処理
void hvxCallback(const GattHVXCallbackParams *params)
{ 
 if(isDebug){
    Serial.println(params->data[0]);
  }
   digitalWrite(BLINK_PIN_MOBILE, !digitalRead(BLINK_PIN_MOBILE));

  
//  if(params->data[0] == MOTER_GO){
//    //Serial.print("GO");
//    //wheel.Go();;
//    wheel.setMoveMode(MOTER_GO);
//  }
//  if(params->data[0] == MOTER_RIGHT){
//    //Serial.print("RIGHT");
//    //wheel.Right);
//    wheel.setMoveMode(MOTER_RIGHT);
//  }
//  if(params->data[0] == MOTER_LEFT){
//    //Serial.print("LEFT");
////    wheel.Left();
//     wheel.setMoveMode(MOTER_LEFT);
//  
//  if(params->data[0] == MOTER_BACK){
//    //Serial.print("BACK");
//   // wheel.Stop();
//   wheel.setMoveMode(MOTER_BACK);
//  }  
//  if(params->data[0] == MOTER_STOP){
//    //Serial.print("BACK");
//   // wheel.Stop();
//   wheel.setMoveMode(MOTER_STOP);
//  }
  //movestate = params->data[0];
  wheel.mode = params->data[0];
  //Serial.println(params->data[0]);
  
}
//Write
void onDataWriteCallback(const GattWriteCallbackParams *params)
{ //Serial.println("GattClient write");
}
//Read
void onDataReadCallback(const GattReadCallbackParams *params)
{ //Serial.println("GattClinet read");
}
//デコーダー
uint32_t ble_advdata_parser(uint8_t type, uint8_t advdata_len,  uint8_t *p_advdata, uint8_t *len,uint8_t *p_field_data)
{
  uint8_t index=0;
  uint8_t field_length, field_type;

  while(index<advdata_len) {
    field_length = p_advdata[index];
    field_type   = p_advdata[index+1];
    if(field_type == type) {
      memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
      *len = field_length - 1;
      return NRF_SUCCESS;
    }
    index += field_length + 1;
  }
  return NRF_ERROR_NOT_FOUND;
}
//Caractersitcを発見した時の処理（onConnectinonで呼ばれる）
void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) {
  //Serial.println("\r\n----Characteristic Discovered3");

//  if(chars->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
//    Serial.println(chars->getUUID().getShortUUID(), HEX);
////    if(chars->getUUID().getShortUUID() == 0x2A37) {
////      Serial.println("Found HRM characteristic ");
////      characteristic_is_fond = 1;
////      chars_hrm = *chars;
////    }
//  }
//  else {
//    uint8_t index;
//    const uint8_t *uuid = chars->getUUID().getBaseUUID();
//    for(index=0; index<16; index++) {
//      Serial.print(uuid[index], HEX);
//    }
//  }
  if(chars->getProperties().notify() == 0x01){
   // Serial.println("found notify");
    characteristic_is_fond = 1;
    chars_hrm = *chars;
  }
}
