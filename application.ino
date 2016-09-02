#include "config.h"
#include "util.h"
#include "Sensor_Array.h"
#include "sd-card-library.h"
#include "audio.h"
#include "MQTT.h"

using namespace OSBH;

// disable wifi on boot
SYSTEM_MODE(SEMI_AUTOMATIC);

// global sensor array
Sensor_Array sensors(ONE_WIRE_PIN, DHT_PIN1, DHT_PIN2, DHT_TYPE);

// whether there's an SD card present in the socket
enum SD_State { SD_REMOVED, SD_READY, SD_UNPREPARED };
SD_State sd_state = SD_REMOVED;

// buffer for reading/writing strings. Using the spark/wiring String class
// would be more convenient, but risks memory fragmentation.
static const int IO_BUFFER_LEN = 36;
char io_buffer[IO_BUFFER_LEN];

/**************************************************************************/
/*
        MQTT Communication
 */
/**************************************************************************/


void callback(char* topic, byte* payload, unsigned int length);

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
    Serial.begin(9600);
    DEBUG_PRINTLN("starting setup...");

   // attempt to connect to wifi
    if (!init_wifi()) {
        DEBUG_PRINTLN("could not establish wifi connection");
    }
    else
    {
		 // connect to the mqtt server
			client.connect(MQTTID, MQTTUSER, MQTTPSW);
			if(!client.isConnected()) {
				 DEBUG_PRINTLN("could not connect to mqtt server");
			}
			else
			{
				DEBUG_PRINTLN("connected to:");
				DEBUG_PRINTLN(MQTTSERVER);
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


        // get sensor readings
        for (uint8_t i = 0; i < sensors.count(); ++i) {
            sensors.getEventString(i, io_buffer, IO_BUFFER_LEN);

            //mqtt write
            char tempstr[30] = "";
            strcpy(tempstr, MQTTBASETOPIC);
            strcat(tempstr, mqtttopics[i]);
            if(client.isConnected()){
            //client.publish("ronjac/feeds/Bienensensor","20000.00;");
            client.publish("xdb/feeds/Bienensensor","20000.00;");
            DEBUG_PRINTLN(tempstr);
            DEBUG_PRINTLN("jemand da?");
			}
			else //try again if not connected
			{
				client.connect(MQTTID, MQTTUSER, MQTTPSW);
				if(client.isConnected()){
				client.publish("nn/feeds/Temp1",io_buffer);

				}
			}

            //end mqtt write

            write(i == sensors.count() - 1);

        }


        //perform audio analysis
       //updateFFT();
       //audio_write();
		updateFFT();
		char tempstr[30] = "";
		strcpy(tempstr, MQTTBASETOPIC);
		strcat(tempstr, MQTTAUDIOTOPIC);
		DEBUG_PRINTLN(tempstr);
		for(int i=0; i < FFT_SIZE/2+2; i++) {
			mqtt_audio_output(io_buffer, IO_BUFFER_LEN, i);
			client.publish(tempstr,io_buffer);
			delay(500);
		}



        next_read = now + read_interval;


    }

    if (Spark.connected()) Spark.process();
		Serial.println(Time.timeStr());
		delay(1000);
}
