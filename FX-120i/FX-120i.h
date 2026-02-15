/*
  ADX-FX.h - Library for interfacing with an ADL FX-120i model lab scale.
  Created by Damon Cali, February 9, 2018.
*/

#ifndef FX_120I_H
#define FX_120I_H

class FX_120i {
  public:
    FX_120i();
    void  begin(Stream &stream);
    void  reZero();
    void  on();
    void  off();
    char *readIdNumber();
    char *readSerialNumber();
    char *readModelName();
    float readTareWeight();
    float readImmediateWeight();
    char responseBuffer[18]; // 17 in data format + null terminator
  private:
    void  processStringResponse(uint8_t length);
    float processFloatResponse();
    const char *_cancelCommand;           //= "C";
    const char *_immediateWeightCommand;  //= "Q";
    const char *_stableWeightCommand;     //= "S";
    const char *_continuousWeightCommand; //= "SIR";
    const char *_calCommand;              //= "CAL";
    const char *_onCommand;               //= "ON";
    const char *_offCommand;              //= "OFF";
    const char *_powerCommand;            //= "P";
    const char *_printCommand;            //= "PRT";
    const char *_reZeroCommand;           //= "R";
    const char *_sampleCommand;           //= "SMP";
    const char *_tareCommand;             //= "T";
    const char *_modeCommand;             //= "U";
    const char *_idNumberCommand;         //= "?ID";
    const char *_serialNumberCommand;     //= "?SN";
    const char *_modelNameCommand;        //= "?TN";
    const char *_tareWeightCommand;       //= "?PT";
    Stream *_stream;
};

#endif