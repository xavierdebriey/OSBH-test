#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "application.h"


// 1: debug printing, 0: no debug printing
#define NECTAR_DEBUG 1

#if NECTAR_DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__);
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__);
#else
    #define DEBUG_PRINT(...) do {} while (0)
    #define DEBUG_PRINTLN(...) do {} while (0)
#endif

/* Information about beehives */
#define SHT31_NBR 5 // One SHT31 sensor by beehive + 1 on the board
#define SENSORS_NBR (2*SHT31_NBR) // Each SHT31 sensor includes 1 temperature sensor and 1 humidity sensor

/* Information about I2C buses */
//uint8_t i2cBusesArray[] = {0, 1, 2, 4, 7};

/* Variables configuration (version: if sending time are different than reading time) */
/* #define READ_TIME_INTERVAL (15*ONE_MIN_MILLIS); // Time interval between each sensors datas reading
#define CELLULAR_TIME_INTERVAL (ONE_HOUR_MILLIS); // Time interval between each celullar sensors datas sending
#define datas_READING_NBR = CELLULAR_TIME_INTERVAL / READ_TIME_INTERVAL;
#define TIMEOUT_CELLULAR = (5*ONE_MIN_MILLIS);
#define WAKE_UP_PIN = D3; // Pin to manually wake up device from sleep mode */

/* Variables configuration (version: if sending time is the same than reading time) */
#define READ_SEND_TIME_INTERVAL (30*1000) // (15*ONE_MIN_MILLIS) // Time interval between each sensors datas reading
#define WAKE_UP_PIN D3 // Pin to manually wake up device from sleep mode

/* datasbase Connection */
/*#define MQTTSERVER "io.adafruit.com"
#define MQTTSERVERPORT 1883
#define MQTTID "sparkclient"
#define MQTTUSER "xdb"
#define MQTTPSW "d1b95cd247244be38ec91cf95bf17edc"*/

/* MQTT Topics */
/*char* MQTTBASETOPIC = "xdb/feeds";
char* MQTTAUDIOMAGNTOPIC = "/audio_magnitudes";
char* MQTTAUDIOFREQTOPIC = "/audio_frequencies";
char* MQTTWEIGHTTOPIC = "/weight";
char* mqtttopics[] = {"/temperature", "/humidity"};*/


#endif
