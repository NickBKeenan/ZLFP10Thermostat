


#include "ZLFP10Thermostat.h"
#include <ZLFP10Controller.h>
#include "LEDStatusStrip.h"
#include "DewPoint.h"
#define STATUSPINCOUNT 4
#define STATUSBASEPIN 8




ZLFP10Thermostat::ZLFP10Thermostat(uint8_t pDHTSensorPin):DehumidifyingMultiStageThermostat(pDHTSensorPin) 
{
        Mode=MODE_OFF;
        theLEDStatusStrip.SetPins(STATUSBASEPIN, STATUSPINCOUNT);
}

void ZLFP10Thermostat::setTempPins(uint8_t pRoomTempPin, uint8_t pCoilTempPin)
{
  FCUController.setRoomTempPin(pRoomTempPin);
  FCUController.setCoilTempPin(pCoilTempPin);
}

void ZLFP10Thermostat::setClientSerial(HardwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin,  uint8_t ModbusID)
{
  FCUController.setClientHardwareSerial(pswSerial,  pRS485DEPin,  pRS485REPin, ModbusID);
}

ZLFP10ModbusServer TheServer;

void ZLFP10Thermostat::setServerSerial(SoftwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin, uint8_t ModbusID)
{
  FCUController.setServerSoftwareSerial(pswSerial,  pRS485DEPin,  pRS485REPin,  ModbusID, &TheServer);
  TheServer.SetupClient(FCUController.GetClient());
  TheServer.SetParentThermostat(this);
}

void ZLFP10Thermostat::setup() 
{
    DEBUG_INFO(DEBUG_MODULE_THERMOSTAT, "Starting thermostat setup");
    DEBUG_INFO_STR(DEBUG_MODULE_THERMOSTAT, "File", __FILE__);
    DEBUG_INFO_STR(DEBUG_MODULE_THERMOSTAT, "Build date", __DATE__);

    LEDStatusStrip theLEDStatusStrip;
  
    setThermostatInterval(0.2);
    setDefaultStage(2);
    MultiStageThermostat::setup();
    DEBUG_INFO(DEBUG_MODULE_THERMOSTAT, "Thermostat setup complete");
    EnableDehumidify();
}


void ZLFP10Thermostat::DisplayStatus() {
    // Use the debug framework for status display
    DEBUG_INFO(DEBUG_MODULE_THERMOSTAT, "Status update");
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "On/Off", Onoff);
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Mode", Mode);
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Fan Mode Setting", FCUController.FCUSettings.FanModeSetting);
    DEBUG_INFO_FLOAT(DEBUG_MODULE_THERMOSTAT, "Temperature", getTemp());
    DEBUG_INFO_FLOAT(DEBUG_MODULE_THERMOSTAT, "Humidity", getHumidity());
    DEBUG_INFO_FLOAT(DEBUG_MODULE_THERMOSTAT, "Dew point", DewPoint(getTemp(), getHumidity()));
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Setpoint", FCUSetTemp);
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Stage", getLastStage());
    DEBUG_INFO_FLOAT(DEBUG_MODULE_THERMOSTAT, "Upper threshold", upperthreshold);
    DEBUG_INFO_FLOAT(DEBUG_MODULE_THERMOSTAT, "Lower threshold", lowerthreshold);
    DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Next check", (nextAdjustmentTime > millis()) ? (nextAdjustmentTime-millis())/1000 : 0);
    
    // FCU properties
    DEBUG_INFO_INT(DEBUG_MODULE_FCU, "Temp pin", FCUController.lastTempPin);
    DEBUG_INFO_FLOAT(DEBUG_MODULE_FCU, "Reported room temp", FCUController.FCUSettings.RoomTemp);
    DEBUG_INFO_INT(DEBUG_MODULE_FCU, "Fan RPM", FCUController.FCUSettings.FanRPM);
    DEBUG_INFO_INT(DEBUG_MODULE_FCU, "Fan setting", FCUController.FCUSettings.FanSetting);
    DEBUG_INFO_FLOAT(DEBUG_MODULE_FCU, "Coil temperature", FCUController.FCUSettings.coilTemp);
    DEBUG_INFO_INT(DEBUG_MODULE_FCU, "Valve open", FCUController.FCUSettings.valveOpen);
    DEBUG_INFO_INT(DEBUG_MODULE_FCU, "Fan fault", FCUController.FCUSettings.FanFault);
}
int Delays[]=
{
  ADJUSTMENT_INTERVAL+60, 
  ADJUSTMENT_INTERVAL,
  ADJUSTMENT_INTERVAL,
  ADJUSTMENT_INTERVAL, 
  ADJUSTMENT_INTERVAL
};

void ZLFP10Thermostat::RestartSession()
{
    DEBUG_INFO(DEBUG_MODULE_THERMOSTAT, "Restarting session - applying new settings");
    
    nextcheck = 0;
    settings.CoolingSetpoint=FCUSetTemp;
    settings.HeatingSetpoint=FCUSetTemp;
    settings.mode=Mode;
     
    setSettings(settings);
    setStageDelays(MAXFANSPEED+1, Delays);
    theLEDStatusStrip.BlinkEm(2, 100);
    theLEDStatusStrip.SetStatus(2);
    FCUController.Calibrate();

    // for unknown reasons, when the FCU is powered off and on it comes up with an E0 error. This detects that error and attempts to clear it
    // by soft-powering off and on
    if(FCUController.FCUSettings.Onoff)
    {
      if(FCUController.FCUSettings.FanFault)
      {
        FCUController.SetOnOff(false);
        delay(1000);
        FCUController.SetOnOff(true);

      }
    }
    DehuSettings dhs;
    dhs.bottomTempLimit=1.0;
	dhs.bottomTempRange=0.2;
	dhs.HumidityLimit=55;
	dhs.HumidityRange=5;

  SetParameters(&dhs);
}

int oldStage=-1;
short oldCoil=-1;
    

void ZLFP10Thermostat::loop() 

{


    // once a second read temp, 
    // once a minute, and on change in temp, read settings
    float oldtemp = getTemp();
    delay(1000);
    ReadTemp();
    
    unsigned long now = millis();
    float temp=getTemp();
    if (oldtemp != temp || now > nextcheck  || nextcheck == 0)
    {
            short oldOnOff, oldMode, oldSetTemp;
            oldOnOff=Onoff;
            oldMode=Mode ;
            oldSetTemp= FCUSetTemp;

    
            ReadFCUSettings();
            // if on/off, mode or setpoint has changed since last iteration, reset everything
            if(oldOnOff!=Onoff || oldMode!=Mode ||  oldSetTemp!= FCUSetTemp)
            {
              DEBUG_INFO(DEBUG_MODULE_THERMOSTAT, "Restarting session");
              DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "On/Off", Onoff);
              DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "Mode", Mode);
              DEBUG_INFO_INT(DEBUG_MODULE_THERMOSTAT, "FCU Set Temp", FCUSetTemp);
              RestartSession();
            }

            int newStage=getStage();

            if(oldStage!=newStage || nextcheck==0)// first time in
            {
              DebugStream->println();
              FCUController.SetFanSpeed(newStage);
              theLEDStatusStrip.SetStatus(newStage);
            }
              oldStage=newStage;
              if(newStage==4)
              {
                if(FCUController.FCUSettings.coilTemp != oldCoil)
                {
                  DebugStream->println();
                }
              }
            oldCoil=FCUController.FCUSettings.coilTemp;
            nextcheck = now +10000; // don't check for 60 seconds unless the temperature chnages
        }
        DisplayStatus();
        
    // check the server side    
    FCUController.ServiceAnyRequests();
   


}

void ZLFP10Thermostat::ReadFCUSettings()
{
  FCUController.ReadSettings();  

  Onoff=FCUController.FCUSettings.Onoff;
    
    if(FCUController.FCUSettings.Mode== FCU_MODE_AUTO) // auto temp mode, compare setpoints against actual temp
    {
      Mode=MODE_COOL;
      FCUSetTemp = FCUController.FCUSettings.CoolSetpoint;
      /*if(getTemp() > FCUController.FCUSettings.AutoCoolingSetpoint) // cooling set point
      {
        Mode=MODE_COOL;
      }
      if(getTemp() < FCUController.FCUSettings.AutoHeatingSetpoint) // heating set point
      {
        Mode=MODE_HEAT;
      }

      // have to break this out because setpoint could change outside of mode change
      if(Mode== MODE_HEAT)
      {
        FCUSetTemp=FCUController.FCUSettings.AutoHeatingSetpoint;
      }
      if(Mode== MODE_COOL)
      {
        FCUSetTemp= FCUController.FCUSettings.AutoHeatingSetpoint;
      }
      */
    }
    else
    {
      short FCUMode;
      FCUMode = FCUController.FCUSettings.Mode;
      FCUSetTemp=0;
      if(FCUMode==FCU_MODE_HEAT)
      {
        FCUSetTemp = FCUController.FCUSettings.HeatSetpoint;
        Mode=MODE_HEAT;
      }
      if(FCUMode==FCU_MODE_COOL)
      {
        FCUSetTemp = FCUController.FCUSettings.CoolSetpoint;
        Mode=MODE_COOL;
      }
  }
  


}

void ZLFP10Thermostat::SetDebugOutput(Stream * pDebug)
{
  DebugStream=pDebug;
  FCUController.SetDebugOutput(pDebug);
};

// Getter methods for FCU settings
word ZLFP10Thermostat::getFCUOnOffStatus() {
    return FCUController.FCUSettings.Onoff;
}

word ZLFP10Thermostat::getFCUModeStatus() {
    return FCUController.FCUSettings.Mode;
}

word ZLFP10Thermostat::getFCUFanSpeedStatus() {
    return FCUController.FCUSettings.FanModeSetting;
}

// Additional getter methods for new registers
int ZLFP10Thermostat::getTempFault() {
    return FCUController.FCUSettings.TempFault;
}

int ZLFP10Thermostat::getCoilTempFault() {
    return FCUController.FCUSettings.CoilTempFault;
}

float ZLFP10Thermostat::getHumidity() {
    return lastHum; // Use inherited humidity from Arduino sensor
}

float ZLFP10Thermostat::getActualHumidity() {
    return lastHum * 10; // Return humidity * 10 for register 39322 (similar to getActualRoomTemp)
}

// Missing getter methods for FCU holding registers
word ZLFP10Thermostat::getCoolSetpoint() {
    return FCUController.FCUSettings.CoolSetpoint;
}

word ZLFP10Thermostat::getHeatSetpoint() {
    return FCUController.FCUSettings.HeatSetpoint;
}

// Missing getter methods for FCU input registers
word ZLFP10Thermostat::getFCURoomTemp() {
    return FCUController.FCUSettings.RoomTemp;
}

word ZLFP10Thermostat::getCoilTemp() {
    return FCUController.FCUSettings.coilTemp;
}

word ZLFP10Thermostat::getFanSetting() {
    return FCUController.FCUSettings.FanSetting;
}

word ZLFP10Thermostat::getFanRPM() {
    return FCUController.FCUSettings.FanRPM;
}

word ZLFP10Thermostat::getValveOpen() {
    return FCUController.FCUSettings.valveOpen;
}

word ZLFP10Thermostat::getFanFault() {
    return FCUController.FCUSettings.FanFault;
}

// Missing getter methods for thermostat state
word ZLFP10Thermostat::getOnoff() {
    return Onoff;
}

word ZLFP10Thermostat::getMode() {
    return Mode;
}

word ZLFP10Thermostat::getFCUSetTemp() {
    return FCUSetTemp;
}

