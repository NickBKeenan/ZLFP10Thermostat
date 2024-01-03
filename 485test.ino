
#include "ZLFP10Thermostat.h"
#define SERIAL_PORT_HARDWARE Serial1
#define RS485_TX_PIN 18
#define RS485_DE_PIN 7
#define RS485_RE_PIN 8
#define DHT_SENSOR_PIN 6
#define TEMP_READER_PIN 9

ZLFP10Thermostat theThermostat(DHT_SENSOR_PIN);


void setup() {
   pinMode(A0, OUTPUT);
   analogWrite(A0, 150); 
  theThermostat.setup(&SERIAL_PORT_HARDWARE, RS485_TX_PIN, RS485_DE_PIN, RS485_RE_PIN, DHT_SENSOR_PIN, TEMP_READER_PIN);

}

void loop()
{
  theThermostat.loop( );
}
  
