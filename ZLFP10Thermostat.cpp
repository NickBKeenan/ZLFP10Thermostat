


#include "ZLFP10Thermostat.h"

ModbusMaster node;

ZLFP10Thermostat::ZLFP10Thermostat(uint8_t pDHTSensorPin):MultiStageThermostat(pDHTSensorPin) //, DHT22,6)
#ifdef DEBUGOUTPUTDEVICE_LCD
,lcd(0x27, 20, 4)
#endif
{

}
void ZLFP10Thermostat::setTempReader(uint8_t pTempReaderPin)
{
  TempReaderPin= pTempReaderPin;
}
void ZLFP10Thermostat::setSerial(SoftwareSerial &pswSerial, uint8_t pRS485DEPin,uint8_t pRS485REPin)
{
    pswSerial.begin(9600);

    node.begin(15, pswSerial);
  DEBUGOUTLN("node begin");
  node.SetPins(pRS485DEPin, pRS485REPin);
  DEBUGOUTLN("Pins set");    


}

void ZLFP10Thermostat::setup() 
{
    DebugSetup();
    DEBUGOUTLN("starting");


    
  
     pinMode(TempReaderPin, OUTPUT);
     analogWrite(TempReaderPin, 134); // write a random value to silence the alarm
    setThermostatInterval(0.1);
    setDefaultStage(2);
    MultiStageThermostat::setup();
    DEBUGOUTLN("done starting");
}

void ZLFP10Thermostat::ReadSettings() {
  
    
    #define HOLDINGCOUNT  21
    #define INPUTCOUNT  10
    #define HOLDINGFIRST  0x6E8D
    #define INPUTFIRST  0xB6D1

    short holdingData[HOLDINGCOUNT];
    short inputData[INPUTCOUNT];
      uint8_t  result;
    


    result = node.readHoldingRegisters( HOLDINGFIRST, HOLDINGCOUNT);
    if(result !=0)
    {
      DEBUGOUTLN("");
      DEBUGOUT("Error reading holding registers ");
      DEBUGOUT2(result, HEX);
      DEBUGOUTLN("");
      return;
    }
    int x;
    for (x = 0; x < HOLDINGCOUNT; x++) {
              holdingData[x] = node.getResponseBuffer(x);


    }
    delay(150);  // need a little bit of time between requests

    result = node.readInputRegisters( INPUTFIRST, INPUTCOUNT);
    if(result !=0)
    {
      DEBUGOUTLN("");
      DEBUGOUT("Error reading input registers ");
      DEBUGOUT2(result, HEX);
      DEBUGOUTLN("");
      
      return;
    }

    for (x = 0; x < INPUTCOUNT; x++) {
        inputData[x] = node.getResponseBuffer(x);
    }

    actualOnoff = holdingData[0];
    if(holdingData[1]== FCU_MODE_AUTO) // auto temp mode, compare setpoints against actual temp
    {
      if(getTemp() > holdingData[11]/10) // cooling set point
      {
        Mode=MODE_COOL;
      }
      if(getTemp() < holdingData[12]/10) // cooling set point
      {
        Mode=MODE_HEAT;
      }

      // have to break this out because setpoint could change outside of mode change
      if(Mode== MODE_HEAT)
      {
        FCUSetTemp=holdingData[12]/10;
      }
      if(Mode== MODE_COOL)
      {
        FCUSetTemp= holdingData[11]/10;
      }

    }
    else
    {
      short FCUMode;
      FCUMode = holdingData[1];
      FCUSetTemp=0;
      if(FCUMode==FCU_MODE_HEAT)
      {
        FCUSetTemp = holdingData[10]/10 ;
        Mode=MODE_HEAT;
      }
      if(FCUMode==FCU_MODE_COOL)
      {
        FCUSetTemp = holdingData[9]/10 ;
        Mode=MODE_COOL;
      }
  }
  

    reportedRoomTemp = inputData[0]/10;
    coiltemp = inputData[1]/10;
    if(reportedFanSetting != inputData[2] && !startingmode)
    {
      DEBUGOUTLN("");
    }
    
    reportedFanSetting = inputData[2];
    reportedRPM = inputData[3];
    valveOpen = inputData[4];

// fan setting is holdingData[2];

    // fluttering detected, try to adjust setpoints
    if(lastTempSetting !=0 && reportedRoomTemp!= lastTempSetting && millis()> nextAdjustmentTime && nextAdjustmentTime > 0)
    {
        DeFlutter();
    }

}


void ZLFP10Thermostat::SetFanSpeed(int fanspeed)
{
  // version 3 -- look up setting in the table created in Calibrate()

  int newSetting;
  newSetting=SetLevels[fanspeed];
  analogWrite( TempReaderPin, newSetting); 
  delay(500); // to prevent read errors
  DEBUGOUTLN("");
  DEBUGOUT(" Setpoint=");
  DEBUGOUT(newSetting);
  DEBUGOUTLN("");
  if(Mode==MODE_HEAT)
    lastTempSetting=FCUSetTemp-fanspeed;
  else
    lastTempSetting=FCUSetTemp+fanspeed;
}


void ZLFP10Thermostat::DisplayStatus() {
    unsigned long hour;
    unsigned long minutes;
    unsigned long seconds;
    unsigned long now = millis();
    seconds = now / 1000;
    minutes = now / 60000;
    seconds -= minutes * 60;

    hour = minutes / 60;
    minutes -= hour * 60;

    DEBUGOUT(hour);
    DEBUGOUT(":");
    if (minutes < 10)
        DEBUGOUT('0');
    DEBUGOUT(minutes);
    DEBUGOUT(":");

    if (seconds < 10)
        DEBUGOUT('0');
    DEBUGOUT(seconds);
    // sensor properties
    DEBUGOUT(", Temp=");
    DEBUGOUT2(getTemp(), 1);
    
    DEBUGOUT(", Humidity=");
    DEBUGOUT2(getHumidity(), 0);
    // thermostat properties
    DEBUGOUT(", Setpoint:");
    DEBUGOUT(FCUSetTemp );
    DEBUGOUT(", Stage=");
    DEBUGOUT(getLastStage());
    DEBUGOUT(", U=");
    DEBUGOUT2(upperthreshold,1);
    DEBUGOUT(" L=");
    DEBUGOUT2(lowerthreshold,1);
    DEBUGOUT(" Next check=");
    if (nextAdjustmentTime > millis())
    {
        Serial.print((nextAdjustmentTime-millis())/1000);
    }
    else
        Serial.print("0");


    //FCU properties
    DEBUGOUT(", Reported Room Temp=");
    DEBUGOUT(reportedRoomTemp );
    
    
    DEBUGOUT(",  RPM=");
    DEBUGOUT(reportedRPM);

    DEBUGOUT(", Fan setting=");
    DEBUGOUT(reportedFanSetting);
    DEBUGOUT(", Coil Temperature=");
    DEBUGOUT(coiltemp);
    DEBUGOUT(", Valve=");
    DEBUGOUT(valveOpen);
    DEBUGOUT("   " );
    DEBUGOUT('\r');
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
     nextcheck = 0;
    settings.CoolingSetpoint=FCUSetTemp;
    settings.HeatingSetpoint=FCUSetTemp;
    settings.mode=Mode;
     
    setSettings(settings);
    setStageDelays(MAXFANSPEED+1, Delays);

    Calibrate();
 
}

void ZLFP10Thermostat::Calibrate()
{

// the fan speed is setpoint minus reported temperature
  // Using the onboard 5V, it seems that reported is roughly 165-input
  
  // So estimate the extremes of our range, 15C and 30C, measure what we get, and then interpolate the points in between
  // Then test them against actual and adjust

  // We only need three points 1,2,3. Zero is anything less than one, and four is anything less than three. 
  //So figure out those three and then substract two from one to get zero, and add two to three to get four
  DEBUGOUTLN("Calibrating...");
  

  short LevelL, LevelH;   // input levels for the extremes, low and high
  short ReadingL, ReadingH; // room temperature readings we get back
  LevelL= 165-15; // set for 15 C
  LevelH=165-30;  // set for 30 C

  //check out low end
  analogWrite(TempReaderPin, LevelL);
  delay(1000);
  ReadSettings();
  ReadingL=reportedRoomTemp;

  // check out high end
  analogWrite(TempReaderPin, LevelH);
  delay(1000);
  ReadSettings();
  ReadingH=reportedRoomTemp;
  DEBUGOUT(LevelL); //150
  DEBUGOUT(" ");
  DEBUGOUT(ReadingL); //170 17c
  DEBUGOUT(" ");
  DEBUGOUT(LevelH); //135
  DEBUGOUT(" ");
  DEBUGOUT(ReadingH); //300 30c
  DEBUGOUT(" ");

  // (LevelH-TargetLevel)/(LevelH-LevlL)= (ReadingH-TargetReading)/(ReadingH-ReadingL); or
  // TargetLevel= LevelH - (LevelH-LevelL)*(ReadingH-TargetReading)/(ReadingH-ReadingL)
// get initial guesses based on what the extremes of the range told us
  short temps[3];
  
  if(Mode==MODE_HEAT)
  {
    temps[0]=(FCUSetTemp-1);
    temps[1]=(FCUSetTemp-2);
    temps[2]=(FCUSetTemp-3);
  }
  else
  {
    temps[0]=(FCUSetTemp+1);
    temps[1]=(FCUSetTemp+2);
    temps[2]=(FCUSetTemp+3);

  }
  SetLevels[1]=LevelH + (LevelL-LevelH)*(ReadingH-temps[0])/(ReadingH-ReadingL);
  SetLevels[2]=LevelH + (LevelL-LevelH)*(ReadingH-temps[1])/(ReadingH-ReadingL);
  SetLevels[3]=LevelH + (LevelL-LevelH)*(ReadingH-temps[2])/(ReadingH-ReadingL);

// on the basement FCU, both 156 and 157 map to 20C. However, 156 flutters with 21C
// This line forces it to pick 157, which seems stable
// I'm not sure how to solve this, one idea would be to adaptively adjust the values when flutter happens.
 // SetLevels[2]=157;
  int x;
  
  // go through the settings, set each one and then compare what happens and adjust
  // Keep going until you can do it three times without adjustment, or 30 times total (failure)
 int goodTries=0; // number of sucessful reads   
  for(x=0; x< 30 && goodTries < 3; x++)
  {
    int y;
    DEBUGOUTLN("");
    int goodLevels=0;    // number of successful reads in this iteration
    for(y=1; y< 4; y++)
    {
      short reading;
      DEBUGOUT(SetLevels[y]);
      analogWrite(TempReaderPin, SetLevels[y]);
      delay(1000);
      ReadSettings();
      reading=reportedRoomTemp;
      DEBUGOUT(" ");
      DEBUGOUT(reading);
      DEBUGOUT(" . ");

      if(reading != temps[y-1] )
      {// need adjustment
        DEBUGOUT(" * ");
        SetLevels[y] += (reading-FCUSetTemp+y);
        DEBUGOUT(SetLevels[y]);
        DEBUGOUT(" * ");
        
      }
      else
      {// it works!
        DEBUGOUT(" - ");
        goodLevels++;
      }
      
    }// go through each level
    if(goodLevels==3) // every read on this try was successful
    {
      goodTries++;
    }
    else
    {
      goodTries=0; // reset to zero on failure, it has to be three in a row because failure also mean we adjust the values
    }
  
  }// tries

  if(Mode==MODE_HEAT)
  {
    SetLevels[0]=SetLevels[1]-2;
    SetLevels[4]=SetLevels[3]+2;
  }
  else
  {
    SetLevels[0]=SetLevels[1]+2;
    SetLevels[4]=SetLevels[3]-2;
  }
  if(goodTries==3)
  {
    DEBUGOUTLN("Done Calibrating");
  }
  else
  {
    DEBUGOUTLN("Unable to calibrate");
    // trigger a P5 error
    analogWrite(TempReaderPin,0);
    while(1);
  }
  
}

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
            oldOnOff=actualOnoff;
            oldMode=Mode ;
            oldSetTemp= FCUSetTemp;

    
            ReadSettings();
            // if on/off, mode or setpoint has changed since last iteration, reset everything
            if(oldOnOff!=actualOnoff || oldMode!=Mode ||  oldSetTemp!= FCUSetTemp)
            {
              RestartSession();
            }

            int oldStage=getLastStage();
            int newStage=getStage();
            if(oldStage!=newStage || nextcheck==0)// first time in
            {
              SetFanSpeed(newStage);
            }

            nextcheck = now +10000; // don't check for 60 seconds unless the temperature chnages
        }
        DisplayStatus();
        
    
   


}

void ZLFP10Thermostat::DeFlutter()
{
  // we get here when we set the temperature, and then we read it back and it's not what we set it to.
  // we have:
  // settemp -- thermostat setting on the unit
  // reportedRoomTemp  -- what the unit is reporting
  // lastTempSetting -- what we set the temp to
  DEBUGOUTLN();
  DEBUGOUT("DEFluttering. Settemp=");
  DEBUGOUT(lastTempSetting);
  DEBUGOUT(" reported temp=");
  DEBUGOUTLN(reportedRoomTemp);
  DEBUGOUTLN();


  if(Mode==MODE_HEAT)
  { 
    
    if(lastTempSetting==FCUSetTemp)
    {
        if(reportedRoomTemp< lastTempSetting )
        {
          SetLevels[0]--;
          DEBUGOUT("Adjusting level 0 to ");
          DEBUGOUTLN(SetLevels[0]);
          return;
        }
    }
    if(lastTempSetting==FCUSetTemp-(MAXFANSPEED))
    {
        if(reportedRoomTemp> lastTempSetting )
        {
          SetLevels[MAXFANSPEED]++;
          DEBUGOUT("Adjusting level ");
          DEBUGOUT(MAXFANSPEED);
          DEBUGOUT(" to ");
          DEBUGOUTLN(SetLevels[MAXFANSPEED]);
          return;
        }
    }
    if(reportedRoomTemp > lastTempSetting)
    {
      // 
      int setting =(FCUSetTemp-lastTempSetting);
      DEBUGOUT(" Setting=");
      DEBUGOUT(setting);
      DEBUGOUT(" Level=");
      DEBUGOUT(SetLevels[setting]);
      DEBUGOUT(" Next Level=");
      DEBUGOUT(SetLevels[setting+1]);

      if(SetLevels[setting+1]-SetLevels[setting]>1)
      {
        SetLevels[setting]++;
          DEBUGOUT("Adjusting level ");
          DEBUGOUT(setting);
          DEBUGOUT(" to ");
          DEBUGOUTLN(SetLevels[setting]);
      }
    }
    else
    {
      
      int setting =(FCUSetTemp-lastTempSetting);
      if(SetLevels[setting]-SetLevels[setting-1]>1)
      {
        SetLevels[setting]--;
          DEBUGOUT("Adjusting level ");
          DEBUGOUT(setting);
          DEBUGOUT(" to ");
          DEBUGOUTLN(SetLevels[setting]);
      }

    }
  } 
  else
  {
    // cooling TBD
  } 

}
void ZLFP10Thermostat::DebugSetup()
{
  #ifdef DEBUGOUTPUT_ENABLED  
    #ifdef DEBUGOUTPUTDEVICE_SERIAL
      Serial.begin(9600);
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
   #endif
   #endif
}