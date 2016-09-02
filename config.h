#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "application.h"


// 1: debug printing, 0: no debug printing
#define OSBH_DEBUG 1

#if OSBH_DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__);
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__);
#else
    #define DEBUG_PRINT(...) do {} while (0)
    #define DEBUG_PRINTLN(...) do {} while (0)
#endif

/* Pin and sensor definitions */
#define ONE_WIRE_PIN   3
#define DHT_PIN1       D0 // 4         // Pin for internal DHT sensor.
#define DHT_PIN2       5         // Pin for external DHT sensor.
#define DHT_TYPE       DHT22     // DHT 22 (AM2302)
#define SD_CD_PIN_IN   A6
#define SD_CD_PIN_OUT  A7

/* Audio analysis parameters */
int MICROPHONE = A0; // 10;        // A0 on spark Core/Photon
int FFT_SIZE = 128;         //FFT Bucket Size (32,64,128,256 - higher means more frequency resolution)
int SAMPLEDELAY = 600;      //Delay for sampling in microseconds f = 1/t*10^6

/* Configuration variables */
#define IDEAL_READ_INTERVAL 5000 // may be adjusted upward to match sensors' min_delays

/* Output variables */
#define LOGFILE_NAME "osbh.csv"
#define LOGFILE_NAME_AUDIO "audio.csv"
#define DELIMITER ","
#define LINE_END "\n"
#define GMT_OFFSET 1

/* Database Connection */
#define MQTTSERVER "io.adafruit.com"
#define MQTTSERVERPORT 1883
#define MQTTID "sparkclient"
#define MQTTUSER "xdb"
#define MQTTPSW "d1b95cd247244be38ec91cf95bf17edc"
//#define MQTTUSER "ronjac"
//#define MQTTPSW "92227a87656a7029f7758fc1283abe79320a8673"

/* MQTT Topics */
//char* MQTTBASETOPIC = "ronjac/feeds";
char* MQTTBASETOPIC = "xdb/feeds";
char* MQTTAUDIOTOPIC = "/audio";
char* mqtttopics[] = {"/DHT1temp", "/DHT1hum", "/DHT2temp", "/DHT2hum", "/Temp1", "/Temp2"};


#endif
