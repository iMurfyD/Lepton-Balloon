// This library is public domain. Whee!!!
// Avery Bodenstein

#ifndef MLX90621_H

#define MLX90621_H
#include <stdint.h>

#define EEPROM_SIZE 256
#define EEPROM_ADDR 0x50
#define MLX_ADDR 0x60
#define MLXWIDTH 4
#define MLXHEIGHT 16

class MLX90621 {
 public:
  MLX90621(void);
  MLX90621(uint16_t configParam);
  uint8_t setConfig(uint16_t configParam);
  
  uint8_t init(void);
  uint16_t readTamb(void);
  double calcTa(uint16_t rawTemp);
  double calcTo(int16_t rawTemp, uint8_t loc);
  // export frame as text
  void exportText(double dataBuf[64], char *fileName);
  // export frame as png
  void exportPng(double dataBuf[64], char *fileName);
  // read EEPROM data
  void readEEPROM(uint8_t dataBuf[64]);
  // full frame read
  void readFrame(int16_t dataBuf[64]);
  // write command to MLX
  void writeCmd(uint8_t cmd, uint8_t offset, uint8_t ad_step, uint8_t nReads);
  // write data to MLX
  void writeData(uint8_t cmd, uint16_t data, uint8_t check);
  
 private:
  // file info
  int _I2C;
  int _adapter_nr;
  char _i2cFilename[20];
  //uint8_t _adcRes;
  double _Ta;
  int8_t _brownOut,_osc_trim,_error;
  uint8_t _EEPROM_Data[EEPROM_SIZE] = {};
  uint8_t _dA[64] = {};
  uint8_t _B[64] = {};
  uint8_t _dalpha[64] = {};
  uint16_t _configParam;
  // Ta calculation parameters
  uint8_t _K_ts;
  int16_t _K_t1,_K_t2,_V_th;
  double _K_t1_c, _K_t2_c, _V_th_c;
  // To calculation parameters
  uint8_t _Ks_s;
  int16_t _A_com;
  uint8_t _dAs, _Bs;
  uint16_t _alpha0;
  uint8_t _alpha0_s, _dalpha_s;

  double _Ai[64] = {};
  double _Bi[64] = {};
  double _alpha[64] = {};
  double _A_cp, _B_cp, _TGC, _ksta, _Ks4, _emissivity, _alpha_cp;
  //uint16_t _dalpha_cp;
  
  // open I2C interface
  int initI2C(void);
  // close I2C interface
  void closeI2C(void);
  
};
#endif
