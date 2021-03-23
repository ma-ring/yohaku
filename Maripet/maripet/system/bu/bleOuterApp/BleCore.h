#include <nRF5x_BLE_API.h>

BLE bleCore;
#define BLINK_PIN_CORE D2
//menber
#define CORE_APP "Core"
uint8_t device_is_hrm_core = 0;
static uint8_t characteristic_is_fond_core = 0;
static uint8_t descriptor_is_found_core = 0;
//5a 3a 79 6f 1f 39 49 7d 93 f9 16 43 2c d2 c3 24
static uint8_t service_uuid_arry_core[] ={0x5a,0x3a ,0x79 ,0x6f ,0x1f ,0x39 ,0x49 ,0x7d ,0x93 ,0xf9 ,0x16 ,0x43 ,0x2c ,0xd2 ,0xc3 ,0x24};
UUID service_uuid_core(service_uuid_arry_core);
//f0 ea 41 86 7e bf 49 9d bc 81 7c 6d 83 a2 c6 07
static uint8_t chars_uuid_arry_core[] ={0xf0 ,0xea ,0x41 ,0x86 ,0x7e ,0xbf ,0x49 ,0x9d ,0xbc ,0x81 ,0x7c ,0x6d ,0x83 ,0xa2 ,0xc6 ,0x07};
UUID chars_uuid_core(chars_uuid_arry_core);
static DiscoveredCharacteristic            chars_hrm_core;
static DiscoveredCharacteristicDescriptor  desc_of_chars_hrm_core(NULL,GattAttribute::INVALID_HANDLE,GattAttribute::INVALID_HANDLE,UUID::ShortUUIDBytes_t(0));

//methods
void connectcallback_core(const Gap::ConnectionCallbackParams_t *params);
void disconnection_core(const Gap::DisconnectionCallbackParams_t *paramss);
void scanCallBack_core(const Gap::AdvertisementCallbackParams_t *params);
void discoveredServiceCallBack_core(const DiscoveredService *service) ;
void discoveryTerminationCallback_core(Gap::Handle_t connectionHandle);
void discoveredCharsDescriptorCallBack_core(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) ;
void discoveredDescTerminationCallBack_core(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) ;
void hvxCallback_core(const GattHVXCallbackParams *params);
void onDataWriteCallback_core(const GattWriteCallbackParams *params);
void onDataReadCallback_core(const GattReadCallbackParams *params);
uint32_t ble_advdata_parser_core(uint8_t type, uint8_t advdata_len,  uint8_t *p_advdata, uint8_t *len,uint8_t *p_field_data);
void discoveredCharacteristicCallBack_core(const DiscoveredCharacteristic *chars) ;

//------------------------------------------------------------------------------------------------
void setupBleCore(){
  bleCore.init();

  //callback
  bleCore.onConnection(connectcallback_core);
  bleCore.onDisconnection(disconnection_core);
  bleCore.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback_core);
  bleCore.gattClient().onHVX(hvxCallback_core);
  bleCore.gattClient().onDataWrite(onDataWriteCallback_core);
  bleCore.gattClient().onDataRead(onDataReadCallback_core);

  //scan
  bleCore.setScanParams(1000, 200, 0, false);
  bleCore.startScan(scanCallBack_core);

  pinMode(BLINK_PIN_CORE, OUTPUT);
  digitalWrite(BLINK_PIN_CORE,LOW);
}
void loopBleCore(){
  bleCore.waitForEvent();
}

//-------------------------------------------------------------------------------------------------
void connectcallback_core( const Gap::ConnectionCallbackParams_t *params)
{
  Serial.println("Connection callback core");

  
  //デバイスの検索
  //discoveredServiceCallBack：Serviceを探す
  //discoveredCharacteristicCallBack:Characteristicを探してUUIDをと取得
  if(device_is_hrm_core)
    bleCore.gattClient().launchServiceDiscovery(params->handle, discoveredServiceCallBack_core, discoveredCharacteristicCallBack_core, service_uuid_core, chars_uuid_core);
//  if(device_is_simple_periferal)
//    bleCore.gattClient().launchServiceDiscovery(params->handle, discoveredServiceCallBack_core, discoveredCharacteristicCallBack_core);
}
void disconnection_core(const Gap::DisconnectionCallbackParams_t *paramss)
{
  //リセット  
  Serial.println("Disconnected, start to scanning");
  device_is_hrm_core = 0;
  characteristic_is_fond_core = 0;
  descriptor_is_found_core = 0;
  bleCore.startScan(scanCallBack_core);
  digitalWrite(BLINK_PIN_CORE,LOW);
}
//周辺危機を探索
void scanCallBack_core(const Gap::AdvertisementCallbackParams_t *params)
{
  uint8_t index;
  uint8_t len;
  uint8_t adv_name[31];
  if( NRF_SUCCESS == ble_advdata_parser_core(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    
    Serial.print("Complete name len : ");
    Serial.println(len, DEC);
    Serial.print("Complete name is : ");
    Serial.println((const char*)adv_name);
    if(memcmp(CORE_APP, adv_name, len) == 0x00) {
      Serial.print("Complete name is : ");
      digitalWrite(BLINK_PIN_CORE, HIGH);
      Serial.println((const char*)adv_name);
      bleCore.stopScan();
      device_is_hrm_core = 1;
      bleCore.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
      Serial.println("Got device, stop scan Core");
      
  
    }
  }

}
void discoveredServiceCallBack_core(const DiscoveredService *service) {
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
void discoveryTerminationCallback_core(Gap::Handle_t connectionHandle){
  Serial.println("\r\n----Characteristic Discovered");
 if(characteristic_is_fond_core == 1) {
    bleCore.gattClient().discoverCharacteristicDescriptors(chars_hrm_core, discoveredCharsDescriptorCallBack_core, discoveredDescTerminationCallBack_core);
  }
}
//ペリフェラル側のNotifyにCCCD(0x290)2があるかのチェック
void discoveredCharsDescriptorCallBack_core(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) {
 if(params->descriptor.getUUID().getShortUUID() == 0x2902) {
    // Save characteristic info (CCCDを使う＝notifyでCentralに通知することを許可=advertiser）
    descriptor_is_found_core = 1;
    desc_of_chars_hrm_core = params->descriptor;
  }
}
//ペリフェラルから通知を受け取る準備
void discoveredDescTerminationCallBack_core(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) {
  if(descriptor_is_found_core) {
    Serial.println("Open HRM notify");
    //valueを送ってNotifyを許可
    uint16_t value = 0x0001;
    bleCore.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_hrm_core.getConnectionHandle(), desc_of_chars_hrm_core.getAttributeHandle(), 2, (uint8_t *)&value);
  }
}
//Notify通知かindicateが来た時の処理
void hvxCallback_core(const GattHVXCallbackParams *params)
{ 
  
  if(memcmp(params->data, "GO",2) == 0x00){Serial.print("GO1");}
  if(memcmp(params->data, "RIGHT",5) == 0x00){Serial.print("RIGHT2");}
  if(memcmp(params->data, "LEFT",4) == 0x00){Serial.print("RIGHT3");}
  if(memcmp(params->data, "BACK",4) == 0x00){Serial.print("BACK4");}
  
}
//Write
void onDataWriteCallback_core(const GattWriteCallbackParams *params)
{ Serial.println("GattClient write");}
//Read
void onDataReadCallback_core(const GattReadCallbackParams *params)
{ Serial.println("GattClinet read");}
//デコーダー
uint32_t ble_advdata_parser_core(uint8_t type, uint8_t advdata_len,  uint8_t *p_advdata, uint8_t *len,uint8_t *p_field_data)
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
void discoveredCharacteristicCallBack_core(const DiscoveredCharacteristic *chars) {
  Serial.println("\r\n----Characteristic Discovered Core");

  if(chars->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
    Serial.println(chars->getUUID().getShortUUID(), HEX);
    if(chars->getUUID().getShortUUID() == 0x2A37) {
      Serial.println("Found HRM characteristic ");
      characteristic_is_fond_core = 1;
      chars_hrm_core = *chars;
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
