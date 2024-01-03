#define AVR
#include <ArduinoModbus.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Set up a new SoftwareSerial object
#define MAXFANSPEED 4
#define MINFANSPEED 0  // 2 is actually the lowest settable speed, 1=off
#define THERMOSTAT_INTERVAL 2  // how many tenths of a degree above and below the setpoint to allow
#define ADJUSTMENT_INTERVAL 180000 // don't adust more than once every 3 minutes
#define UNIT_ADDRESS 15 // the MODBUS address of the FCU. This can be changed through the control panel if needed.
#define MODE_AUTO 0
#define MODE_HEAT 4


class ZLFP10Thermostat
{
    DHT sensor;   // the temp/humidity sensor
    int TempReaderPin; // PWm pin that connects to FCU temp reader
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
    unsigned long nextAdjustmentTime = 0; //don't adjust the fan speed more than once every three minutes
    unsigned long nextcheck = 0;   // track the last time we read settings and status from the unit, do it every minute
    short lastFanSpeed=0; // track what we tried to set the last time we set
    int lastFanSpeedOutput=0;
    short SetLevels[5]; // the digital output level for each fan speed, set in Calibrate(); 
public:
    ZLFP10Thermostat(uint8_t pDHTSensorPin);
    void setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, uint8_t pDHTSensorPin, int pTempReaderPin);
    void ReadSettings();
    void ReadTemp();
    bool UpdateFanSpeed(float oldtemp);  // this is really the heart of it
    void SetFanSpeed(short newFanSpeed); 
    void DisplayStatus();
    void loop();
    void Calibrate();

};

extern ZLFP10Thermostat theThermostat;
extern const char * HoldingLabels[];
extern  char * InputLabels[];