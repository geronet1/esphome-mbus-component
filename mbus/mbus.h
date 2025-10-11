#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mbus {
	
static const uint8_t mbus_max_retries_ = 3;

static const unsigned char* mbus_reset_frame_ = (const unsigned char*)"\x10\x40\xFD\x3D\x16";
static const size_t mbus_reset_frame_len_ = 5;

static const unsigned char* mbus_request_frame_ = (const unsigned char*)"\x10\x5B\xFD\x58\x16";
static const size_t mbus_request_frame_len_ = 5;

static const unsigned char* mbus_select_frame_raw_ = 
	(const unsigned char*)"\x68\x0B\x0B\x68\x73\xFD\x52\x00\x00\x00\x00\x00\x00\x00\x00\x00\x16";
static const size_t mbus_select_frame_len_ = 17;

static const uint8_t mbus_ack_ = 0xE5;
static const uint8_t mbus_long_frame_ = 0x68;


enum MbusState {
	MBUS_STATE_IDLE,
	MBUS_STATE_AWAIT_LOCK,
	MBUS_STATE_BUS_RESET_PRE,
	MBUS_STATE_BUS_RESET,
	MBUS_STATE_BUS_RESET_2,
	MBUS_STATE_AWAIT_SELSCT_SA,
	MBUS_STATE_AWAIT_SELSCT_SA_2,
	MBUS_STATE_AWAIT_HEADER,
	MBUS_STATE_AWAIT_DATA,
	MBUS_STATE_RETRY_WAIT,
	MBUS_STATE_RETRY,
}; 
	
class Mbus : public uart::UARTDevice, public PollingComponent {
 public:

  void setup() override;

  void loop() override;

  void dump_config() override;
  
  void update() override;

  float get_setup_priority() const override;
  
  void set_secondary_address(uint64_t secondary_address) { this->secondary_address = secondary_address; }
  
  uint64_t secondary_address;
  
  uint8_t telegram[270];
  uint8_t telegram_count;

 protected:
 
 
 uint32_t mbus_timeout_short_;
 uint32_t mbus_timeout_long_;
 uint32_t mbus_timer_;
 enum MbusState mbus_state_;
 uint8_t mbus_retry_count_;
 bool mbus_update_due_;
 uint16_t mbus_telegram_len_;
 uint8_t mbus_select_frame_[mbus_select_frame_len_];
  
  uint8_t mbus_checksum(const uint8_t* data);

  
};

	
}  // namespace mbus
}  // namespace esphome
