

#include "ZLFP10Thermostat.h"


#define RS485_RX_PIN 2  // this goes to RD pin on 485
#define RS485_TX_PIN 3  // this goes to DI pin on 485 
#define DHT_SENSOR_PIN 6
#define RS485_RE_PIN 7
#define RS485_DE_PIN 8
#define TEMP_READER_PIN 9

ZLFP10Thermostat theThermostat(DHT_SENSOR_PIN);
SoftwareSerial SoftSerial(RS485_RX_PIN, RS485_TX_PIN);

void setup() {
  theThermostat.setSerial(SoftSerial,RS485_DE_PIN, RS485_RE_PIN );
  
  theThermostat.setTempReader(TEMP_READER_PIN);
  theThermostat.setup();

}

void loop()
{
  theThermostat.loop( );
}
  
  