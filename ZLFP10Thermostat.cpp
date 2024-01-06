


#include "ZLFP10Thermostat.h"



ZLFP10Thermostat::ZLFP10Thermostat(uint8_t pDHTSensorPin):sensor(pDHTSensorPin, DHT22,6)
#ifdef DEBUGOUTPUTDEVICE_LCD
,lcd(0x27, 20, 4)
#endif
{

}

void ZLFP10Thermostat::setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, uint8_t pDHTSensorPin, int pTempReaderPin) 
{
    DebugSetup();


    sensor.begin();
    TempReaderPin= pTempReaderPin;
     pinMode(TempReaderPin, OUTPUT);
     analogWrite(TempReaderPin, 134); // write a random value to silence the alarm
    RS485.setSerial(phwSerial);
    RS485.setPins(pRS485TXPin, pRS485DEPin, pRS485REPin);
    phwSerial->begin(9600);
// 
 //   RS485.begin(9600);
    
    DEBUGOUTLN("starting");

    if (!ModbusRTUClient.begin(9600)) {
        DEBUGOUTLN("Failed to start Modbus RTU Server!");
        while (1)
            ;

    }
    DEBUGOUTLN("done starting");
}

void ZLFP10Thermostat::ReadSettings() {
  
    #define HOLDINGCOUNT  21
    #define INPUTCOUNT  10
    #define HOLDINGFIRST  0x6E8D
    #define INPUTFIRST  0xB6D1

    short holdingData[HOLDINGCOUNT];
    short inputData[INPUTCOUNT];
    if(!ModbusRTUClient.requestFrom(UNIT_ADDRESS, HOLDING_REGISTERS, HOLDINGFIRST, HOLDINGCOUNT))
    {
      DEBUGOUTLN("");
      DEBUGOUT("Error reading holding registers");
      DEBUGOUTLN("");
      return;
    }
    int x;
    for (x = 0; x < HOLDINGCOUNT; x++) {
        holdingData[x] = ModbusRTUClient.read();

    }
    delay(150);  // need a little bit of time between requests

    if(!ModbusRTUClient.requestFrom(UNIT_ADDRESS, INPUT_REGISTERS, INPUTFIRST, INPUTCOUNT))
    {
      DEBUGOUTLN("");
      DEBUGOUT("Error reading input registers");
      return;
    }

    for (x = 0; x < INPUTCOUNT; x++) {
        inputData[x] = ModbusRTUClient.read();
    }

    actualOnoff = holdingData[0];
    if(holdingData[1]== MODE_AUTO) // auto temp mode, compare setpoints against actual temp
    {
      if(temp > holdingData[11]/10) // cooling set point
      {
        actualMode=MODE_COOL;
      }
      if(temp < holdingData[12]/10) // cooling set point
      {
        actualMode=MODE_HEAT;
      }

      // have to break this out because setpoint could change outside of mode change
      if(actualMode== MODE_HEAT)
      {
        settemp=holdingData[12];
      }
      if(actualMode== MODE_COOL)
      {
        settemp= holdingData[11];
      }

    }
    else
    {
      actualMode = holdingData[1];
      settemp=0;
      if(actualMode==MODE_HEAT)
        settemp = holdingData[10] ;
      if(actualMode==MODE_COOL)
        settemp = holdingData[9] ;
  }
  

    reportedRoomTemp = inputData[0];
    coiltemp = inputData[1];
    actualFanspeed = inputData[2];
    reportedRPM = inputData[3];
    valveOpen = inputData[4];

// fan setting is holdingData[2];

}

void ZLFP10Thermostat::ReadTemp() {

    hum = sensor.readHumidity();
    temp = sensor.readTemperature();
}

void ZLFP10Thermostat::OnHitUpperLimit()
 // called when the upper thermostat limit is hit
 {
    if(actualMode==MODE_HEAT)
      fanspeed--;
    if(actualMode==MODE_COOL)
      fanspeed++;
    // move the threshold up by a unit so we don't trigger again
    upperthreshold += 0.1;
    nextAdjustmentTime = millis()+ADJUSTMENT_INTERVAL;
    // if we're turning fan off, allow another 60 seconds to turn it back on again
    // when you turn the fan speed off, it shuts off the zone valve. When you turn it back on again it takes about 60 seconds for the coil to heat up
    if(fanspeed <=MINFANSPEED)
    {
      nextAdjustmentTime+=60000;
    }

 }
void ZLFP10Thermostat::OnHitLowerLimit()
 // called when the lower thermostat limit is hit
 {

    if(actualMode==MODE_HEAT)
      fanspeed++;
    if(actualMode==MODE_COOL)
    {
      fanspeed--;
      if(fanspeed <=ULOWFANSPEED)
      {
        fanspeed=ULOWFANSPEED;
      }
    }
    // move the threshold down a unit
    lowerthreshold -= 0.1;
    nextAdjustmentTime = millis()+ADJUSTMENT_INTERVAL;
    // if we're turning fan off, allow another 60 seconds to turn it back on again
    if(fanspeed <=MINFANSPEED)
    {
      nextAdjustmentTime+=60000;
    }

 }


void ZLFP10Thermostat::SetFanSpeed()
{
  // version 3 -- look up setting in the table created in Calibrate()

  int newSetting;
  newSetting=SetLevels[fanspeed];
  analogWrite( TempReaderPin, newSetting); 
  delay(500); // to prevent read errors
  DEBUGOUT(" Setpoint=");
  DEBUGOUT(newSetting);
  DEBUGOUTLN("");

}

//////////////////////////////////////////////////////////////////////////////

// This is the heart of the program.

// Essentially, you check the current temperature, if it's above the setpoint you slow the fan down, if it's below you speed the fan up
// But there are lots of twists

// If the fan is on its lowest setting, and the temperture is above the setpoint, stop the fan. Do this by switching the unit out of heating mode
// and into auto mode. The setpoint for auto needs to be set below that for heating.

// You don't want the fan speed changing around a lot, that's annoying. So the variable nextAdjustmentTime is used to limit adjustments to one
// every three minutes -- four if you're coming out of being off because it takes about a minute for the unit to come on

// The temperature sensor tends to bounce around between two degrees. This function gets called whenever the temperature changes, you only want to 
// change the fan speed once for each setpoint. The variables upperthreshold and lowerthreshold hold the current points at which the fan speed changes. 
// Once a threshold has been hit, it gets moved 0.1 away from the setpoint, so the temperature has to move a whole unit to move it again. If the
// temperture moves two units toward the setpoint then the threshold gets reset to its original value.

// On startup, if the temperature is below the setpoint the fan is put on highest speed until the setpoint is reached for the first time.
// This is tracked in the variable startingmode.

// modes
//0=auto；1=cooling;2=dehumidification；3=ventilate；4=heating；
// dehum and ventilate don't care about temperature, just let run
// cooling and heating are the same, the only difference is which direction the temperature goes in
// auto I need to think about

bool ZLFP10Thermostat::UpdateFanSpeed() 

{
    // if upperthreshold is zero, we're being called for the first time
    if (upperthreshold == 0)
    {
        
        upperthreshold = ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0;
        lowerthreshold = ((float)settemp - THERMOSTAT_INTERVAL) / 10.0;
        // if we're outside the thermostat band, set to off or highest. Otherwise do nothing, just keep doing what you're doing
        if(actualMode==MODE_HEAT)
        {
          if (temp >= upperthreshold) {

            fanspeed = MINFANSPEED;
          }
          if (temp < lowerthreshold) {

            fanspeed = MAXFANSPEED;
          }
        }
        if(actualMode==MODE_COOL)
        {
          if (temp < lowerthreshold) {
            // in cool mode, run at Ultra-low at all times for dehumidification
            fanspeed = ULOWFANSPEED;
          }
          if (temp >= upperthreshold) {

            fanspeed = MAXFANSPEED;
          }
        }
    }


    //Whenever we cross a threshold, we increase that threshold so we don't keep crossing back and forth
    // When we move 2 units away from the threshold toward the setpoint, then reset the threshold 
    //does the threshold need to be adjusted? Have we moved two units closer to the setpoint?
    if (upperthreshold > temp + 0.2)
    {
        upperthreshold = temp + 0.2;
        if (upperthreshold < ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0)
        {
            upperthreshold = ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0;
        }
    }
    if (lowerthreshold < temp - 0.2)
    {
        lowerthreshold = temp - 0.2;
        if (lowerthreshold > ((float)settemp - THERMOSTAT_INTERVAL) / 10.0)
        {
            lowerthreshold = ((float)settemp - THERMOSTAT_INTERVAL) / 10.0;
        }

    }


  
    
    
      if (millis()  > nextAdjustmentTime) // don't adust fan more than once every three minutes
      {
          // this is meant to compare temp >=upperthreshold . However since they are floating points the exact equals doesn't always hit
            if (upperthreshold - temp < 0.09) 
            { 

                // only adjust speed down on rising temp

                
                OnHitUpperLimit();
            }
            if (temp - lowerthreshold < 0.09)
            {
                // only adjust speed up on falling temperature
                OnHitLowerLimit();
          }
      }
    

    if (fanspeed < MINFANSPEED) {
        fanspeed = MINFANSPEED;
    }
    if (fanspeed > MAXFANSPEED) {
        fanspeed = MAXFANSPEED;
    }
    
    // on startup, if we're outside the band we go to highest setting. The first time we reach the band, throttle back to LOWFANSPEED
    if (startingmode)
    {
        if(actualMode==MODE_HEAT)
        {
          if (temp > lowerthreshold)
          {
              if (fanspeed > LOWFANSPEED)
                  fanspeed = LOWFANSPEED;
              startingmode = false;
              DEBUGOUTLN("Leaving starting mode");
          }
        }
        if(actualMode==MODE_COOL)
        {
          if (temp < upperthreshold)
          {
              if (fanspeed > LOWFANSPEED)
                  fanspeed = LOWFANSPEED;
              startingmode = false;
              DEBUGOUTLN("Leaving starting mode");
          }
        }

    }

    if (actualFanspeed == fanspeed)
    {
          // nothing has changed, we're done
            return true;
    }
    DEBUGOUTLN("");

    SetFanSpeed();
    ReadSettings(); // read back the settings for display
    return true; // success!
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
    DEBUGOUT(", Temp=");
    DEBUGOUT2(temp, 1);
    DEBUGOUT(", Humidity=");
    DEBUGOUT2(hum, 0);
    DEBUGOUT(", Setpoint:");
    DEBUGOUT(settemp / 10);
    DEBUGOUT('.');
    DEBUGOUT(settemp % 10);
    DEBUGOUT(", Fan Speed=");
    DEBUGOUT(fanspeed);
    DEBUGOUT(", Reported Room Temp=");
    DEBUGOUT(reportedRoomTemp / 10);
    DEBUGOUT('.');
    DEBUGOUT(reportedRoomTemp % 10);
    DEBUGOUT(", Fan RPM=");
    DEBUGOUT(reportedRPM);

    DEBUGOUT(", Reported Fan setting=");
    DEBUGOUT(actualFanspeed);
    DEBUGOUT(", Coil Temperature=");
    DEBUGOUT(coiltemp / 10);
    DEBUGOUT(", Mode=");
    DEBUGOUT(actualMode); 
    DEBUGOUT(", Valve=");
    DEBUGOUT(valveOpen);
    DEBUGOUT(", U=");
    DEBUGOUT(upperthreshold);
    DEBUGOUT(" L=");
    DEBUGOUT(lowerthreshold);

    DEBUGOUT('\r');
}

void ZLFP10Thermostat::RestartSession()
{
    fanspeed = -1;
    startingmode = true;
    nextAdjustmentTime = 0;
    nextcheck = 0;
    upperthreshold = 0;
    lowerthreshold=0;

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
  if(actualMode==MODE_HEAT)
  {
    temps[0]=(settemp-10);
    temps[1]=(settemp-20);
    temps[2]=(settemp-30);
  }
  else
  {
    temps[0]=(settemp+10);
    temps[1]=(settemp+20);
    temps[2]=(settemp+30);

  }
  SetLevels[1]=LevelH + (LevelL-LevelH)*(ReadingH-temps[0])/(ReadingH-ReadingL);
  SetLevels[2]=LevelH + (LevelL-LevelH)*(ReadingH-temps[1])/(ReadingH-ReadingL);
  SetLevels[3]=LevelH + (LevelL-LevelH)*(ReadingH-temps[2])/(ReadingH-ReadingL);
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
        SetLevels[y] += (reading-settemp+y*10)/10;
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

  if(actualMode==MODE_HEAT)
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
    float oldtemp = temp;
    delay(1000);
    ReadTemp();
    if (!isnan(temp))
    {
        unsigned long now = millis();
        if (oldtemp != temp || now > nextcheck  || nextcheck == 0)
        {
            short oldOnOff, oldMode, oldSetTemp;
            oldOnOff=actualOnoff;
            oldMode=actualMode ;
            oldSetTemp= settemp;

    
            ReadSettings();
            // if on/off, mode or setpoint has changed since last iteration, reset everything
            if(oldOnOff!=actualOnoff || oldMode!=actualMode ||  oldSetTemp!= settemp)
            {
              RestartSession();
            }



            if (UpdateFanSpeed())
            {
                nextcheck = now +60000; // don't check for 60 seconds unless the temperature chnages
            }
            else
            {
                DEBUGOUTLN("");
                // error. don't count this as a successful update, so dont update temperature
                temp = oldtemp;
            }
        }
        DisplayStatus();
        
    }
    else
    {
        DEBUGOUT("INVALID TEMP");
        temp = oldtemp;
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