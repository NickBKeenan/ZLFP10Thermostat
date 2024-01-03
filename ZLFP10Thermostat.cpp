


#include "ZLFP10Thermostat.h"



ZLFP10Thermostat::ZLFP10Thermostat(uint8_t pDHTSensorPin):sensor(pDHTSensorPin, DHT22,6)
{

}

void ZLFP10Thermostat::setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, uint8_t pDHTSensorPin, int pTempReaderPin) 
{



    sensor.begin();
    TempReaderPin= pTempReaderPin;
     pinMode(TempReaderPin, OUTPUT);
     analogWrite(TempReaderPin, 134);
    RS485.setPins(pRS485TXPin, pRS485DEPin, pRS485REPin);
    RS485.setSerial(phwSerial);
    Serial.begin(9600);
    Serial.println("starting");

    if (!ModbusRTUClient.begin(9600)) {
        Serial.println("Failed to start Modbus RTU Server!");
        while (1)
            ;

    }
    fanspeed = -1;
    startingmode = true;
    Calibrate();
    Serial.println("done starting");
}

void ZLFP10Thermostat::ReadSettings() {
    int holdingCount = 21;
    int inputCount = 10;
    int holdingFirst = 0x6E8D;
    int inputFirst = 0xB6D1;

    short holdingData[holdingCount];
    short inputData[inputCount];
    if(!ModbusRTUClient.requestFrom(UNIT_ADDRESS, HOLDING_REGISTERS, holdingFirst, holdingCount))
    {
      Serial.println();
      Serial.print("Error reading holding registers");
      Serial.println();
      return;
    }
    int x;
    for (x = 0; x < holdingCount; x++) {
        holdingData[x] = ModbusRTUClient.read();

    }
    delay(150);  // need a little bit of time between requests

    if(!ModbusRTUClient.requestFrom(UNIT_ADDRESS, INPUT_REGISTERS, inputFirst, inputCount))
    {
      Serial.println();
      Serial.print("Error reading input registers");
      return;
    }

    for (x = 0; x < inputCount; x++) {
        inputData[x] = ModbusRTUClient.read();
    }

    actualOnoff = holdingData[0];
    actualMode = holdingData[1];
    settemp = holdingData[10] ;

    reportedRoomTemp = inputData[0];
    coiltemp = inputData[1];
    actualFanspeed = inputData[2];
    reportedRPM = inputData[3];
    valveOpen = inputData[4];


}

void ZLFP10Thermostat::ReadTemp() {

    hum = sensor.readHumidity();
    temp = sensor.readTemperature();
}



void ZLFP10Thermostat::SetFanSpeed(short newFanSpeed)
{
  // version 3 -- look up setting in the table created in Calibrate()

  int newspeed=0;
  newspeed=SetLevels[newFanSpeed];
  /* version 2
  Serial.println();
  Serial.print("Setting fan speed=");
  Serial.print(newFanSpeed);
  // the fan speed is setpoint-reported temperature
  // Using the onboard 5V, it seems that reported=165-input
  // which gives input=165-reported=165-(setpoint-fanspeed)= 165-setpoint+fanspeed
  // if fanspeed is Min or max, bump it to be sure

  // that's the calculated fan speed. But if we get called again, and the newFanSpeed is the same as last time we were called, it's
  // because the reported fan speed hasn't met what we requested. In that case, bump the setting in the direction that we're off
  newspeed= 165-settemp/10+newFanSpeed;
  if(newFanSpeed==MINFANSPEED)
  {
    newspeed--;
  }
  if(newFanSpeed==MAXFANSPEED)
  {
    newspeed++;
  }
  if(newFanSpeed==lastFanSpeed) // we've been here before
  {
    Serial.println("Overriding fan speed:");
    Serial.print(newspeed);
    newspeed=lastFanSpeedOutput+lastFanSpeed-actualFanspeed;
    Serial.print(" with ");
    Serial.print(newspeed);
  }
  lastFanSpeed=newFanSpeed;
  lastFanSpeedOutput=newspeed; 
  */ 
  //end of version 2
  analogWrite( TempReaderPin, newspeed); 
  delay(500); // to prevent read errors
  Serial.print(" Setpoint=");
  Serial.print(newspeed);
  Serial.println();

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

bool ZLFP10Thermostat::UpdateFanSpeed(float oldtemp) 

{


    // does the threshold need to be adjusted? Have we move two units closer to the setpoint?
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


  
    // if oldtemp is zero, we're being called for the first time
    if (oldtemp == 0)
    {
        // if we're outside the thermostat band, set to off or highest. Otherwise do nothing, just keep doing what you're doing
        upperthreshold = ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0;
        lowerthreshold = ((float)settemp - THERMOSTAT_INTERVAL) / 10.0;

        if (temp >= upperthreshold) {

            fanspeed = MINFANSPEED;
        }
        if (temp < lowerthreshold) {

            fanspeed = MAXFANSPEED;
        }
    }
    else
    {
        if (millis()  > nextAdjustmentTime) // don't adust fan more than once every three minutes
        {
          // this is meant to compare temp >=upperthreshold . However since they are floating points the exact equals doesn't always hit
            if (upperthreshold - temp < 0.09) { 

                // only adjust speed down on rising temp

                fanspeed--;
                // move the threshold up by a unit so we don't trigger again
                upperthreshold += 0.1;
                nextAdjustmentTime = millis()+ADJUSTMENT_INTERVAL;
                // if we're turning fan off, allow another 60 seconds to turn it back on again
                if(fanspeed <=MINFANSPEED)
                {
                  nextAdjustmentTime+=60000;
                }
            }
            if (temp - lowerthreshold < 0.09)
            {
                // only adjust speed up on falling temperature
                fanspeed++;
                // move the threshold down a unit
                lowerthreshold -= 0.1;
                nextAdjustmentTime = millis()+ADJUSTMENT_INTERVAL;
            }
        }
    }

    if (fanspeed < MINFANSPEED) {
        fanspeed = MINFANSPEED;
    }
    if (fanspeed > MAXFANSPEED) {
        fanspeed = MAXFANSPEED;
    }
    
    // on startup, if we're outside the band we go to highest setting. When we reach the band, throttle back to 2
    if (startingmode)
    {
        if (temp > lowerthreshold)
        {
            if (fanspeed > 2)
                fanspeed = 2;
            startingmode = false;
            Serial.println("Leaving starting mode");
        }
    }

        if (actualFanspeed == fanspeed)
        {
          // nothing has changed, we're done
            return true;
        }
    Serial.println();

    SetFanSpeed(fanspeed);
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

    Serial.print(hour);
    Serial.print(":");
    if (minutes < 10)
        Serial.print('0');
    Serial.print(minutes);
    Serial.print(":");

    if (seconds < 10)
        Serial.print('0');
    Serial.print(seconds);
    Serial.print(", Temp=");
    Serial.print(temp, 1);
    Serial.print(", Humidity=");
    Serial.print(hum, 0);
    Serial.print(", Setpoint:");
    Serial.print(settemp / 10);
    Serial.print('.');
    Serial.print(settemp % 10);
    Serial.print(", Fan Speed=");
    Serial.print(fanspeed);
    Serial.print(", Reported Room Temp=");
    Serial.print(reportedRoomTemp / 10);
    Serial.print('.');
    Serial.print(reportedRoomTemp % 10);
    Serial.print(", Fan RPM=");
    Serial.print(reportedRPM);

    Serial.print(", Reported Fan setting=");
    Serial.print(actualFanspeed);
    Serial.print(", Coil Temperature=");
    Serial.print(coiltemp / 10);
    Serial.print(", Mode=");
    Serial.print(actualMode); 
    Serial.print(", Valve=");
    Serial.print(valveOpen);
    Serial.print(", U=");
    Serial.print(upperthreshold);
    Serial.print(" L=");
    Serial.print(lowerthreshold);

    Serial.print('\r');
}



void ZLFP10Thermostat::Calibrate()
{

// the fan speed is setpoint minus reported temperature
  // Using the onboard 5V, it seems that reported is roughly 165-input
  
  // So estimate the extremes of our range, 15C and 30C, measure what we get, and then interpolate the points in between
  // Then test them against actual and adjust

  // We only need three points 1,2,3. Zero is anything less than one, and four is anything less than three. 
  //So figure out those three and then substract two from one to get zero, and add two to three to get four
  Serial.println("Calibrating...");
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
  Serial.print(LevelL); //150
  Serial.print(" ");
  Serial.print(ReadingL); //170 17c
  Serial.print(" ");
  Serial.print(LevelH); //135
  Serial.print(" ");
  Serial.print(ReadingH); //300 30c
  Serial.print(" ");

  // (LevelH-TargetLevel)/(LevelH-LevlL)= (ReadingH-TargetReading)/(ReadingH-ReadingL); or
  // TargetLevel= LevelH - (LevelH-LevelL)*(ReadingH-TargetReading)/(ReadingH-ReadingL)
// get initial guesses based on what the extremes of the range told us
  SetLevels[1]=LevelH + (LevelL-LevelH)*(ReadingH-(settemp-10))/(ReadingH-ReadingL);
  SetLevels[2]=LevelH + (LevelL-LevelH)*(ReadingH-(settemp-20))/(ReadingH-ReadingL);
  SetLevels[3]=LevelH + (LevelL-LevelH)*(ReadingH-(settemp-30))/(ReadingH-ReadingL);
  int x;
  
  // go through the settings, set each one and then compare what happens and adjust
  // Keep going until you can do it three times without adjustment, or 30 times total (failure)
 int goodTries=0; // number of sucessful reads   
  for(x=0; x< 30 && goodTries < 3; x++)
  {
    int y;
    Serial.println();
    int goodLevels=0;    // number of successful reads in this iteration
    for(y=1; y< 4; y++)
    {
      short reading;
      Serial.print(SetLevels[y]);
      analogWrite(TempReaderPin, SetLevels[y]);
      delay(1000);
      ReadSettings();
      reading=reportedRoomTemp;
      Serial.print(" ");
      Serial.print(reading);
      Serial.print(" . ");

      if(reading != settemp-y*10 )
      {// need adjustment
        Serial.print(" * ");
        SetLevels[y] += (reading-settemp+y*10)/10;
        Serial.print(SetLevels[y]);
        Serial.print(" * ");
        
      }
      else
      {// it works!
        Serial.print(" - ");
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
    SetLevels[0]=SetLevels[1]-2;
    SetLevels[4]=SetLevels[3]+2;
  if(goodTries==3)
  {
    Serial.println("Done Calibrating");
  }
  else
  {
    Serial.println("Unable to calibrate");
    // trigger a P5 error
    analogWrite(TempReaderPin,0);
    while(1);
  }
  
}

void ZLFP10Thermostat::loop() {

    // once a second read temp, if changed then 
    // once a minute, and on change in temp, read settings
    float oldtemp = temp;
    delay(1000);
    ReadTemp();
    if (!isnan(temp))
    {
        unsigned long now = millis();
        if (oldtemp != temp || now > nextcheck  || nextcheck == 0)
        {
            ReadSettings();

            if (UpdateFanSpeed(oldtemp))
            {
                nextcheck = now +60000; // don't check for 60 seconds unless the temperature chnages
            }
            else
            {
                Serial.println();
                temp = oldtemp;
            }
        }
        DisplayStatus();
        
    }
    else
    {
        temp = oldtemp;
    }


}
