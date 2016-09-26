/*
 *  Adafruit_DSX_U
 *
 *  Adaption of Adafruit Unified driver to Spark and DS OneWire sensors
 *
 *  DSX_Sensor code based off PietteTech_DSX_U class, which is
 *  copyright (c) 2014 Scott Piette (scott.piette@gmail.com)
 *
 *  Developed for the Open Source Beehives Project
 *       (http://www.opensourcebeehives.net)
 *
 *  This adaptation is released under the following license:
 *	GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 */

#ifndef __DSX_H__
#define __DSX_H__

#include "Adafruit_Sensor.h"
#include "OneWire.h"


// Adafruit_Sensor interface to a specific DSX* sensor on the
// One Wire bus.
class DSX_Sensor : public Adafruit_Sensor
{
public:
    DSX_Sensor(uint8_t addr[8], OneWire *one, int32_t sensorId, char *sensorName);

    bool  getEvent(sensors_event_t*) override;
    void  getSensor(sensor_t*) override;

private:
    float getTemperature();
    void  setMinDelay(sensor_t* sensor);
    void  setDefaultName();

    char     _name[12];                        /**< sensor name */
    uint8_t  _addr[8];
    OneWire  *_one;
    int32_t  _sensorID;
};


// Wrapper class that manages an array of DS* sensors on a single OneWire bus.
class DSX_Unified
{
public:
    static const uint8_t MAX_SENSORS = 4;

    explicit DSX_Unified(uint8_t one_wire_pin);
    virtual ~DSX_Unified();

    // Query one-wire bus and initialize child sensors before first use.
    // Once the object is prepared, re-preparing it will have no effect,
    //svoid prepare(uint8_t starting_sensor_id);
    void prepare(uint8_t starting_sensor_id);

    // The child sensors detected on the one-wire bus when the object
    // was prepared. The first children_cnt pointers in the array are
    // guaranteed non-null.
    // All sensors in the array have the same lifetime as this parent
    // object, and should not be accessed after the parent is destroyed.
    Adafruit_Sensor** sensors() { return _sensors; }
    uint8_t count() { return _sensor_cnt; }

private:
    DSX_Unified(const DSX_Unified&) = delete;
    DSX_Unified& operator=(const DSX_Unified&) = delete;

    OneWire _one;

    Adafruit_Sensor* _sensors[MAX_SENSORS];
    uint8_t _sensor_cnt;
};

#endif
