#include <ModbusServer.h>
#include <ModbusClient.h>
#include <MultiStageThermostat.h>


#pragma once






class ZLFP10ModbusServer: public ModbusServer
{
  ModbusClient * pClient;
  MultiStageThermostat * pParentThermostat;
  public:
  void SetParentThermostat(MultiStageThermostat * pParentThermostat);
  void SetupClient(ModbusClient * p_pClient);
  void RequestReaction();

  // we're only going to support three Modbus actions, 3, 4 and 10
  private: 
    void  readHoldingRegisters(); //
    void  readInputRegisters();  //
    void  writeMultipleRegisters();//
    void IllegalFunction(); // called if any other is specified

};


