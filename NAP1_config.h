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
#define FLEXIFORCE_PIN      A0
#define ONE_WIRE_WATER_PROOF_PIN  D0
#define ONE_WIRE_OTHER_PIN  D2
#define DHT11_PIN       D1         // Pin for internal DHT sensor.
#define DHT22_PIN       D3         // Pin for external DHT sensor.
#define DHT11_TYPE      DHT11     // DHT 22 (AM2302)
#define DHT22_TYPE      DHT22     // DHT 22 (AM2302)
#define SD_CD_PIN_IN   A6
#define SD_CD_PIN_OUT  A7

/* Audio analysis parameters */
int MICROPHONE = A1; // 10;        // A0 on spark Core/Photon
int FFT_SIZE = 128;         //FFT Bucket Size (32,64,128,256 - higher means more frequency resolution)
int SAMP_NUM = FFT_SIZE;
int SAMPLEDELAY = 600;      //Delay for sampling in microseconds f = 1/t*10^6

/* Flexiforce A201 100 lbs pressure sensor pin definition */
//int flexiforce = A0;

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
char* MQTTWEIGHTTOPIC = "/weight";
char* mqtttopics[] = {"/temperature", "/humidity"};

/* Particle Events */
char* PARTICLECATEGEVENT = "event_category";
char* PARTICLEAUDIOSIGNEVENT = "audio_signal";
char* PARTICLEAUDIOMAGNEVENT = "audio_magnitude";
char* PARTICLEAUDIOFREQEVENT = "audio_frequency";
char* PARTICLEWEIGHTEVENT = "weight";
char* particleevents[] = {"board_temperature", "board_humidity", "outBH_temperature", "topBH_temperature"};
//char* particleevents[] = {"DHT11_temperature", "DHT11_humidity", "DHT22_temperature", "DHT22_humidity", "WP_temperature", "NWP_temperature"};


#endif
