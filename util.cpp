#include "util.h"
#include "application.h"
#include "sd-card-library.h"


bool OSBH::init_wifi(uint16_t timeout_ms)
{
    WiFi.on();
    //WiFi.connect();
    Spark.connect();
    uint32_t timeout = millis() + timeout_ms;
    while (!Spark.connected() && millis() < timeout) {
        Spark.process();
        delay(100);
    } 
    return Spark.connected();
    //return WiFi.ready();
}


bool OSBH::sync_time(uint16_t timeout_ms)
{
    if (Spark.connected()) {
        Spark.syncTime();

        // syncTime is non-blocking, so wait for a valid time to come back
        // from the server before proceeding
        uint32_t timeout = millis() + timeout_ms;
        while (Time.year() <= TIME_EPOCH_START && millis() < timeout) {
            Spark.process();
            delay(100);
        }
        return millis() < timeout;
    }
    return false;
}

bool OSBH::write_to_sd(SDClass& sd, const char* line, const char* filename) 
{
    if (File myFile = sd.open(filename, FILE_WRITE)) {
        bool success = (myFile.print(line) == strlen(line));
        myFile.close();
        return success;
    }
    return false;
}

bool OSBH::get_timestamp(char* buffer, const int size, float gmt_offset)
{
    // return empty string if the GMT offset doesn't make sense
    if (gmt_offset < GMT_MIN_OFFSET || gmt_offset > GMT_MAX_OFFSET) {
        buffer[0] = '\0';
        return false;
    }

    // get pointer to unix time string
    time_t t = Time.now() + (time_t)(gmt_offset * ONE_HOUR_SECS);
    struct tm *calendar_time = localtime(&t);
    char* time_str = asctime(calendar_time);

    // copy into buffer
    // (we need to make a copy because the pointed-to string will change on
    // subsequent calls to asctime)
    int timestamp_len = min(size, strlen(time_str));
    strncpy(buffer, time_str, timestamp_len);

    // drop trailing line break from time_str
    buffer[timestamp_len - 1] = '\0';

    return true;
}

void OSBH::append_suffix(char* buffer, const int size, const char* suffix)
{
    int suffix_len = strlen(suffix) + 1; // include space for null terminator

    // if the buffer is too small to contain the suffix, bail
    if (suffix_len > size) return;

    // copy the suffix as close to the end of the string as possible
    int suffix_start_pos = min(strlen(buffer), size - suffix_len);
    strncpy(buffer + suffix_start_pos, suffix, suffix_len);
}
