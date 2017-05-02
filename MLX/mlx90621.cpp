// This library is public domain. Whee!!!
// Avery Bodenstein (averybod@gmail.com)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <bcm2835.h>
#include "mlx90621.h"

/*
 * Initialization funtions
*/
// instantiate MLX with default parameters
MLX90621::MLX90621(void) {
  _configParam = 0;
  _error = 0;
}
// instantiate MLX with user parameters
MLX90621::MLX90621(uint16_t configParam) {
  _configParam = configParam;
  _error = 0;
}
// initialize MLX sensor
uint8_t MLX90621::init(void) {
  uint8_t i;
  // begin serial interface
  printf("Initializing MLX90621\n");
  // begin wire interface
  if (!bcm2835_init()){
    printf("Failed to initialize I2C Bus\n");
    return 0;
  }
  bcm2835_i2c_begin();
  bcm2835_i2c_set_baudrate(400000);
  // POR
  
  // wait 5ms
  usleep(5000);
  // check connection
  //Wire.beginTransmission(EEPROM_ADDR);
  //Wire.write(0);
  //_error = Wire.endTransmission();
  // check for error
  //checkError(_error);
  // read EEPROM table
  printf("Reading EEPROM...\n");
  readEEPROM(_EEPROM_Data);
  // store calibration constants
  for(i=0;i<64;i++){
    _dA[i] = _EEPROM_Data[i];
    _B[i] = _EEPROM_Data[i+64];
    _dalpha[i] = _EEPROM_Data[i+128];
  }
  _osc_trim = _EEPROM_Data[247];
  // Ta compensation values
  _K_t1 = (_EEPROM_Data[219]<<8) | _EEPROM_Data[218];
  _K_t2 = (_EEPROM_Data[221]<<8) | _EEPROM_Data[220];
  _V_th = (_EEPROM_Data[223]<<8) | _EEPROM_Data[222];
  _K_ts = _EEPROM_Data[210];
  // To compensation values
  _A_com = (_EEPROM_Data[209]<<8) | _EEPROM_Data[208];
  _dAs = (_EEPROM_Data[217] & 0xF0) >> 4;
  _dBs = _EEPROM_Data[217] & 0x0F;
  _emissivity = (_EEPROM_Data[229]<<8) | _EEPROM_Data[228];
  _dalpha_cp = (_EEPROM_Data[215]<<8) | _EEPROM_Data[214];
  _TGC = _EEPROM_Data[216];
  _alpha0 = (_EEPROM_Data[225]<<8) | _EEPROM_Data[224];
  _alpha0_s = _EEPROM_Data[226];
  _dalpha_s = _EEPROM_Data[227];
  _ksta = (_EEPROM_Data[231]<<8) | _EEPROM_Data[230];
  // convert 2s compliment to unsigned
  _K_t1 = uint16_t(_K_t1);
  _K_t2 = uint16_t(_K_t2);
  _V_th = uint16_t(_V_th);
  //printf("Unsigned Ta Values:\n");
  //Serial.print(_K_t1);Serial.print(',');Serial.print(_K_t2);Serial.print(',');Serial.println(_V_th);
  // only use configParam from EEPROM if no user supplied value.
  if (_configParam == 0){
    _configParam = (_EEPROM_Data[246]<<8) | _EEPROM_Data[245];
  }
  // write oscillator trim value into 0x93
  printf("Writing Oscillator trim value...\n");
  writeData(0x04,_osc_trim,0xAA);
  printf("Done.\n");
  setConfig(_configParam);
  // set Brown Out flag to 1 (0x92B10)
}
// change configuration parameter
uint8_t MLX90621::setConfig(uint16_t configParam){
  // write configuration parameter to 0x92
  printf("Writing Configuration Parameter...\n");
  writeData(0x03,_configParam,0x55);
  printf("Done.\n");
  // extract ADC resolution for clarity
  uint8_t adcRes = ((_configParam & 48) >> 4);
  // compensate Ta parameters based on config parameter
  _V_th_c = double(_V_th) / (1 << (3 - adcRes));
  _K_t1_c = double(_K_t1) / (1 << ((_K_ts & 240) >> 4));
  _K_t1_c = _K_t1_c / (1 << (3 - adcRes));
  _K_t2_c = double(_K_t2) / (1L << (10 + (_K_ts & 15)));
  _K_t2_c = _K_t2_c / (1 << (3 - adcRes));
  // compensate To parameters based on config parameter
  _ksta = _ksta/(1L << 20);
  int8_t ks4ee = _EEPROM_Data[196];
  uint8_t ks_s = _EEPROM_Data[192] & 0x0F;
  _Ks4 = ks4ee/(1L << (ks_s+8));
  uint8_t i = 0;
  for(i=0;i<64;i++){
    _Ai[i] = ((_A_com + _dA[i]) * (1 << _dAs))/(1 << (3 - adcRes));
    _Bi[i] = (_B[i])/(1 << (_dBs + 3 - adcRes));
    _alpha[i] = ((_alpha0/(1<<_alpha0_s))+(_dalpha[i]/(1<<_dalpha_s)))/(1 << (3 - adcRes));
  }
}
/*
 * Calculation Functions
*/
// calculates actual cold junction temp from raw data
double MLX90621::calcTa(uint16_t rawTemp) {
  double Ta = 0;
  double sqrRt = pow(pow(_K_t1_c,2) - 4.0 * _K_t2_c * (_V_th_c-rawTemp),0.5);
  Ta = ((-_K_t1_c+sqrRt)/(2.0*_K_t2_c))+25.0;
  _Ta = Ta;
  return Ta;
}
// calculates temperature seen by single pixel
double MLX90621::calcTo(uint16_t rawTemp, uint8_t loc){
    // Calculate Compensated IR signal
    // compensate for offset
    double V_ir_c = rawTemp - (_Ai[loc] + _Bi[loc] * (_Ta - 25.0) );
    // compensate for thermal gradient
    V_ir_c = V_ir_c - (double(_TGC) / 32.0);
    // compensate for emissivity
    V_ir_c = V_ir_c / _emissivity;
    printf("V_ir_c\n");
    //Serial.println(V_ir_c);
    // Calculate compensated alpha
    double alpha_c = (1.0 + _ksta*(_Ta-25.0)) * (_alpha[loc] - _TGC*_alpha_cp);
    // calculate Sx
    double Tak = pow(_Ta+273.15,4);
    double Sx = _Ks4 * pow(pow(alpha_c,3)*V_ir_c+pow(alpha_c,4)*Tak,0.25);
    // calculate final temperature
    double To = pow(V_ir_c/(alpha_c*(1-_Ks4*273.15)+Sx),0.25) - 273.15; 
}
/*
 * Data Interaction functions
*/
// reads cold junction temp from MLX
uint16_t MLX90621::readTamb(){
  uint16_t T_amb;
  // write command to read ambient temp
  writeCmd(0x02,0x40,0x00,0x01);
  // request two data bytes
  //Wire.requestFrom(MLX_ADDR,2);
  // LSB first
  //uint16_t T_amb = Wire.read();
  //T_amb = T_amb | (Wire.read() << 8);
  // calculate true T_amb
  T_amb = calcTa(T_amb);
  // return ambient temp
  return T_amb;
}
// read EEPROM
void MLX90621::readEEPROM(uint8_t dataBuf[EEPROM_SIZE]) {
  printf("in readEEPROM\n");
  // read EEPROM table
  uint16_t i,j;
  // begin i2c interface
  //bcm2835_i2c_begin();
  // write address to begin reading at
  printf("Setting Slave Address\n");
  bcm2835_i2c_setSlaveAddress(EEPROM_ADDR);
  // recieve bytes and write into recieve buffer
  printf("Reading Data\n");
  uint8_t error = bcm2835_i2c_write((char *)0x00,1);
  printf("%i\n",error);
  printf("Reading Data\n");
  bcm2835_i2c_read((char *)dataBuf,EEPROM_SIZE);
  //bcm2835_i2c_write_read_rs((char *)0x00, 1,(char *)dataBuf, EEPROM_SIZE);
  // output table for debugging
  printf("EEPROM Contents:\n");
  for(j=0;j<16;j++){
    for(i=0;i<256;i++){
      printf("%X,",dataBuf[16*j+i]);
    }
    printf("\n");
  }
  printf("Done Reading\n");
}
// single column frame read
void MLX90621::readFrame_sc(uint16_t dataBuf[64]) {
  uint8_t column,i;
  uint16_t temp;
  for(column = 0;column<16;column++){
    // write command to read single column
    writeCmd(0x02,column*4,0x01,0x04);
    // request 8 bytes (single column)
    //Wire.requestFrom(MLX_ADDR,8);
    for(i=0;i<4;i++){
      //temp = Wire.read();
      //temp = temp | (Wire.read() << 8);
      //Serial.print(temp,HEX);Serial.print(',');
      //delay(2);
      dataBuf[4*column+i] = temp;
    }
    //Serial.println();
  }
}
// single row frame read
void MLX90621::readFrame_sl(uint16_t dataBuf[64]) {
  uint8_t row,i;
  uint16_t temp;
  for(row = 0;row<4;row++){
    // write command to read single row
    writeCmd(0x02,row*16,0x01,0x10);
    // request 32 bytes (single row)
    //Wire.requestFrom(MLX_ADDR,32);
    for(i=0;i<16;i++){
      //temp = Wire.read();
      //temp = temp | (Wire.read() << 8);
      //Serial.print(temp,HEX);Serial.print(',');
      //delay(2);
      dataBuf[16*row+i] = temp;
    }
    //Serial.println();
  }
}
// write command to MLX sensor
void MLX90621::writeCmd(uint8_t cmd, uint8_t offset, uint8_t ad_step, uint8_t nReads) {
  //Wire.beginTransmission(MLX_ADDR);
  //Wire.write(cmd); // command to read ambient temp
  //Wire.write(offset); // offset address
  //Wire.write(ad_step); // address step
  //Wire.write(nReads); // number of reads
  // stop transmission
  //_error = Wire.endTransmission(false);
  // check for error
  //checkError(_error);
}
// write data to MLX sensor
void MLX90621::writeData(uint8_t cmd, uint16_t data, uint8_t check) {
  //Wire.beginTransmission(MLX_ADDR);
  //Wire.write(cmd); // command
  //Wire.write((data&0xff)-check); // LSB check
  //Wire.write(data&0xff); // LSB
  //Wire.write((data>>8)-check); // MSB check
  //Wire.write(data>>8); // MSB
  // stop transmission
  //_error = Wire.endTransmission(true);
  // check for error
  //checkError(_error);
}

/*
 * General Utility functions
*/
/*
// decrypt I2C errors
void MLX90621::checkError(uint8_t error) {
  if(error){
    Serial.println("Error Establishing Connection With MLX");
    // decode error
    switch(error){
      case 1: Serial.println("Data too long");
              break;
      case 2: Serial.println("Recieved Nack on address");
              break;
      case 3: Serial.println("Recieved Nack on data");
              break;
      default: Serial.println("Unknown error");
    }
    // capture process
    while(1);
  }
}
*/

