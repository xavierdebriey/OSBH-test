/*
 * Nectar Technologies library
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"
#include "config.h"
#include "Adafruit_SHT31.h"


/* Constants */
#define ONE_HOUR_SECS (60*60)
#define ONE_MIN_MILLIS (60*1000)
#define ONE_HOUR_MILLIS (60*60*1000)
#define ONE_DAY_MILLIS (24*60*60*1000)
#define TIME_EPOCH_START 1970
#define GMT_MIN_OFFSET -12
#define GMT_MAX_OFFSET 13

/* TCA9548A I2C Multiplexer adress */
#define TCAADDR 0x70
#define I2C_BUSES_NBR 8


namespace NECTAR {

/* Utility functions */

bool init_wifi(uint16_t timeout_ms = 10000);

// 1) Activate cellular module
// 2) Activate cellulare connection
// 3) Activate particle cloud connection
bool initWholeConnection(uint16_t timeout_ms = 3*ONE_MIN_MILLIS);

// Activate cellular connection
bool celullarConnection(uint16_t timeout_ms = 10000);

// Activate particle cloud connection
bool particleConnection(uint16_t timeout_ms = 10000);

// Return value may not be reliable indicator of success/failure
// in cases where time has already been synced previously and is
// being re-synced
bool syncTime(uint16_t timeout_ms = 1000);

// Begin SHT31 sensors
//void beginSHT31Sensors(Adafruit_SHT31 sht31);
void beginSHT31Sensors(Adafruit_SHT31 sht31);

// Read SHT31 sensors datas + Copy them to a float array
//void readAllSHT31Sensorsdatas(float *temp_hum_float_array, Adafruit_SHT31 sht31);
void readAllSHT31SensorsDatas(float *temp_hum_float_array, Adafruit_SHT31 sht31);

// Initialize I2C buses from TCA9548A I2C Multiplexer
void tcaselect(uint8_t i2c_bus);

// Create a string with float array datas
String floatArrayToString(float *float_array, uint8_t array_size);

/*// Appends line to file on SD card. creates file if it doesn't exist.
bool writeToSD(SDClass& sd, const char *line, const char *filename);

// Clears buffer and returns false if the GMT offset is out of range
bool getTimeStamp(char *buffer, const int size, float gmt_offset = 0.);

// Appends a suffix to the buffer. assumes buffer contains a null-terminated
// string. may overwrite the end of that string if there isn't room in the
// buffer to append it at the end.
void appendSuffix(char *buffer, const int size, const char *suffix);*/

}


#endif
