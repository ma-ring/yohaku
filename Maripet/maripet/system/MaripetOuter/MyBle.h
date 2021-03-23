#include <nRF5x_BLE_API.h>
#include <arduino.h>

BLE ble;

//control flag 
bool isMove = false;
bool isFrec = false;
bool isLED = false;
bool isDemoMode = false;
int demo_go_time = 3000;
#define PIN_BLE_MODE D4
bool BLE_CONNECT_MOBILE = false;

#define BLINK_PIN 13
// Connect handle
static uint16_t peripheral_handle = BLE_CONN_HANDLE_INVALID;
static uint16_t client_handle     = BLE_CONN_HANDLE_INVALID;

//menber for centeral roll
#define PERIFERAL_NAME "Maripet"
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

//menber for periferal roll
#define My_NAME "OUTER"
#define TXRX_BUF_LEN 20
//5a 3a 79 6f 1f 39 49 7d 93 f9 16 43 2c d2 c3 24
static uint8_t service_uuid_arry_perif[] ={0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
UUID service_uuid_perif(service_uuid_arry_perif);
//Characteristic UUID
static const uint8_t tx_uuid[]   = { 0x71, 0x3D, 0x00, 0x03, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E } ;
static const uint8_t rx_uuid[]   = { 0xf0 ,0xea ,0x41 ,0x86 ,0x7e ,0xbf ,0x49 ,0x9d ,0xbc ,0x81 ,0x7c ,0x6d ,0x83 ,0xa2 ,0xc6 ,0x07} ;
static const uint8_t service1_chars3_uuid[] = {0x71, 0x3D, 0, 4, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
// Used in advertisement
static const uint8_t uart_base_uuid_rev[]    = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};
//Characteristic Value
uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};
uint8_t chars3_value[TXRX_BUF_LEN] = {0x01, 0x02, 0x03};
bool central_connected = false;
//Characteristic Property Setting etc
GattCharacteristic  txCharacteristic (tx_uuid, txPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
GattCharacteristic  rxCharacteristic (rx_uuid, rxPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY| GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
GattCharacteristic characteristic3(service1_chars3_uuid, chars3_value, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *myChars[] = {&txCharacteristic, &rxCharacteristic, &characteristic3};
//Service Setting
GattService         myService(service_uuid_perif, myChars, sizeof(myChars) / sizeof(GattCharacteristic *));
static uint8_t tx_buf[TXRX_BUF_LEN];
Ticker ticker1s;
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

//-------------------------------------------------------------------------------------------------
void writeSensorDataToAnroid(int val){
  uint8_t sendData[1] = {val};
  ble.gattServer().write(client_handle,sendData,1,false);
}

void connectcallback( const Gap::ConnectionCallbackParams_t *params)
{
  //Serial.println("CONNECTED");
  if(params->role == Gap::CENTRAL){
     //Serial.println("CONNECTed by PERIFERAL DEVIDE");
     //discoveredServiceCallBack：Serviceを探す
     //discoveredCharacteristicCallBack:Characteristicを探してUUIDをと取得
     if(device_is_hrm){
       ble.gattClient().launchServiceDiscovery(params->handle, discoveredServiceCallBack, discoveredCharacteristicCallBack, service_uuid, chars_uuid1);
     }
     client_handle = params->handle;
     ble.startAdvertising();
  }
  else{
      //Serial.println("CONNET to CENTRAL DEVIDE");
      peripheral_handle = params->handle;
      central_connected = true;
      
  }
}
void disconnection(const Gap::DisconnectionCallbackParams_t *paramss)
{
  //リセット  
  //Serial.println("Disconnected, start to scanning");

  if(peripheral_handle == paramss->handle){
    //Serial.println("re Advertising");
    peripheral_handle = BLE_CONN_HANDLE_INVALID;
    central_connected = false;
    digitalWrite(BLINK_PIN, !digitalRead(BLINK_PIN));
    delay(5000);
    digitalWrite(BLINK_PIN, !digitalRead(BLINK_PIN));
    ble.startAdvertising();
  }
  else{
    //Serial.println("re scanning");
    client_handle = BLE_CONN_HANDLE_INVALID;
    device_is_hrm = 0;
    characteristic_is_fond = 0;
    descriptor_is_found = 0;
    digitalWrite(BLINK_PIN,LOW);
    ble.startScan(scanCallBack);   
  }

} 
//周辺危機を探索
void scanCallBack(const Gap::AdvertisementCallbackParams_t *params)
{
  uint8_t index;
  uint8_t len;
  uint8_t adv_name[31];
  if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {

    if(memcmp(PERIFERAL_NAME, adv_name, len) == 0x00) {
      //Serial.print("Complete name is : ");
      //Serial.println((const char*)adv_name);
      ble.stopScan();
      device_is_hrm = 1;
      ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
      //Serial.println("Got device, stop scan Mobile");
      digitalWrite(BLINK_PIN, HIGH);
      ble.startAdvertising();
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
  //Serial.println("\r\n----Characteristic Discovered");
 if(characteristic_is_fond == 1) {
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
//Coreへの指示
void moterControll(int num){
  if(central_connected){
    tx_buf[0] = num;
    ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  }
}

//Notify通知かindicateが来た時の処理
void hvxCallback(const GattHVXCallbackParams *params)
{ 
  digitalWrite(BLINK_PIN, !digitalRead(BLINK_PIN));
  //moter
  if(memcmp(params->data, "GO",2) == 0x00){ 
    //Serial.println("GO");
    tx_buf[0] = 0;
  }else if(memcmp(params->data, "RIGHT",5) == 0x00){
    //Serial.println("RIGHT");
    tx_buf[0] = 1;
  }else if(memcmp(params->data, "LEFT",4) == 0x00){
    //Serial.println("LEFT");
    tx_buf[0] = 2;
  }else if(memcmp(params->data, "BACK",4) == 0x00){
    //Serial.println("BACK");
    tx_buf[0] = 3;
  }else if(memcmp(params->data, "STOP",4) == 0x00){
    //Serial.println("STOP");
    tx_buf[0] = 4;
  }else if(memcmp(params->data, "MOVE_0",6) == 0x00){
    //Serial.println("MOVE_F");
    isMove = false; tx_buf[0] = 4; 
    ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  }
  else if(memcmp(params->data, "MOVE_1",6) == 0x00){
    //Serial.println("MOVE_T");
    isMove = true; 
  }
  //LED
  else if(memcmp(params->data, "LED_0",5) == 0x00){
    //Serial.println("LED_F");
    isLED = false; 
  }else if(memcmp(params->data, "LED_1",5) == 0x00){
    //Serial.println("LED_T");
    isLED = true; 
  }
  //frec
  else if(memcmp(params->data, "frec_0",6) == 0x00){
    //Serial.println("VIB_F");
    isFrec = false; 
  }else if(memcmp(params->data, "frec_1",6) == 0x00){
    //Serial.println("VIB_T");
    isFrec = true; 
  }
  //demo
  else if(memcmp(params->data, "demo_0",6) == 0x00){
    //Serial.println("DEMO_F");
    isDemoMode = false; 

  }else if(memcmp(params->data, "demo_1",6) == 0x00){
    //Serial.println("DEMO_T");
    isDemoMode = true; 
  }
  
  //動的なデータを送信
  if(isMove && central_connected ){
   // Serial.println("SEND");
     ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
     delay(1000);
     digitalWrite(BLINK_PIN, !digitalRead(BLINK_PIN));
     delay(1000);
     digitalWrite(BLINK_PIN, !digitalRead(BLINK_PIN));
  }
}
//demo func
void demo(){
  tx_buf[0] = 1;
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  delay(demo_go_time);
  tx_buf[0] = 1;
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  delay(demo_go_time);
  tx_buf[0] = 1;
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  delay(demo_go_time);
  tx_buf[0] = 1;
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  delay(demo_go_time);  

  if(!isDemoMode){
    isDemoMode = false; 
    tx_buf[0] = 4; 
    ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
  }
}

void updateCV(int val){
   //動的なデータを送信
  tx_buf[0] = val;
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
}
//Write
void onDataWriteCallback(const GattWriteCallbackParams *params)
{ 
  Serial.println("GattClient write");
}
//Read
void onDataReadCallback(const GattReadCallbackParams *params)
{ 
  Serial.println("GattClinet read");
}
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
 // Serial.println("\r\n----Characteristic Discovered");

  if(chars->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
    //Serial.println(chars->getUUID().getShortUUID(), HEX);
    if(chars->getUUID().getShortUUID() == 0x2A37) {
      //Serial.println("Found HRM characteristic ");
      characteristic_is_fond = 1;
      chars_hrm = *chars;
    }
  }
  else {
    uint8_t index;
    const uint8_t *uuid = chars->getUUID().getBaseUUID();
    for(index=0; index<16; index++) {
     // Serial.print(uuid[index], HEX);
    }
  }
}

void dataWriteCallback(const GattWriteCallbackParams *Handler){
  uint8_t index;
//Serial.println("aa ");
  uint8_t buf[TXRX_BUF_LEN];
  uint16_t bytesRead = 20;
 // Serial.println("Write Handle : ");
  // Check the attribute belong to which characteristic
  if (Handler->handle == rxCharacteristic.getValueAttribute().getHandle()) {
    // Read the value of characteristic
    ble.readCharacteristicValue(rxCharacteristic.getValueAttribute().getHandle(), buf, &bytesRead);
    for(index=0; index<bytesRead; index++) {
      //Serial.print(buf[index], HEX);
    }
    //Serial.println(" ");
  }
}

void setAdvertise(){
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  // Add short name to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,(const uint8_t *)"TXRX", 4);
  // Add complete 128bit_uuid to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid_rev));
  // Add complete device name to scan response data
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME,(const uint8_t *)My_NAME, sizeof(My_NAME) - 1);

}

void task_handle(void) {
  tx_buf[0] += 2;
  tx_buf[1] += 2;
  tx_buf[2] += 2;

  //BLEデータ送信
//  Serial.print(tx_buf[0]); Serial.print("\t");
//  Serial.print(tx_buf[1]); Serial.print("\t");
//  Serial.println(tx_buf[2]);

  
  //動的なデータを送信
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
}


//------------------------------------------------------------------------------------------------
void setupBle(){
  //ticker1s.attach(task_handle, 4);
  
  ble.init();

  //callback
  ble.onConnection(connectcallback);
  ble.onDisconnection(disconnection);
  //gatt client
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
  ble.gattClient().onHVX(hvxCallback);
  ble.gattClient().onDataWrite(onDataWriteCallback);
  ble.gattClient().onDataRead(onDataReadCallback);
  //gatt server
  ble.onDataWritten(dataWriteCallback);
  //adv_Data
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  ble.addService(myService);
  ble.setDeviceName((const uint8_t *)My_NAME);
  ble.setTxPower(4);
  setAdvertise();
  
  ble.setTxPower(4);
  //start adv
  ble.setAdvertisingInterval(160);
  ble.setAdvertisingTimeout(0);
  //if(BLE_CONNECT_MOBILE){
    ble.startAdvertising();
  //}
  //scan
  ble.setScanParams(1000, 200, 0, false);
  ble.startScan(scanCallBack);
  pinMode( 13, OUTPUT);
  pinMode(BLINK_PIN, OUTPUT);
  digitalWrite(BLINK_PIN,LOW);

  pinMode(PIN_BLE_MODE, INPUT);

  pinMode(D3,INPUT);
}
void loopBle(){
  ble.waitForEvent(); 

//  int b = analogRead(D3);
//  tx_buf[0] = b;  
//  //動的なデータを送信
//  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), tx_buf, 3);
   
  static bool preState;
  //bool state = analogRead(PIN_BLE_MODE);


  
//  if(state && preState){
//    BLE_CONNECT_MOBILE = !BLE_CONNECT_MOBILE;
//    setupBle();
//  }
//  preState = state;
}

void resetForMobileConnection(){
  
  ble.init();

  //callback
  ble.onConnection(connectcallback);
  ble.onDisconnection(disconnection);
  //gatt client
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
  ble.gattClient().onHVX(hvxCallback);
  ble.gattClient().onDataWrite(onDataWriteCallback);
  ble.gattClient().onDataRead(onDataReadCallback);
  //gatt server
  ble.onDataWritten(dataWriteCallback);
  //adv_Data
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  ble.addService(myService);
  ble.setDeviceName((const uint8_t *)My_NAME);
  ble.setTxPower(4);
  setAdvertise();
  
  ble.setTxPower(4);
  //start adv
  ble.setAdvertisingInterval(160);
  ble.setAdvertisingTimeout(0);

  //scan
  ble.setScanParams(1000, 200, 0, false);
  ble.startScan(scanCallBack);
  pinMode(BLINK_PIN, OUTPUT);
  digitalWrite(BLINK_PIN,LOW);
}
