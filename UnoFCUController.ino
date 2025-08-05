




#include "ZLFP10Thermostat.h"
#include "DebugFramework.h"
#include "LEDStatusStrip.h"

//#define DEBUGOUTPUTDEVICE_LCD
#define DEBUGOUTPUTDEVICE_SERIAL

#ifdef DEBUGOUTPUTDEVICE_LCD
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27, 20, 4);
  struct LCDStream : public Stream{
  NullStream( void ) { return; };
  int available( void ) { return 0; };
  void flush( void ) { return; };
  int peek( void ) { return -1; };
  int read( void ){ return -1; };;
  size_t write( uint8_t u_Data ){ return lcd.write(u_Data); };
};
LCDStream lcdDebug;


#endif  

#define SERIAL1_DE_PIN 2
#define ROOM_TEMP_PIN 3
#define SW_SERIAL_TX_PIN 4
#define SW_SERIAL_DE 5
#define SW_SERIAL_RX_PIN 6
#define DHT_SENSOR_PIN 7
#define STATUSBASEPIN 8
#define STATUSPINCOUNT 4 // pins are 8 through 11
#define COIL_TEMP_PIN 12
#define CLIENT_MODBUS_ID 16
#define SERVER_MODBUS_ID 96
#define ENABLE_LEDS false

SoftwareSerial SoftSerial(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);

ZLFP10Thermostat theThermostat(DHT_SENSOR_PIN);



struct NullStream : public Stream{
  NullStream( void ) { return; };
  int available( void ) { return 0; };
  void flush( void ) { return; };
  int peek( void ) { return -1; };
  int read( void ){ return -1; };;
  size_t write( uint8_t u_Data ){ return u_Data, 0x01; };
};

NullStream DebugNull;

void setup() {
  LEDStatusStrip::setLEDsEnabled(ENABLE_LEDS);

  theThermostat.setTempPins(ROOM_TEMP_PIN, COIL_TEMP_PIN);
  DebugSetup();
  
  // Initialize SoftwareSerial for Modbus server
  SoftSerial.begin(9600);  // Back to 9600 baud

  theThermostat.setClientSerial(Serial1,SERIAL1_DE_PIN, SERIAL1_DE_PIN , CLIENT_MODBUS_ID);
  theThermostat.setServerSerial(SoftSerial,SW_SERIAL_DE, SW_SERIAL_DE, SERVER_MODBUS_ID );
  
  theThermostat.setup();
  
}

void loop()
{


  theThermostat.loop( );
  
}
  
  

  void DebugSetup()
{
  // Initialize debug framework
  Debug.setDebugLevel(DEBUG_LEVEL_INFO);
  Debug.enableModule(DEBUG_MODULE_MAIN);
  Debug.enableModule(DEBUG_MODULE_THERMOSTAT);
  Debug.enableModule(DEBUG_MODULE_FCU);
  Debug.enableModule(DEBUG_MODULE_MODBUS);
  Debug.enableModule(DEBUG_MODULE_SENSOR);
  Debug.enableModule(DEBUG_MODULE_LED);
  
  // Set up debug output
  #ifdef DEBUGOUTPUTDEVICE_SERIAL
    Serial.begin(9600);
    while(!Serial && millis() < 5000)
      ;
    
    if(Serial) {
      Debug.setDebugStream(&Serial);
      Debug.info(DEBUG_MODULE_MAIN, "Debug framework initialized");
      Debug.info(DEBUG_MODULE_MAIN, "Starting");
      Debug.info(DEBUG_MODULE_MAIN, "Build date");
      theThermostat.SetDebugOutput(&Serial);
    }
  #endif
 
  #ifdef DEBUGOUTPUTDEVICE_LCD
    lcd.init();  // initialize the lcd
    lcd.clear();
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("--------------------");
    lcd.setCursor(6, 1);
    lcd.print("GEEEKPI");
    lcd.setCursor(1, 2);
    lcd.print("Arduino IIC Screen");
    lcd.setCursor(0, 3);
    lcd.print("--------------------");
    lcd.clear();
    lcd.setCursor(0, 0);
    Debug.setDebugStream(&lcdDebug);
    theThermostat.SetDebugOutput(&lcdDebug);
  #endif
}
