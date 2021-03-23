#include <nRF5x_BLE_API.h>

BLE bleMobile;

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
