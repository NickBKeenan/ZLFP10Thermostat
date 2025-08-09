


#include "ZLFP10ModbusServer.h"
#include "ZLFP10Thermostat.h"

void ZLFP10ModbusServer::RequestReaction()
{

  Serial.println();

  switch(ReceivedFunctionCode())
  {
      case MODBUS04_ReadInputRegisters:
        Serial.println("DEBUG: Processing READ_INPUT_REGISTERS");
        readInputRegisters();  
        Serial.println("DEBUG: READ_INPUT_REGISTERS completed");
        break;
      case MODBUS03_ReadHoldingRegisters:
        Serial.println("DEBUG: Processing READ_HOLDING_REGISTERS");
        readHoldingRegisters(); 
        Serial.println("DEBUG: READ_HOLDING_REGISTERS completed");
        break;
    case MODBUS10_WriteMultipleRegisters:
      Serial.println("DEBUG: Processing WRITE_MULTIPLE_REGISTERS");
      writeMultipleRegisters();
      Serial.println("DEBUG: WRITE_MULTIPLE_REGISTERS completed");
      break;
    default:
      Serial.print("DEBUG: Illegal function code: 0x");
      Serial.println(ReceivedFunctionCode(), HEX);
    IllegalFunction();

  }

  Serial.println("DEBUG: RequestReaction completed");

}

void ZLFP10ModbusServer::readHoldingRegisters()
{
  // called when a packet has been received
  uint16_t Register, Response;
  word Count;
  Register=ResponseBufferGetAt(0);
  Count= ResponseBufferGetAt(1);
  
  DEBUG_INFO_INT(DEBUG_MODULE_MODBUS, "readHoldingRegisters", Register);
  DEBUG_INFO_INT(DEBUG_MODULE_MODBUS, "Count", Count);
  
  // Handle multiple registers if Count > 1
  for(int i = 0; i < Count; i++) {
    word value = 0;
    uint16_t currentRegister = Register + i;
    
    // Map register addresses to internal getter methods
    switch(currentRegister) {
      case 28301: // FCU On/Off status
        value = ((ZLFP10Thermostat*)pParentThermostat)->getFCUOnOffStatus();
        break;
        
      case 28302: // FCU Mode status
        value = ((ZLFP10Thermostat*)pParentThermostat)->getFCUModeStatus();
        break;
        
      case 28303: // FCU Fan Speed
        value = ((ZLFP10Thermostat*)pParentThermostat)->getFCUFanSpeedStatus();
        break;
        
      case 28312: // FCU Cooling Setpoint
        value = ((ZLFP10Thermostat*)pParentThermostat)->getCoolSetpoint();
        break;
        
      case 28313: // FCU Heating Setpoint
        value = ((ZLFP10Thermostat*)pParentThermostat)->getHeatSetpoint();
        break;
        
      default:
        value = 9999;
        break;
    }
    
    TransmitBufferPutAt(i, value);
  }
  
  // Add delay to prevent frame overlap
  delay(0);

  // Validate we're sending the correct function code
  if(ReceivedFunctionCode() != MODBUS03_ReadHoldingRegisters) {
    DEBUG_INFO(DEBUG_MODULE_MODBUS, "ERROR: Function code mismatch!");
    return;
  }
  
  SendFrame(MODBUS03_ReadHoldingRegisters, ReceivedAddress(), Count, Register, false);
  
  DEBUG_INFO(DEBUG_MODULE_MODBUS, "Holding registers response sent successfully");
  
  // Add additional delay after sending
  delay(0);
}

   
void  ZLFP10ModbusServer::readInputRegisters()
{
  DEBUG_INFO(DEBUG_MODULE_MODBUS, "=== INPUT REGISTERS REQUEST RECEIVED ===");
  
  uint16_t Register;
  word Count;
  Register=ResponseBufferGetAt(0);
  Count= ResponseBufferGetAt(1);
  
  // Small delay to ensure request is fully received
  delay(0);
  
  // Clear transmit buffer before filling
  for(int i = 0; i < 64; i++) {
    TransmitBufferPutAt(i, 0);
  }
      
  DEBUG_INFO_INT(DEBUG_MODULE_MODBUS, "readInputRegisters", Register);
  DEBUG_INFO_INT(DEBUG_MODULE_MODBUS, "Count", Count);
      
  // Handle multiple registers if Count > 1
  for(int i = 0; i < Count; i++) {
        word value = 0;
        uint16_t currentRegister = Register + i;
        
        // Map register addresses to internal getter methods
        switch(currentRegister) {
          case 39321: // Actual room temperature (Arduino sensor)
            value = (word)(pParentThermostat->getTemp() * 10); // Scale float to integer (22.5°C → 225)
            break;
            
          case 39322: // Actual humidity (Arduino sensor)
            value = ((ZLFP10Thermostat*)pParentThermostat)->getActualHumidity();
            break;
            
          case 46801: // FCU room temperature
            value = ((ZLFP10Thermostat*)pParentThermostat)->getFCURoomTemp();
            break;
            
          case 46802: // FCU coil temperature
            value = ((ZLFP10Thermostat*)pParentThermostat)->getCoilTemp();
            break;
            
          case 46803: // FCU fan setting
            value = ((ZLFP10Thermostat*)pParentThermostat)->getFanSetting();
            break;
            
          case 46804: // FCU fan RPM
            value = ((ZLFP10Thermostat*)pParentThermostat)->getFanRPM();
            break;
            
          case 46805: // FCU valve open status
            value = ((ZLFP10Thermostat*)pParentThermostat)->getValveOpen();
            break;
            
          case 46806: // Reserved/Unused register
            value = 0;
            break;
            
          case 46807: // Reserved/Unused register
            value = 0;
            break;
            
          case 46808: // FCU fan fault
            value = ((ZLFP10Thermostat*)pParentThermostat)->getFanFault();
            break;
            
          case 46809: // FCU temperature fault
            value = ((ZLFP10Thermostat*)pParentThermostat)->getTempFault();
            break;
            
          case 46810: // FCU coil temperature fault
            value = ((ZLFP10Thermostat*)pParentThermostat)->getCoilTempFault();
            break;
            
          default:
            value = 9999;
            break;
        }
        
    TransmitBufferPutAt(i, value);
  }
   
  // Add delay to prevent frame overlap and ensure stable communication
  delay(0);

  // Validate we're sending the correct function code
  if(ReceivedFunctionCode() != MODBUS04_ReadInputRegisters) {
    DEBUG_INFO(DEBUG_MODULE_MODBUS, "ERROR: Function code mismatch!");
    return;
  }
      

        // Ensure we have valid data before sending
  if(Count == 0 || Count > 64) {
    DEBUG_INFO(DEBUG_MODULE_MODBUS, "ERROR: Invalid count value!");
    return;
  }
  
  // Validate register range (using unsigned comparison to avoid overflow)
  if(Register < 39321U || Register > 46810U) {
    DEBUG_INFO_INT(DEBUG_MODULE_MODBUS, "ERROR: Invalid register range", Register);
    return;
  }
      
  uint8_t result = SendFrame(MODBUS04_ReadInputRegisters, ReceivedAddress(), Count, Register, false);
      
  DEBUG_INFO(DEBUG_MODULE_MODBUS, "=== END RESPONSE DEBUG ===");
      
  // Add additional delay after sending to ensure complete transmission
  delay(0);
}


void  ZLFP10ModbusServer::writeMultipleRegisters()
{
  // Parse according to observed buffer layout
  uint16_t Register = ResponseBufferGetAt(0);
  uint16_t Count = ResponseBufferGetAt(1);

  Serial.println();
  Serial.print("Writing multiple registers. Register: 0x");
  Serial.print(Register, HEX);
  Serial.print(" Count: ");
  Serial.print(Count);
  
  // Data starts at index 2
  for (int i = 0; i < Count; i++) {
    uint16_t value = ResponseBufferGetAt(2 + i);
    Serial.print(" Register ");
    Serial.print(Register + i);
    Serial.print(" Value: 0x");
    Serial.println(value, HEX);
    pClient->TransmitBufferPutAt(i, value);
  }

  // Debug: Print what ModbusFrame actually parsed
  Serial.print("DEBUG: Unit ID: 0x");
  Serial.println(ReceivedAddress(), HEX);
  Serial.print("DEBUG: Register: 0x");
  Serial.println(Register, HEX);
  Serial.print("DEBUG: Count: ");
  Serial.println(Count);

  // Forward the command to the FCU
  pClient->writeMultipleRegisters(Register, Count);

  Serial.println("DEBUG: FCU command sent");

  SendFrame(MODBUS10_WriteMultipleRegisters, ReceivedAddress(), Count, Register, false);
  Serial.println();
}

void ZLFP10ModbusServer::IllegalFunction()
{

}

void ZLFP10ModbusServer::SetupClient(ModbusClient* p_pClient)
{
  pClient=p_pClient;
}

void ZLFP10ModbusServer::SetParentThermostat(MultiStageThermostat * p_pParentThermostat)
{
  pParentThermostat=p_pParentThermostat;
}