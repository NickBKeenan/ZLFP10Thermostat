


#include <ArduinoModbus.h>
#include <DHT.h>

// Set up a new SoftwareSerial object
#define MAXFANSPEED 4
#define MINFANSPEED 1  // 2 is actually the lowest settable speed, 1=off
#define THERMOSTAT_INTERVAL 2  // how many tenths of a degree above and below the setpoint to allow
#define UNIT_ADDRESS 15 // the MODBUS address of the FCU. This can be changed through the control panel if needed.


class ZLFP10Thermostat
{
    DHT sensor;   // the temp/humidity sensor
    
    float hum;       //Stores humidity value
    float temp;      //Stores temperature value
    short fanspeed;  // fan speed setting
    short settemp;   // temperature setting from the unit
  
    bool startingmode;  //this is true until the thermostat has been satisfied for the first time
    short actualOnoff;  // what the unit reports for onoff setting
    short actualMode;   // what the unit reports for mode -- heat, cool, vent, etc.
    short actualFanspeed; // what the unit reports for fan speed setting
    short reportedRoomTemp; // what the unit reports for room temp. Not used, just for curiosity
    short reportedRPM;    // what the unit reports for fan rpm. Not used, just for curiosity
    short coiltemp;     // what the unit reports for coil temperature Not used, just for curiosity
    short valveOpen;  // what the unit reports for zone valve setting.Not used, just for curiosity 
    float upperthreshold = 0;  // if we hit this temperature, slow the fan down
    float lowerthreshold = 0; // if we hit this temperature, speed the fan up
    unsigned long lastAdjustment = 0; //don't adjust the fan speed more than once every three minutes
    unsigned long lastcheck = 0;   // track the last time we read settings and status from the unit, do it every minute
public:
    ZLFP10Thermostat(int pDHTSensorPin);
    void setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, int pDHTSensorPin);
    void ReadSettings();
    void ReadTemp();
    bool UpdateFanSpeed(float oldtemp);  // this is really the heart of it
    void DisplayStatus();
    void loop();

};

extern ZLFP10Thermostat theThermostat;