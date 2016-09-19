/*
 * Open Source Beehives library
 */

#ifndef __NAP1_UTIL_H__
#define __NAP1_UTIL_H__

#include "stdint.h"


/* Constants */
#define ONE_HOUR_SECS (60*60)
#define ONE_DAY_MILLIS (24*60*60*1000)
#define TIME_EPOCH_START 1970
#define GMT_MIN_OFFSET -12
#define GMT_MAX_OFFSET 13

/* Forward declarations */
class SDClass;

//namespace OSBH {
namespace NAP1 {

/* Utility functions */

bool init_wifi(uint16_t timeout_ms = 10000);

// return value may not be reliable indicator of success/failure
// in cases where time has already been synced previously and is
// being re-synced
bool sync_time(uint16_t timeout_ms = 1000);

// appends line to file on SD card. creates file if it doesn't exist.
bool write_to_sd(SDClass& sd, const char* line, const char* filename);

// clears buffer and returns false if the GMT offset is out of range
bool get_timestamp(char* buffer, const int size, float gmt_offset = 0.);

// appends a suffix to the buffer. assumes buffer contains a null-terminated
// string. may overwrite the end of that string if there isn't room in the
// buffer to append it at the end.
void append_suffix(char* buffer, const int size, const char* suffix);

}


#endif
