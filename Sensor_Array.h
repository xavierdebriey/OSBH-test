#ifndef __SENSOR_ARRAY_H__
#define __SENSOR_ARRAY_H__

#include "DSX_U.h"
#include "DHT_U.h"
#include "Adafruit_Sensor.h"


namespace OSBH {

/* Sensor array wrapper */

class Sensor_Array
{
public:
    // Construct individual sensor objects, but don't yet initialize the array
    Sensor_Array(uint8_t one_wire_pin, uint8_t dht_pin1, uint8_t dht_pin2, uint8_t dht_type);

    // Initialize the array. Call this before calling any of the methods below.
    void begin();

    // Get sensor or event data from a senser in the array
    bool getEvent(uint8_t index, sensors_event_t* event);
    bool getSensor(uint8_t index, sensor_t* sensor);

    // Get sensor or event info from a sensor in the array, then place
    // a string representation of it into the buffer argument
    void getSensorString(uint8_t index, char* buffer, size_t len);
    void getEventString(uint8_t index, char* buffer, size_t len);

    // The number of sensors in the array
    uint8_t count() const { return _sensor_cnt; }

    // The longest min_delay, in milliseconds, for all the sensors in the array.
    uint32_t minDelay() const { return _min_delay; }

private:
    Sensor_Array(const Sensor_Array&) = delete;
    Sensor_Array& operator=(const Sensor_Array&) = delete;

    // dht objects, each representing one temp and one humidity sensor
    static const uint8_t DHT_CNT = 2;
    DHT_Unified _dht[DHT_CNT];

    // this object can represent multiple DSX* sensors on the same onewire bus
    DSX_Unified _dsx;

    // array of Adafruit_sensor* pointers
    Adafruit_Sensor* _sensors[DSX_Unified::MAX_SENSORS + DHT_CNT*2];
    uint8_t _sensor_cnt;
    bool isValidIndex(uint8_t index) const; // make sure index is in-bounds and non-null 

    void setMinDelay(); // iterates sensor array and caches main delay
    uint32_t _min_delay; // cached min delay in milliseconds
};

}


#endif