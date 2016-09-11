//#ifndef __CONFIG_H__
//#define __CONFIG_H__
#ifndef __NAP1_CONFIG_H__
#define __NAP1_CONFIG_H__

#include "application.h"


// 1: debug printing, 0: no debug printing
#define NAP1_DEBUG 1

#if NAP1_DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__);
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__);
#else
    #define DEBUG_PRINT(...) do {} while (0)
    #define DEBUG_PRINTLN(...) do {} while (0)
#endif

/* Pin and sensor definitions */
#define DHT_PIN        D0        // Pin for internal DHT sensor.
#define DHT_TYPE       DHT22     // DHT 22 (AM2302)

/* Audio analysis parameters */
int MICROPHONE = A0; // 10;        // A0 on spark Core/Photon
int FFT_SIZE = 128;         //FFT Bucket Size (32,64,128,256 - higher means more frequency resolution)
int SAMPLEDELAY = 600;      //Delay for sampling in microseconds f = 1/t*10^6

/* Configuration variables */
#define IDEAL_READ_INTERVAL 5000 // may be adjusted upward to match sensors' min_delays

/* Output variables */
#define LOGFILE_NAME "nap1.csv"
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

/* MQTT Topics */
char* MQTTBASETOPIC = "xdb/feeds";
char* MQTTAUDIOMAGNTOPIC = "/audio_magnitudes";
char* MQTTAUDIOFREQTOPIC = "/audio_frequencies";
char* mqtttopics[] = {"/temperature", "/humidity"};


#endif
