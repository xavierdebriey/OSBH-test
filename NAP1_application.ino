#include "NAP1_config.h"
#include "NAP1_util.h"
#include "NAP1_Sensor_Array.h"
#include "sd-card-library.h"
#include "NAP1_audio.h"
#include "MQTT.h"

using namespace NAP1;

// disable wifi on boot
SYSTEM_MODE(SEMI_AUTOMATIC);

// global sensor array
NAP1_Sensor_Array sensors(ONE_WIRE_WATER_PROOF_PIN, ONE_WIRE_OTHER_PIN, DHT11_PIN, DHT22_PIN, DHT11_TYPE, DHT22_TYPE);

// whether there's an SD card present in the socket
enum SD_State { SD_REMOVED, SD_READY, SD_UNPREPARED };
SD_State sd_state = SD_REMOVED;

// buffer for reading/writing strings. Using the spark/wiring String class
// would be more convenient, but risks memory fragmentation.
static const int IO_BUFFER_LEN = 36;
char io_buffer[IO_BUFFER_LEN];
char board_temperature[IO_BUFFER_LEN];
char board_humidity[IO_BUFFER_LEN];
char outBH_temperature[IO_BUFFER_LEN];
char topBH_temperature[IO_BUFFER_LEN];
double audio_output[128];
double audio_sampling = 0.0;

/**************************************************************************/
/*
        MQTT Communication
 */
/**************************************************************************/


//void callback(char* topic, byte* payload, unsigned int length);

/**
 * if want to use IP address,
 * byte server[] = { XXX,XXX,XXX,XXX };
 * MQTT client(server, 1883, callback);
 * want to use domain name,
 * MQTT client("www.sample.com", 1883, callback);
 **/
MQTT client( MQTTSERVER , MQTTSERVERPORT , callback);

// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);

    if (message.equals("RED"))
        RGB.color(255, 0, 0);
    else if (message.equals("GREEN"))
        RGB.color(0, 255, 0);
    else if (message.equals("BLUE"))
        RGB.color(0, 0, 255);
    else
        RGB.color(255, 255, 255);
    delay(1000);
}





/**************************************************************************/
/*
        Utility functions
 */
/**************************************************************************/

// writes the contents of io_buffer to all the currently active output destinations.
// last_entry indicates whether the contents of the buffer fall at the end of a
// line of sensor data.
void write(bool last_entry = false)
{
    // double-check that buffer is null-terminated
    io_buffer[IO_BUFFER_LEN-1] = '\0';

    // prep for writing as CSV
    append_suffix(io_buffer, IO_BUFFER_LEN, (last_entry ? LINE_END : DELIMITER));

    // save data to SD card, if it's ready
    if (sd_state == SD_READY && !write_to_sd(SD, io_buffer, LOGFILE_NAME)) {
        DEBUG_PRINTLN("could not write to SD card");
        sd_state = SD_UNPREPARED; // attempt to re-initialize later
    }

    // print to debug output
    DEBUG_PRINT(io_buffer);
}

// output audio data to seperate audio file
void audio_write()
{
  get_timestamp(io_buffer, IO_BUFFER_LEN, GMT_OFFSET);
	// null-termination
	io_buffer[IO_BUFFER_LEN-1] = '\0';
	// append end-delimiter
	append_suffix(io_buffer, IO_BUFFER_LEN, LINE_END );
	// write to SD
	if (sd_state == SD_READY && !write_to_sd(SD, io_buffer, LOGFILE_NAME_AUDIO)) {
        DEBUG_PRINTLN("failed to write to SD card");
        sd_state = SD_UNPREPARED; // attempt to re-initialize later
    }
	updateFFT();
	for(int i=0; i < FFT_SIZE/2+2; i++) {
        csv_audio_output(io_buffer, IO_BUFFER_LEN, i);

			// double-check that buffer is null-terminated
			io_buffer[IO_BUFFER_LEN-1] = '\0';
			// save data to SD card, if it's attached
			if (sd_state == SD_READY && !write_to_sd(SD, io_buffer, LOGFILE_NAME_AUDIO)) {
				DEBUG_PRINTLN("failed to write to SD card");
				sd_state = SD_UNPREPARED; // attempt to re-initialize later
			}
			// print to debug output
			  DEBUG_PRINT(io_buffer);
	}
}

// interrupt handler for SD card
void update_sd_state()
{
    if (!digitalRead(SD_CD_PIN_IN)) {
        // card is absent
        sd_state = SD_REMOVED;
    } else if (sd_state == SD_REMOVED) {
        // card is present, but was previously removed, so requires initialization
        sd_state = SD_UNPREPARED;
    }
}

/**************************************************************************/
/*
        Function:  setup
 */
/**************************************************************************/
void setup()
{
    // Initialize Cloud variables
    Particle.variable("board_temp", board_temperature, STRING);
    Particle.variable("board_hum", board_humidity, STRING);
    Particle.variable("outBH_temp", outBH_temperature, STRING);
    Particle.variable("topBH_temp", topBH_temperature, STRING);
    Particle.variable("audio_samp", &audio_sampling, DOUBLE);

    // Setup serial communication
    Serial.begin(9600);

    DEBUG_PRINTLN("starting setup...");

    // attempt to connect to wifi
    if (!init_wifi()) {
        DEBUG_PRINTLN("could not establish wifi connection");
    }
    else
    {
		    // connect to the mqtt server
			  //client.connect(MQTTID, MQTTUSER, MQTTPSW);
        Particle.connect();
			  if(!Particle.connected()) {
				    //DEBUG_PRINTLN("could not connect to mqtt server");
            DEBUG_PRINTLN("could not connect to particle server");
			  }
			  else
			  {
				    DEBUG_PRINTLN("connected to:");
				    //DEBUG_PRINTLN(MQTTSERVER);
            DEBUG_PRINTLN("Particle cloud");
			  }
	  }

    // initialize sensors
    sensors.begin();

    // reserve memory for FFT (audio analysis)
    FFTinit();

    // attach interrupt on SD card-detect pin to catch inserts/removes
    pinMode(SD_CD_PIN_OUT, OUTPUT); // output signal
    digitalWrite(SD_CD_PIN_OUT, HIGH); // send high
    pinMode(SD_CD_PIN_IN, INPUT_PULLDOWN); // usual input = 0 == card not attached
    attachInterrupt(SD_CD_PIN_IN, update_sd_state, CHANGE); // interrupt function which waits for pin input Change
    update_sd_state(); // set initial state

    // write header for timestamp column
    strncpy(io_buffer, "Timestamp", IO_BUFFER_LEN);
    write();

    // write sensor names/IDs as CSV headers
    for (uint8_t i = 0; i < sensors.count(); ++i) {
        sensors.getSensorString(i, io_buffer, IO_BUFFER_LEN);
        write(i == sensors.count() - 1);
    }

    DEBUG_PRINTLN("setup complete");
}

/**************************************************************************/
/*
        Function:  loop
 */
/**************************************************************************/
void loop()
{
    // timing variables
    static uint32_t next_sync;
    static uint32_t next_read;
    const uint32_t now = millis();


    // make sure our read interval respects our minimum sensor delays
    static const uint32_t read_interval = max(IDEAL_READ_INTERVAL, sensors.minDelay());


    // time synchronization
    if (now > next_sync) {
        DEBUG_PRINTLN("syncing time...");
        sync_time();
        next_sync = now + ONE_DAY_MILLIS;
    }


    // initialize SD card if necessary
    if (sd_state == SD_UNPREPARED) {
        SD.begin();
        sd_state = SD_READY;
    }


    // read and report sensor data
    if (now > next_read)
    {


        DEBUG_PRINTLN("reading...");

        // write timestamp
        get_timestamp(io_buffer, IO_BUFFER_LEN, GMT_OFFSET);
        write();

        //client.publish(MQTTBASETOPIC, "START_TEMP_HUM_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "START_TEMP_HUM_SEQUENCE");
        delay(500);

        // get sensor readings
        for (uint8_t i = 0; i < sensors.count(); ++i) {
            sensors.getEventString(i, io_buffer, IO_BUFFER_LEN);
            DEBUG_PRINTLN("");
            //mqtt write
            char tempstr[30] = "";
            //strcpy(tempstr, MQTTBASETOPIC);
            //strcat(tempstr, mqtttopics[i]);
            strcpy(tempstr, particleevents[i]);

            if (strcmp(tempstr, "board_temperature") == 0) {
                memcpy (board_temperature, io_buffer, IO_BUFFER_LEN);
                /*for(int j=0; j<36; j++){
                    board_temperature[j] = io_buffer[j];
                }*/
            } else if (strcmp(tempstr, "board_humidity") == 0) {
                memcpy (board_humidity, io_buffer, IO_BUFFER_LEN);
                /*for(int j=0; j<36; j++){
                    board_humidity[j] = io_buffer[j];
                }*/
            } else if (strcmp(tempstr, "outBH_temperature") == 0) {
                memcpy (outBH_temperature, io_buffer, IO_BUFFER_LEN);
                /*for(int j=0; j<36; j++){
                    outBH_temperature[j] = io_buffer[j];
                }*/
            } else if (strcmp(tempstr, "topBH_temperature") == 0) {
                memcpy (topBH_temperature, io_buffer, IO_BUFFER_LEN);
                /*for(int j=0; j<36; j++){
                    topBH_temperature[j] = io_buffer[j];
                }*/
            } else {
                DEBUG_PRINTLN("Failed reading sensor");
            }

            if(Particle.connected()) {
                //client.publish(tempstr, io_buffer);
                Particle.publish(tempstr, io_buffer);

                DEBUG_PRINTLN(tempstr);
			      }
			      else //try again if not connected
			      {
                //client.connect(MQTTID, MQTTUSER, MQTTPSW);
				        Particle.connect();
				        if(Particle.connected()) {
                    //client.publish(tempstr, io_buffer);
                    Particle.publish(tempstr, io_buffer);
				        }
			      }

            //end mqtt write

            write(i == sensors.count() - 1);

            delay(5000);

        }
        //client.publish(MQTTBASETOPIC, "END_TEMP_HUM_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "END_TEMP_HUM_SEQUENCE");

        delay(500);

        /*// Perform audio analysis (version 1)
        //updateFFT();
        //audio_write();
		    updateFFT();
		    char tempstr_audio_freq[30] = "";
        char tempstr_audio_magn[30] = "";
        //strcpy(tempstr_audio_freq, MQTTBASETOPIC);
		    //strcat(tempstr_audio_freq, MQTTAUDIOFREQTOPIC);
		    //strcpy(tempstr_audio_magn, MQTTBASETOPIC);
		    //strcat(tempstr_audio_magn, MQTTAUDIOMAGNTOPIC);
        strcpy(tempstr, PARTICLEAUDIOFREQEVENT);
        strcpy(tempstr, PARTICLEAUDIOMAGNEVENT);
		    DEBUG_PRINTLN(tempstr_audio_freq); // A VOIRRRRRRR
        DEBUG_PRINTLN(tempstr_audio_magn); // A VOIIRRRRRRR
        //client.publish(MQTTBASETOPIC, "START_AUDIO_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "START_AUDIO_SEQUENCE");
		    for(int i=0; i < FFT_SIZE/2+2; i++) {
			      mqtt_audio_frequencies_output(io_buffer, IO_BUFFER_LEN, i);
			      //client.publish(tempstr_audio_freq, io_buffer);
            Particle.publish(tempstr_audio_freq, io_buffer);
            delay(500);
            mqtt_audio_magnitudes_output(io_buffer, IO_BUFFER_LEN, i);
			      //client.publish(tempstr_audio_magn, io_buffer);
            Particle.publish(tempstr_audio_magn, io_buffer);
			      delay(500);
		    }
        //client.publish(MQTTBASETOPIC, "END_AUDIO_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "END_AUDIO_SEQUENCE");
        delay(500);*/

        // Perform audio analysis (version 2)
        updateFFT();
        char tempstr[30] = "";
        strcpy(tempstr, PARTICLEAUDIOSIGNEVENT);
        DEBUG_PRINTLN(tempstr);
        Particle.publish(PARTICLECATEGEVENT, "START_AUDIO_SEQUENCE");

        // Sampling of the audio signal at SAMPLEDELAY as sampling period
        for(int i=0; i < SAMP_NUM; i++) {
            audio_output[i] = (double) analogRead(MICROPHONE);
            delayMicroseconds(SAMPLEDELAY);
        }

        // Send audio signal datas to Cloud
        for(int i=0; i < SAMP_NUM; i++) {
            audio_sampling = audio_output[i];
            Particle.publish(tempstr, String(audio_sampling));
            delay(1000);
        }

        Particle.publish(PARTICLECATEGEVENT, "END_AUDIO_SEQUENCE");
        delay(500);

        /*// Read weight data
        char tempstr[30] = "";
        //strcpy(tempstr, MQTTBASETOPIC);
        //strcat(tempstr, MQTTWEIGHTTOPIC);
        strcpy(tempstr, PARTICLEWEIGHTEVENT);
        //client.publish(MQTTBASETOPIC, "START_WEIGHT_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "START_WEIGHT_SEQUENCE");
        delay(500);
        float_t voltage = analogRead(FLEXIFORCE_PIN)*3.3/4095;
        snprintf(io_buffer, IO_BUFFER_LEN, " %4.4f; \n " , voltage);
        if(Particle.connected()) {
            //client.publish(tempstr, io_buffer);
            Particle.publish(tempstr, io_buffer);
            DEBUG_PRINTLN(tempstr);
        }
        else //try again if not connected
        {
            //client.connect(MQTTID, MQTTUSER, MQTTPSW);
            Particle.connect();
            if(Particle.connected()) {
                //client.publish(tempstr, io_buffer);
                Particle.publish(tempstr, io_buffer);
            }
        }
        delay(500);
        //client.publish(MQTTBASETOPIC, "END_WEIGHT_SEQUENCE");
        Particle.publish(PARTICLECATEGEVENT, "END_WEIGHT_SEQUENCE");
        delay(500);*/



        next_read = now + read_interval;


    }

    if (Particle.connected()) {
        Particle.process();
    }

		Serial.println(Time.timeStr());
		delay(500);
}
