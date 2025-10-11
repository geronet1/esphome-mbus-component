#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/mbus/mbus.h"

namespace esphome {
namespace mbus {
	
static const uint8_t MBUS_CONTROL_RSP_UD = 0X08;
static const uint8_t MBUS_CI_RESP_VARIABLE = 0x72;
static const uint8_t MBUS_STATUS_APP_ERROR = 0x03;
static const uint8_t MBUS_STATUS_LOW_POWER = 0x04;
static const uint8_t MBUS_STATUS_PERMANENT_ERROR = 0x08;
static const uint8_t MBUS_STATUS_TEMPORARY_ERROR = 0x10;
static const uint8_t MBUS_DIF_MANUFACTURER_SPECIFIC = 0x0F;
static const uint8_t MBUS_DIF_MANUFACTURER_SPECIFIC_MULTIFRAME = 0x1F;
static const uint8_t MBUS_DIF_FILLER = 0x2F;

enum MbusDIFDatatype : uint8_t {
  MBUS_NO_DATA = 0X00,
  MBUS_INT_8BIT = 0X01,
  MBUS_INT_16BIT = 0X02,
  MBUS_INT_24BIT = 0X03,
  MBUS_INT_32BIT = 0X04,
  MBUS_INT_48BIT = 0X06,
  MBUS_INT_64BIT = 0X07,
  MBUS_REAL = 0X05,
  MBUS_SELECTION = 0X08,
  MBUS_BCD2 = 0X09,
  MBUS_BCD4 = 0X0A,
  MBUS_BCD6 = 0X0B,
  MBUS_BCD8 = 0X0C,
  MBUS_VARIABLE_LEN = 0X0D,
  MBUS_BCD12 = 0X0E,
  MBUS_SPECIAL = 0X0F,
};

enum MbusDIFFunction : uint8_t {
  MBUS_INSTANT_VALUE = 0X00,
  MBUS_MAXIMUM_VALUE = 0X10,
  MBUS_MINIMUM_VALUE = 0X20,
  MBUS_ERROR_VALUE = 0X30,
};

const char* MbusDIFDatatypeToStr(enum MbusDIFDatatype datatype);
const char* MbusDIFFunctionToStr(enum MbusDIFFunction function);
	
class MbusSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(Mbus* parent) { parent_ = parent; }
  void set_mbus_storage(uint64_t mbus_storage) { mbus_storage_requested_ = mbus_storage; }
  void set_mbus_function(enum MbusDIFFunction mbus_function) { mbus_function_requested_ = mbus_function; }
  void set_mbus_tariff(uint32_t mbus_tariff) { mbus_tariff_requested_ = mbus_tariff; }
  void set_mbus_subunit(uint32_t mbus_subunit) { mbus_subunit_requested_ = mbus_subunit; }
  void set_mbus_vife(uint64_t mbus_vife) { mbus_vif_vife_requested_ = mbus_vife; }
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

 protected:
  Mbus* parent_;
//  std::string topic_;
//  uint8_t qos_{0};
  uint8_t telegram_seen;
  uint8_t mbus_data_record_parse_status_;
  float mbus_data_record_parse_result_;
  uint8_t MbusParseDataRecord(uint8_t* tg);

  uint64_t mbus_storage_requested_;
  enum MbusDIFFunction mbus_function_requested_;
  uint32_t mbus_tariff_requested_;
  uint32_t mbus_subunit_requested_;
  uint64_t mbus_vif_vife_requested_;
  
};

}  // namespace mbus
}  // namespace esphome
