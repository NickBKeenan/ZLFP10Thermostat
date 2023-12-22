


#include <ArduinoModbus.h>
#include <DHT.h>

// Set up a new SoftwareSerial object
#define MAXFANSPEED 4
#define THERMOSTAT_INTERVAL 2


class ZLFP10Thermostat
{
    DHT sensor;
    
    float hum;       //Stores humidity value
    float temp;      //Stores temperature value
    short fanspeed;  // fan speed setting
    short settemp;
    int address = 15;
    bool startingmode;
    short actualOnoff;
    short actualMode;
    short actualFanspeed;
    short reportedRoomTemp;
    short reportedRPM;
    short coiltemp;
    short valveOpen;
    float upperthreshold = 0;
    float lowerthreshold = 0;
    unsigned long lastAdjustment = 0;
    unsigned long lastcheck = 0;
public:
    ZLFP10Thermostat(int pDHTSensorPin);
    void setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, int pDHTSensorPin);
    void ReadSettings();
    void ReadTemp();
    bool UpdateFanSpeed(float oldtemp);
    void DisplayStatus();
    void loop();

};

extern ZLFP10Thermostat theThermostat;