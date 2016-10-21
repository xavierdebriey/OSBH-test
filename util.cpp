#include "config.h"
#include "util.h"
#include "application.h"
/*#include "Wire.h"
extern "C" {
    #include "utility/twi.h"  // from Wire library, so we can do bus scanning
}*/

//Adafruit_SHT31 sht31;
uint8_t i2cBusesArray[] = {0, 1, 2, 4, 7};


bool NECTAR::init_wifi(uint16_t timeout_ms)
{
    WiFi.on();
    //WiFi.connect();
    Particle.connect();
    uint32_t timeout = millis() + timeout_ms;
    while (!Particle.connected() && millis() < timeout) {
        Particle.process();
        delay(100);
    }
    return Particle.connected();
    //return WiFi.ready();
}

bool NECTAR::initWholeConnection(uint16_t timeout_ms) {
    /*Cellular.on();

    uint8_t connection_status = 1;
    uint32_t now = millis();
    uint32_t timeout = now + timeout_ms;
    while (!(connection_status == 0) && now < timeout) {
        if (connection_status == 1) {
            if(celullarConnection()) {
                connection_status == 2;
            }
        }
        if (connection_status == 2) {
            if(particleConnection()) {
                connection_status == 0;
            }
        }
        now = millis();
    }

    if (connection_status == 0) {
        return true;
    } else {
        Serial.println("time for whole connection exceeded");
        return false;
    }*/
}

bool NECTAR::celullarConnection(uint16_t timeout_ms) {
    /*Cellular.connect();

    if (Cellular.connecting()) {
        uint32_t now = millis();
        uint32_t timeout = now + timeout_ms;
        while (!Cellular.ready() && now < timeout) {
            now = millis();
        }

        if (Cellular.ready()) {
            Serial.println("cellular connection is ready");
            return true;
        } else {
            Serial.println("time for cellular connection exceeded");
            return false;
        }
    } else {
        Serial.println("cellular connection is ready");
        return true;
    }*/
}

bool NECTAR::particleConnection(uint16_t timeout_ms) {
    Particle.connect();

    uint32_t now = millis();
    uint32_t timeout = now + timeout_ms;
    while (!Particle.connected() && now < timeout) {
        Particle.process();
        delay(100);
        now = millis();
    }

    if (Particle.connected()) {
        Serial.println("Particle Cloud connection is ready");
        return true;
    } else {
        Serial.println("time for Particle Cloud connection exceeded");
        return false;
    }
}

bool NECTAR::syncTime(uint16_t timeout_ms) {
    if (Particle.connected()) {
        Particle.syncTime();

        // syncTime is non-blocking, so wait for a valid time to come back
        // from the server before proceeding
        uint32_t now = millis();
        uint32_t timeout = now + timeout_ms;
        while (Time.year() <= TIME_EPOCH_START && now < timeout) {
            Particle.process();
            delay(100);
            now = millis();
        }
        return now < timeout;
    }
    return false;
}

void NECTAR::beginSHT31Sensors(Adafruit_SHT31 sht31) {

    for (uint8_t i = 0; i < SHT31_NBR; i++) {
        tcaselect(i2cBusesArray[i]);

        if (i2cBusesArray[i] == 0) {
            if (!sht31.begin(0x44)) {
                Serial.println("couldn't find SHT31 sensor from board");
                while (1) delay(1);
            }
        }

        if (i2cBusesArray[i] == 1) {
            if (!sht31.begin(0x44)) {
                Serial.println("couldn't find SHT31 sensor from beehive 1");
                while (1) delay(1);
            }
        }

        if (i2cBusesArray[i] == 2) {
            if (!sht31.begin(0x44)) {
                Serial.println("couldn't find SHT31 sensor from beehive 2");
                while (1) delay(1);
            }
        }

        if (i2cBusesArray[i] == 4) {
            if (!sht31.begin(0x44)) {
                Serial.println("couldn't find SHT31 sensor from beehive 3");
                while (1) delay(1);
            }
        }

        if (i2cBusesArray[i] == 7) {
            if (!sht31.begin(0x44)) {
                Serial.println("couldn't find SHT31 sensor from beehive 4");
                while (1) delay(1);
            }
        }
    }
}

void NECTAR::readAllSHT31SensorsDatas(float *temp_hum_float_array, Adafruit_SHT31 sht31) {
    /*uint8_t j = 0;
    for(uint8_t i = 0; i < SENSORS_NBR; i = i + 2) {
        tcaselect(i2cBusesArray[j]);
        *(temp_hum_float_array + i) = sht31.readTemperature();
        Serial.println(*(temp_hum_float_array + i));
        delay(10000);
        *(temp_hum_float_array + i + 1) = sht31.readHumidity();
        Serial.println(*(temp_hum_float_array + i + 1));
        delay(10000);
        j++;
    }*/
}

void NECTAR::tcaselect(uint8_t i2c_bus) {
  if (i2c_bus > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i2c_bus);
  Wire.endTransmission();
}

String NECTAR::floatArrayToString(float *float_array, uint8_t array_size) {
    String str = "";
    for(uint8_t i = 0; i < array_size; i++) {
        str.concat(String(*(float_array + i)));
        if (i < array_size - 1) {
            str.concat(".");
        } else {
            str.concat(";");
        }
    }
    return str;
}
/*
bool NECTAR::getTimeStamp(char* buffer, const int size, float gmt_offset) {
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

void NECTAR::appendSuffix(char* buffer, const int size, const char* suffix) {
    int suffix_len = strlen(suffix) + 1; // include space for null terminator

    // if the buffer is too small to contain the suffix, bail
    if (suffix_len > size) return;

    // copy the suffix as close to the end of the string as possible
    int suffix_start_pos = min(strlen(buffer), size - suffix_len);
    strncpy(buffer + suffix_start_pos, suffix, suffix_len);
}*/
