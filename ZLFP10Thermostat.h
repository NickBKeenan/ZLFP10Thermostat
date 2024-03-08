// define this if you want debug output
#define DEBUGOUTPUT_ENABLED  
// if you define DEBUGOUTPUT_ENABLED, you have to define one of the following two:
//#define DEBUGOUTPUTDEVICE_LCD
#define DEBUGOUTPUTDEVICE_SERIAL

#ifdef DEBUGOUTPUT_ENABLED  
  #ifdef DEBUGOUTPUTDEVICE_SERIAL
    #define DEBUGOUT(x) Serial.print(x)
    #define DEBUGOUT2(x,y) Serial.print(x,y)
    #define DEBUGOUTLN(x) Serial.println(x)
  #endif
  #ifdef DEBUGOUTPUTDEVICE_LCD
    #define DEBUGOUT(x) lcd.print(x)
    #define DEBUGOUT2(x,y) lcd.print(x,y)
    #define DEBUGOUTLN(x) lcd.setCursor(0, 0);lcd.print(x)
  #endif
#else
  #define DEBUGOUT(x)
  #define DEBUGOUT2(x,y)
  #define DEBUGOUTLN(x) 
#endif
#include <MultiStageThermostat.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#ifdef DEBUGOUTPUTDEVICE_LCD
  #include <LiquidCrystal_I2C.h>
#endif  
#include <DHT.h>

// Set up a new SoftwareSerial object
#define MAXFANSPEED 4
#define MEDFANSPEED 3
#define LOWFANSPEED 2
#define ULOWFANSPEED 1
#define MINFANSPEED 0  //  0=off
#define THERMOSTAT_INTERVAL 2  // how many tenths of a degree above and below the setpoint to allow
#define ADJUSTMENT_INTERVAL 180 // don't adust more than once every 3 minutes
#define UNIT_ADDRESS 15 // the MODBUS address of the FCU. This can be changed through the control panel if needed.
#define FCU_MODE_AUTO 0
#define FCU_MODE_COOL 1
#define FCU_MODE_DEHU 2
#define FCU_MODE_VENT 3
#define FCU_MODE_HEAT 4



class ZLFP10Thermostat: public MultiStageThermostat
{

  Settings settings;
    unsigned long nextcheck = 0;   // track the last time we read settings and status from the unit, do it every minute

    int TempReaderPin; // PWm pin that connects to FCU temp reader
    
  
    short actualOnoff;  // what the unit reports for onoff setting
    short Mode;   // The mode we're in, heat or cool
    short reportedFanSetting; // what the unit reports for fan speed setting
    short reportedRoomTemp; // what the unit reports for room temp. Not used, just for curiosity
    short reportedRPM;    // what the unit reports for fan rpm. Not used, just for curiosity
    short coiltemp;     // what the unit reports for coil temperature Not used, just for curiosity
    short valveOpen;  // what the unit reports for zone valve setting.Not used, just for curiosity 
    short lastTempSetting=0; // used for deflutter
    short FCUSetTemp;
    short SetLevels[5]; // the digital output level for each fan speed, set in Calibrate(); 
    #ifdef DEBUGOUTPUTDEVICE_LCD
      LiquidCrystal_I2C lcd;
    #endif
    void RestartSession(); // called when the mode or setpoint changes
public:
    ZLFP10Thermostat(uint8_t pDHTSensorPin);
    
    void setSerial(SoftwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin);
    void setTempReader(uint8_t pTempReaderPin);
    void setup();
    void ReadSettings(); // read from onboard settings
    void SetFanSpeed(int fanspeed); // write out to temperature spoof
    void DisplayStatus();
    void loop();
    void Calibrate();   // calibrate temperature spoofing
    void DeFlutter();  // if temperature fluttering is detected, revisit the calibration
    void DebugSetup(); // setup the debug output device, can be Serial or LCD
  

};

extern ZLFP10Thermostat theThermostat;
