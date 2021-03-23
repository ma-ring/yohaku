#include <nRF5x_BLE_API.h>

BLE bleMobile;

#define BLINK_PIN_MOBILE D3

//menber
#define MOBILE_APP "Maripet"
uint8_t device_is_hrm = 0;
static uint8_t characteristic_is_fond = 0;
static uint8_t descriptor_is_found = 0;
//6d 28 e8 bf - 20 84 -4e df-9f fe-d0 6a 42 f5 2b 4f
static uint8_t service_uuid_arry[] ={0x6d, 0x28, 0xe8, 0xbf, 0x20, 0x84, 0x4e, 0xdf, 0x9f,0xfe, 0xd0, 0x6a, 0x42, 0xf5, 0x2b, 0x4f};
UUID service_uuid(service_uuid_arry);
//fe742b4b-7ff2-4e2c-a1d8-01dd20286d17
static uint8_t chars_uuid_arry[] ={0xfe,0x74,0x2b,0x4b,0x7f,0xf2,0x4e,0x2c,0xa1,0xd8,0x01,0xdd,0x20,0x28,0x6d,0x17};
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

//------------------------------------------------------------------------------------------------
void setupBleMobile(){
  bleMobile.init();

  //callback
  bleMobile.onConnection(connectcallback);
  bleMobile.onDisconnection(disconnection);
  bleMobile.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
  bleMobile.gattClient().onHVX(hvxCallback);
  bleMobile.gattClient().onDataWrite(onDataWriteCallback);
  bleMobile.gattClient().onDataRead(onDataReadCallback);

  //scan
  bleMobile.setScanParams(1000, 200, 0, false);
  bleMobile.startScan(scanCallBack);
 
  pinMode(BLINK_PIN_MOBILE, OUTPUT);
  digitalWrite(BLINK_PIN_MOBILE,LOW);
}
void loopBleMobile(){
  bleMobile.waitForEvent();  
}

//-------------------------------------------------------------------------------------------------
void connectcallback( const Gap::ConnectionCallbackParams_t *params)
{
  Serial.println("Connection callback Mobile");
  
  //デバイスの検索
  //discoveredServiceCallBack：Serviceを探す
  //discoveredCharacteristicCallBack:Characteristicを探してUUIDをと取得
  if(device_is_hrm)
    bleMobile.gattClient().launchServiceDiscovery(params->handle, discoveredServiceCallBack, discoveredCharacteristicCallBack, service_uuid, chars_uuid1);
//  if(device_is_simple_periferal)
//    bleMobile.gattClient().launchServiceDiscovery(params->handle, discoveredServiceCallBack, discoveredCharacteristicCallBack);
}
void disconnection(const Gap::DisconnectionCallbackParams_t *paramss)
{
  //リセット  
  Serial.println("Disconnected, start to scanning");
  device_is_hrm = 0;
  characteristic_is_fond = 0;
  descriptor_is_found = 0;
  digitalWrite(BLINK_PIN_MOBILE,LOW);
  bleMobile.startScan(scanCallBack);
} 
//周辺危機を探索
void scanCallBack(const Gap::AdvertisementCallbackParams_t *params)
{
  uint8_t index;
 
  uint8_t len;
  uint8_t adv_name[31];
  if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    

    if(memcmp(MOBILE_APP, adv_name, len) == 0x00) {
      Serial.print("Complete name is : ");
      Serial.println((const char*)adv_name);
      bleMobile.stopScan();
      device_is_hrm = 1;
      bleMobile.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
      Serial.println("Got device, stop scan Mobile");
      digitalWrite(BLINK_PIN_MOBILE, HIGH);
    }
  }

}
void discoveredServiceCallBack(const DiscoveredService *service) {
  Serial.println("\r\n----Service Discovered");

  if(service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
    Serial.println(service->getUUID().getShortUUID(), HEX);
  }
  else {
    uint8_t index;
    const uint8_t *uuid = service->getUUID().getBaseUUID();
    for(index=0; index<16; index++) {
      Serial.print(uuid[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
}
//ペアリング後の処理　Notifyを受け取る準備
void discoveryTerminationCallback(Gap::Handle_t connectionHandle){
  Serial.println("\r\n----Characteristic Discovered");
 if(characteristic_is_fond == 1) {
    bleMobile.gattClient().discoverCharacteristicDescriptors(chars_hrm, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
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
    Serial.println("Open HRM notify");
    //valueを送ってNotifyを許可
    uint16_t value = 0x0001;
    bleMobile.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_hrm.getConnectionHandle(), desc_of_chars_hrm.getAttributeHandle(), 2, (uint8_t *)&value);
  }
}
//Notify通知かindicateが来た時の処理
void hvxCallback(const GattHVXCallbackParams *params)
{ 
  
  if(memcmp(params->data, "GO",2) == 0x00){Serial.print("GO");}
  if(memcmp(params->data, "RIGHT",5) == 0x00){Serial.print("RIGHT");}
  if(memcmp(params->data, "LEFT",4) == 0x00){Serial.print("RIGHT");}
  if(memcmp(params->data, "BACK",4) == 0x00){Serial.print("BACK");}
  
}
//Write
void onDataWriteCallback(const GattWriteCallbackParams *params)
{ Serial.println("GattClient write");}
//Read
void onDataReadCallback(const GattReadCallbackParams *params)
{ Serial.println("GattClinet read");}
//デコーダー
uint32_t ble_advdata_parser(uint8_t type, uint8_t advdata_len,  uint8_t *p_advdata, uint8_t *len,uint8_t *p_field_data)
{
  uint8_t index = 0; 
  uint8_t field_length, field_type;
 
  while(index < advdata_len){
    field_length = p_advdata[index];
    field_type = p_advdata[index + 1];
    if(field_type == type){
      memcpy(p_field_data, &p_advdata[index + 2], (field_length -1));
      *len = field_length -1;
      return NRF_SUCCESS;
    }    
    index += field_length + 1;
  }
  return NRF_ERROR_NOT_FOUND;
}
//Caractersitcを発見した時の処理（onConnectinonで呼ばれる）
void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) {
  Serial.println("\r\n----Characteristic Discovered");

  if(chars->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
    Serial.println(chars->getUUID().getShortUUID(), HEX);
    if(chars->getUUID().getShortUUID() == 0x2A37) {
      Serial.println("Found HRM characteristic ");
      characteristic_is_fond = 1;
      chars_hrm = *chars;
    }
  }
  else {
    uint8_t index;
    const uint8_t *uuid = chars->getUUID().getBaseUUID();
    for(index=0; index<16; index++) {
      Serial.print(uuid[index], HEX);
    }
  }
}
