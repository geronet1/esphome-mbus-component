#include "mbus_datarecord.h"

namespace esphome {
namespace mbus {
	
static const char *const TAG = "mbus.datarecord";

const char* MbusDIFDatatypeToStr(enum MbusDIFDatatype datatype){
  switch (datatype){
  case MBUS_NO_DATA: return "MBUS_NO_DATA"; break;
  case MBUS_INT_8BIT: return "MBUS_INT_8BIT"; break;
  case MBUS_INT_16BIT: return "MBUS_INT_16BIT"; break;
  case MBUS_INT_24BIT: return "MBUS_INT_24BIT"; break;
  case MBUS_INT_32BIT: return "MBUS_INT_32BIT"; break;
  case MBUS_INT_48BIT: return "MBUS_INT_48BIT"; break;
  case MBUS_INT_64BIT: return "MBUS_INT_64BIT"; break;
  case MBUS_REAL: return "MBUS_REAL"; break;
  case MBUS_SELECTION: return "MBUS_SELECTION"; break;
  case MBUS_BCD2: return "MBUS_BCD2"; break;
  case MBUS_BCD4: return "MBUS_BCD4"; break;
  case MBUS_BCD6: return "MBUS_BCD6"; break;
  case MBUS_BCD8: return "MBUS_BCD8"; break;
  case MBUS_VARIABLE_LEN: return "MBUS_VARIABLE_LEN"; break;
  case MBUS_BCD12: return "MBUS_BCD12"; break;
  case MBUS_SPECIAL: return "MBUS_SPECIAL"; break;
  default: return "Unknown"; break;
  }
}

const char* MbusDIFFunctionToStr(enum MbusDIFFunction function){
  switch (function){
  case MBUS_INSTANT_VALUE: return "MBUS_INSTANT_VALUE"; break;
  case MBUS_MAXIMUM_VALUE: return "MBUS_MAXIMUM_VALUE"; break;
  case MBUS_MINIMUM_VALUE: return "MBUS_MINIMUM_VALUE"; break;
  case MBUS_ERROR_VALUE: return "MBUS_ERROR_VALUE"; break;
  default: return "Unknown"; break;
 }
}

/* uint8_t MbusSensor::MbusParseDataRecord(uint8_t* tg):
 * 
 * Parse a variable-length Data Record and render it as a float if possible.
 * Match parsed attributes to those specified by the sensor instance.
 * 
 * tg: pointer to the Data Record's DIF
 * 
 * Returns 0 on error, length of parsed data otherwise.
 * 
 * On match, sets this->mbus_data_record_parse_result_ and
 * increments this->mbus_data_record_parse_status_.
 * 
 * On error, logs error message.
 * */

uint8_t MbusSensor::MbusParseDataRecord(uint8_t* tg){
	const char* sensorname=this->get_name().c_str();
	
	uint8_t pos = 0;
	
	uint64_t storage = 0;
	uint32_t tariff = 0;
	uint16_t subunit = 0;
	enum MbusDIFFunction function;
	enum MbusDIFDatatype datatype;
	
	//DIF
	bool extension_flag = false;
	if(tg[pos] & MBUS_DIF_EXTENSION_MASK) extension_flag = true;
	function = (MbusDIFFunction) (tg[pos] & MBUS_DIF_FUNCTION_MASK);
	datatype = (MbusDIFDatatype) (tg[pos] & MBUS_DIF_DATATYPE_MASK);
	if(tg[pos] & MBUS_DIF_STORAGE_MASK) storage = 1;
	pos++;
	
	//DIFE
	uint8_t dife_count = 0;
	while(extension_flag){
		dife_count++;
		if(dife_count > MBUS_DIFE_MAX){
			ESP_LOGE(TAG, " %s: Too many DIFE fields", sensorname);
			return 0;
		}
		extension_flag = tg[pos] & MBUS_DIFE_EXTENSION_MASK;
		tariff += ( ( tg[pos] & MBUS_DIFE_TARIFF_MASK ) >> 4 ) << ( (dife_count-1) * 2 );
		subunit += ( ( tg[pos] & MBUS_DIFE_SUBUNIT_MASK ) >> 6 ) << (dife_count-1);
		storage += ( tg[pos] & MBUS_DIFE_STORAGE_MASK ) << ( ( (dife_count-1) * 4) + 1 );
		pos++;
	}

	//VIF + VIFE
	
	/* Due to the complexities involved with multiple coexisting VIF+VIFE schemes and
	 * their possible future extensions, the approach taken is to represent VIF
	 * and the first 7 VIFEs as a single 64-bit integer and ignore the rest. */
	
	uint64_t vif_vife = 0;
	extension_flag = true;
	uint8_t vife_count = 0;
	
	while(extension_flag){
		vife_count++;
		if(vife_count > MBUS_VIFE_MAX){
			ESP_LOGW(TAG, " %s: Too many VIFE fields.", sensorname);
			return 0;
		}
		if(vife_count > 8){
			ESP_LOGW(TAG, " %s: Too many VIFE fields, ignoring.", sensorname);
		} else {
			vif_vife = vif_vife << 8;
			vif_vife |= (uint64_t) tg[pos];
		}
		extension_flag = tg[pos] & MBUS_VIFE_EXTENSION_MASK;
		pos++;
	}
	
	//Datatype-dependent value parsing
	
	float result = 0;
	uint64_t resultint = 0;	//intermediate, so no need to do excessive float arithmetric
	uint64_t placevalue = 1;

	switch (datatype){
		
	default: 
		ESP_LOGE(TAG, " %s: Unknown datatype %d", sensorname, datatype);
		return 0;

	case MBUS_NO_DATA:
	case MBUS_SELECTION:
		break;
	
	case MBUS_SPECIAL:
		ESP_LOGE(TAG, " %s: Unexpected SPECIAL FUNCTION datatype %d", sensorname, datatype);
		return 0;
	
	case MBUS_VARIABLE_LEN:
		ESP_LOGW(TAG, " %s: VARIABLE LENGTH datatype len = %d, decoding not yet supported.", sensorname, tg[pos]);
		pos++; pos+=tg[pos-1];
		break;
		
	case MBUS_REAL: 
		pos+=4;
		ESP_LOGW(TAG, " %s: REAL datatype, decoding not yet supported.", sensorname);
		break;
	
	case MBUS_INT_64BIT:
		resultint = placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
	case MBUS_INT_48BIT:
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
	case MBUS_INT_32BIT:
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
	case MBUS_INT_24BIT:
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
	case MBUS_INT_16BIT:
		resultint |= placevalue * tg[pos];
		pos++;
		placevalue <<= 8;
		
	case MBUS_INT_8BIT:
		resultint |= placevalue * tg[pos];
		pos++;
		result = (float) resultint;
		break;
	
	case MBUS_BCD12:
		resultint = placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		placevalue *= 100;
		
		resultint += placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		placevalue *= 100;
		
	case MBUS_BCD8:
		resultint += placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		placevalue *= 100;
	
	case MBUS_BCD6:
		resultint += placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		placevalue *= 100;
		
	case MBUS_BCD4:
		resultint += placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		placevalue *= 100;
		
	case MBUS_BCD2:
		resultint += placevalue * ( (tg[pos]&0x0f)+(10*((tg[pos]>>4)&0x0f)) );
		pos++;
		result = (float) resultint;
		break;
		
	}

	//Comparison of attributes and value update
	
	ESP_LOGD(TAG, " %s: function: %s, datatype: %s, storage: %lld, tariff: %d, subunit: %d, VIF(E): 0x%llX, value: %lld", sensorname,
	MbusDIFFunctionToStr(function), MbusDIFDatatypeToStr(datatype),  storage, tariff, subunit, vif_vife, resultint);
	
	if( (storage == this->mbus_storage_requested_) &&
		(function == this->mbus_function_requested_) &&
		(tariff == this->mbus_tariff_requested_) &&
		(subunit == this->mbus_subunit_requested_) &&
		(vif_vife == this->mbus_vif_vife_requested_)
	) {
		ESP_LOGD(TAG, " %s: Match", sensorname);
		this->mbus_data_record_parse_status_++;
		this->mbus_data_record_parse_result_ = result;
	}
	
	return pos;
}
	
}  // namespace mbus
}  // namespace esphome
