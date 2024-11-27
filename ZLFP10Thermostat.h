#include <DehumidifyingMultiStageThermostat.h>



#include <ZLFP10Controller.h>
#include "ZLFP10ModbusServer.h"



class ZLFP10Thermostat: public DehumidifyingMultiStageThermostat
{

  Settings settings;
  
  ZLFP10Controller FCUController;
  
    unsigned long nextcheck = 0;   // track the last time we read settings and status from the unit, do it every minute

    
    
  
    short Onoff;  // what the unit reports for onoff setting
    short Mode;   // The mode we're in, heat or cool
    short FCUSetTemp; // the temperature setting the FCU is reporting
    

    Stream *DebugStream; // used for debug output, can be serial, LCD, or none
    void RestartSession(); // called when the mode or setpoint changes
    void ReadFCUSettings(); // Get info from the FCUController

public:
    ZLFP10Thermostat(uint8_t pDHTSensorPin);
    void setClientSerial(HardwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin, uint8_t ModbusID); // assign the softwareserial port used by MODBUS
    void setServerSerial(SoftwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin, uint8_t ModbusID); // assign the softwareserial port used by MODBUS
    void setTempPins(uint8_t pRoomTempPin, uint8_t pCoilTempPin); // assign the pins used for the FCU temp reader
    void setup();
    void DisplayStatus();  // called once per loop
    void loop();
    
    void SetDebugOutput(Stream * pDebug); // set the device for debug output
    void DoServerAction(); // called when the server needs something from the FCU

};


