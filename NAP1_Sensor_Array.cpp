#include "NAP1_Sensor_Array.h"

//using namespace NAP1;
using namespace NAP1;

NAP1_Sensor_Array::NAP1_Sensor_Array(uint8_t dht_pin, uint8_t dht_type)
    : _dht{
        DHT_Unified(dht_pin, dht_type, 6, 1, 2)
    },
    _sensors{nullptr},
    _sensor_cnt(0),
    _min_delay(0)
    {}


void NAP1_Sensor_Array::begin()
{
    _sensor_cnt = 0;

    // initialize DHT sensors and copy their child sensor pointers into array
    for (uint8_t i = 0; i < DHT_CNT; ++i) {
        DHT_Unified& dht = _dht[i];
        dht.begin();
        _sensors[_sensor_cnt++] = dht.temperature();
        _sensors[_sensor_cnt++] = dht.humidity();
    }

    setMinDelay();
}

bool NAP1_Sensor_Array::isValidIndex(uint8_t index) const
{
    return index < _sensor_cnt && _sensors[index];
}

bool NAP1_Sensor_Array::getEvent(uint8_t index, sensors_event_t* event)
{
    if (!isValidIndex(index)) return false;
    return _sensors[index]->getEvent(event);
}

bool NAP1_Sensor_Array::getSensor(uint8_t index, sensor_t* sensor)
{
    if (!isValidIndex(index)) return false;
    _sensors[index]->getSensor(sensor);
    return true;
}

void NAP1_Sensor_Array::getSensorString(uint8_t index, char* buffer, size_t len)
{
    sensor_t sensor;
    if (!getSensor(index, &sensor))
        return;

    snprintf(buffer, len, "%s_%li", sensor.name, sensor.sensor_id);
}

void NAP1_Sensor_Array::getEventString(uint8_t index, char* buffer, size_t len)
{
    sensors_event_t event;
    if (!getEvent(index, &event))
        return;

    snprintf(buffer, len, "%f", event.data[0]);
}

void NAP1_Sensor_Array::setMinDelay()
{
    sensor_t sensor;
    for (uint8_t i = 0; i < _sensor_cnt; ++i) {
        Adafruit_Sensor* s = _sensors[i];
        if (!s) continue;

        s->getSensor(&sensor);
        int32_t d = sensor.min_delay / 1000; // usecs -> msecs
        if (d > 0 && static_cast<uint32_t>(d) > _min_delay) { // assume negative delays are non-meaningful
            _min_delay = d;
        }
    }
}
