#include "Adafruit_SHT31.h"
#include "config.h"
#include "util.h"
//#include "Wire.h"


using namespace NECTAR;

// disable GSM on boot
SYSTEM_MODE(SEMI_AUTOMATIC);
// SYSTEM_MODE(MANUAL);

char *sens_datas_str = "";
Adafruit_SHT31 sht31;

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
/*MQTT client( MQTTSERVER , MQTTSERVERPORT , callback);*/

// recieve message
/*void callback(char* topic, byte* payload, unsigned int length) {
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
        Function:  setup
 */
/**************************************************************************/
void setup()
{
    // Initialize Wire communication
    Wire.begin();

    // Initialize Serial communication
    Serial.begin(9600);
    Serial.println("Serial port is ready for communication");

    DEBUG_PRINTLN("starting setup...");

    // Initialize Cellular and Particle Cloud connections
    // attempt to connect to wifi
    if (!init_wifi()) {
        DEBUG_PRINTLN("could not establish wifi connection");
    } else {
        // connect to the mqtt server
  			Particle.connect();
  			if(!Particle.connected()) {
  			    DEBUG_PRINTLN("could not connect to Particle Cloud");
  			} else {
      			DEBUG_PRINTLN("connected to Particle Cloud");
  			}
	  }
    //initWholeConnection();

    // Initialize Cloud variables
    Particle.variable("datas", sens_datas_str, STRING);

    // initialize SHT31 sensors from beehives
    //beginSHT31Sensors(sht31);
    delay(5000);
    tcaselect(0);
    if (!sht31.begin(0x44)) {
        Serial.println("Oups Problem");
    }
    Serial.println(sht31.readTemperature());
    delay(5000);
    /*Serial.println(sht31.readHumidity());
    delay(1000);*/

    /*tcaselect(1);
    sht31.begin(0x44);
    Serial.println(sht31.readTemperature());
    delay(1000);
    Serial.println(sht31.readHumidity());
    delay(1000);

    tcaselect(2);
    sht31.begin(0x44);
    Serial.println(sht31.readTemperature());
    delay(1000);
    Serial.println(sht31.readHumidity());
    delay(1000);

    tcaselect(4);
    sht31.begin(0x44);
    Serial.println(sht31.readTemperature());
    delay(1000);
    Serial.println(sht31.readHumidity());
    delay(1000);*/

    DEBUG_PRINTLN("setup complete");
}

/**************************************************************************/
/*
        Function:  loop
 */
/**************************************************************************/
void loop()
{
    // make sure our read interval respects our minimum sensor delays
    // static const uint32_t read_time_interval = max(READ_TIME_INTERVAL, sensors.minDelay());

    // temperature and humidity arrat
    static float temp_hum_float_array[SENSORS_NBR];

    // timing variables
    static uint32_t next_time_sync;
    static uint32_t next_read_send_datas;
    static uint32_t sleep_delay;
    const uint32_t now = millis();
    //const unsigned long now = millis();

    // time synchronization
    if (now > next_time_sync) {
        // Synchronise hardware time with Particle Cloud one
        DEBUG_PRINTLN("syncing hardware and Particle Cloud times...");
        syncTime();
        DEBUG_PRINTLN("hardware and Particle Cloud times are synchronized");

        next_time_sync = now + ONE_DAY_MILLIS;
    }

    // read datas from sensors and send them to Particle cloud
    if (now > next_read_send_datas) {
        // Read datas from sensors
        //DEBUG_PRINTLN("reading datas from sensors...");
        //readAllSHT31SensorsDatas(temp_hum_float_array, sht31);
        readAllSHT31SensorsDatas(temp_hum_float_array, sht31);
        //DEBUG_PRINTLN("sensors datas were well read");

        // Send sensors datas to Particle Cloud
        //DEBUG_PRINTLN("sending sensors datas to Particle Cloud...");
        /*if (Particle.connected()) {
            String sens_datas_str = floatArrayToString(temp_hum_float_array, SENSORS_NBR);
        } else {
            particleConnection();
            if (Particle.connected()) {
                String sens_datas_str = floatArrayToString(temp_hum_float_array, SENSORS_NBR);
            }
        }*/
        //DEBUG_PRINTLN("sensors datas were well sent to Particle cloud");

        next_read_send_datas = now + READ_SEND_TIME_INTERVAL;
    }

    if (Particle.connected()) {
        Particle.process();
    }

    Serial.println(Time.timeStr());
    delay(1000); // Maybe unnecessary

    // Activate deep sleep mode of the device
    //sleep_delay = min(( next_read_send_datas - now ), ( next_time_sync - now ));
    //System.sleep(SLEEP_MODE_DEEP, sleep_delay);


    /*// Activate sleep mode of the device
    sleep_delay = min(( next_cellular_sending - now ), ( next_time_sync - now ));
    System.sleep(WAKE_UP_PIN, CHANGE, sleep_delay); // [SLEEP_NETWORK_STANDBY] is unnecessary*/
}
