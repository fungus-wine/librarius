/*
  ADX-FX.h - Library for interfacing with an ADL FX-120i model lab scale.
  Created by Damon Cali, February 9, 2018.

  FX-120i Commands
  ==============================================
  C   - cancels S or SIR command
  Q   - requests the weight data immediately
  S   - requests the weight data when stabilized
  SIR - requests the weight data continuously

  CAL - same as CAL key
  OFF - turns display off
  ON  - turns display on
  P   - same as ON/OFF key
  PRT - same as PRINT key
  R   - same as RE-ZERO key
  SMP - same as SAMPLE key
  T   - Tare key
  U   - same as MODE key
  ?ID - requests ID number
  ?SN - requests serial number
  ?TN - requests model name
  ?PT - requests tare weight

  Scale sends data, an <AK> (ASCII 06h), or error code as response - see page 61 of instruction manual for details
*/

#include "Arduino.h"
#include "FX-120i.h"

FX_120i::FX_120i() {
  _cancelCommand           = "C";
  _immediateWeightCommand  = "Q";
  _stableWeightCommand     = "S";
  _continuousWeightCommand = "SIR";
  _calCommand              = "CAL"; //
  _onCommand               = "ON"; //
  _offCommand              = "OFF";
  _powerCommand            = "P";  //
  _printCommand            = "PRT";
  _reZeroCommand           = "R";  //
  _sampleCommand           = "SMP";
  _tareCommand             = "T";  //
  _modeCommand             = "U";
  _idNumberCommand         = "?ID";
  _serialNumberCommand     = "?SN";
  _modelNameCommand        = "?TN";
  _tareWeightCommand       = "?PT";
}

void FX_120i::begin(Stream &stream) {
  _stream = &stream;
}

void FX_120i::reZero() {
  _stream->println(_reZeroCommand);
  waitForAcknowledgement(2);
}

void FX_120i::on() {
  _stream->println(_onCommand);
  waitForAcknowledgement(2);
}

void FX_120i::off() {
  _stream->println(_offCommand);
  waitForAcknowledgement(1);
}

char *FX_120i::readIdNumber() {
  _stream->println(_idNumberCommand);
  processStringResponse(18);
  return responseBuffer;
}

char *FX_120i::readSerialNumber() {
  _stream->println(_serialNumberCommand);
  processStringResponse(18);
  return responseBuffer;
}

char *FX_120i::readModelName() {
  _stream->println(_modelNameCommand);
  processStringResponse(18);
  return responseBuffer;
}

float FX_120i::readTareWeight() {
  _stream->println(_tareWeightCommand);
  return processFloatResponse();
}

float FX_120i::readImmediateWeight() {
  _stream->println(_immediateWeightCommand);
  return processFloatResponse();
}

// private

void FX_120i::waitForAcknowledgement(uint8_t numberSent) { // some commands receive two <AK> codes, others, just one
  while (_stream->available() < numberSent * 2 ) {
    ;
  }
  for (uint8_t i = 0; i < numberSent; ++i) {
    _stream->readBytesUntil('\n',responseBuffer, length);
  }
}

void FX_120i::processStringResponse(uint8_t length) {
  uint8_t num;
  while (_stream->available() < length) {
    ;
  }
  num = _stream->readBytesUntil('\n',responseBuffer, length); // returns number of characters read
  responseBuffer[num - 1] = '\0';  // command repsonses end in \r\n, so we replace \r with \0
}

float FX_120i::processFloatResponse() {
  while (_stream->available() < 1) {
    ;
  }
  return _stream->parseFloat();
}