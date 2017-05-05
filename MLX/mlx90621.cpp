// This library is public domain. Whee!!!
// Avery Bodenstein (averybod@gmail.com)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
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
  // begin i2c interface
  _adapter_nr = 1;
  _I2C = -1;
  snprintf(_i2cFilename,19,"/dev/i2c-%d",_adapter_nr);
  initI2C();
  // close i2c interface
  closeI2C();
  // POR
  
  // wait 5ms
  usleep(5000);
  // read EEPROM table
  printf("Reading EEPROM...\n");
  readEEPROM(_EEPROM_Data);
  // only use configParam from EEPROM if no user supplied value.
  if (_configParam == 0){
    _configParam = (_EEPROM_Data[246]<<8) | _EEPROM_Data[245];
  }
  // write oscillator trim value into 0x93
  //printf("Writing Oscillator trim value...\n");
  writeData(0x04,_osc_trim,0xAA);
  //printf("Done.\n");
  setConfig(_configParam);
  // set Brown Out flag to 1 (0x92B10)
}
// open i2c interface
int MLX90621::initI2C(void){
  // check if i2c is already open
  if(_I2C < 0){
    _I2C = open(_i2cFilename, O_RDWR);
    if (_I2C<0){
      printf("Failed to initialize I2C Bus\n");
      return 0;
    }
  }
  return _I2C;
}
// close i2c interface
void MLX90621::closeI2C(void){
  // check if i2c is open
  if(_I2C > 0){
    close(_I2C);
    _I2C = -1;
  }
}
// change configuration parameter
uint8_t MLX90621::setConfig(uint16_t configParam){
  int i;
  // initialize I2C
  initI2C();
  // write configuration parameter to 0x92
  //printf("Writing Configuration Parameter...\n");
  writeData(0x03,_configParam,0x55);
  //printf("Done.\n");
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
  _Ks_s = _EEPROM_Data[0xC0];
  int8_t Ks4_EE = _EEPROM_Data[0xC4];
  _A_com = (_EEPROM_Data[0xD1]<<8) | _EEPROM_Data[0xD0];
  int16_t A_cp_EE = (_EEPROM_Data[0xD4]<<8) | _EEPROM_Data[0xD3];
  int8_t B_cp_EE = _EEPROM_Data[0xD5];
  uint16_t alpha_cp_EE = (_EEPROM_Data[0xD7]<<8) | _EEPROM_Data[0xD6];
  int8_t TGC_EE = _EEPROM_Data[0xD8];
  _dAs = (_EEPROM_Data[0xD9] & 0xF0) >> 4;
  _Bs = _EEPROM_Data[0xD9] & 0x0F;
  _alpha0 = (_EEPROM_Data[0xE1]<<8) | _EEPROM_Data[0xE0];
  _alpha0_s = _EEPROM_Data[0xE2];
  _dalpha_s = _EEPROM_Data[0xE3];
  uint16_t emissivity_EE = (_EEPROM_Data[0xE5]<<8) | _EEPROM_Data[0xE6];
  int16_t ksta_EE = (_EEPROM_Data[0xE7]<<8) | _EEPROM_Data[0xE6];
  // convert 2s compliment to unsigned
  _K_t1 = uint16_t(_K_t1);
  _K_t2 = uint16_t(_K_t2);
  _V_th = uint16_t(_V_th);
  // extract ADC resolution for clarity
  uint8_t adcRes = ((_configParam & 48) >> 4);
  //printf("ADC_Res = %d\n",adcRes);
  // compensate Ta parameters based on config parameter
  _A_cp = double(A_cp_EE)/pow(2,(3 - adcRes));
  _B_cp = double(B_cp_EE)/pow(2,(_Bs + 3 - adcRes));
  _V_th_c = double(_V_th) / pow(2,3-adcRes);
  _K_t1_c = double(_K_t1) / pow(2,(_K_ts & 240) >> 4);
  _K_t1_c = _K_t1_c / pow(2,3-adcRes);
  _K_t2_c = double(_K_t2) / pow(2,10 + (_K_ts & 15));
  _K_t2_c = _K_t2_c / pow(2,3-adcRes);
  // compensate To parameters based on config parameter
  _alpha_cp = double(alpha_cp_EE)/pow(2,_alpha0_s+3-adcRes);
  _TGC = double(TGC_EE)/32.0;
  _emissivity = double(emissivity_EE/32768.0);
  _ksta = double(ksta_EE)/pow(2,20);
  _Ks4 = double(Ks4_EE)/pow(2,(_Ks_s+8));
  for(i=0;i<64;i++){
    _Ai[i] = (double(_A_com + _dA[i]) * pow(2,_dAs))/pow(2,(3 - adcRes));
    _Bi[i] = double(_B[i])/pow(2,(_Bs + 3 - adcRes));
    _alpha[i] = ((double(_alpha0)/pow(2,_alpha0_s))+(double(_dalpha[i])/pow(2,_dalpha_s)))/pow(2,(3 - adcRes));
  }
  // close I2C interface
  closeI2C();
}
/*
 * Calculation Functions
*/
// calculates actual cold junction temp from raw data
double MLX90621::calcTa(uint16_t rawTemp) {
  double Ta = 0;
  double sqrRt = pow(pow(_K_t1_c,2) - 4.0 * _K_t2_c * (_V_th_c-double(rawTemp)),0.5);
  Ta = ((-_K_t1_c+sqrRt)/(2.0*_K_t2_c))+25.0;
  _Ta = Ta;
  /*
  printf("Vth = %g\n",_V_th_c);
  printf("Kt1 = %g\n",_K_t1_c);
  printf("Kt2 = %g\n",_K_t2_c);
  printf("Ta = %g\n",_Ta);
  */
  return Ta;
}
// calculates temperature seen by single pixel
double MLX90621::calcTo(int16_t rawTemp, uint8_t loc){
    // Calculate Compensated IR signal
    // Calculate compensation pixel value
    int8_t V_cp = -36; // TEMPORARY
    double V_ir_cp_c = double(V_cp) - (_A_cp + _B_cp * (_Ta-25.0));
    // compensate for offset
    double V_ir_c = rawTemp - (_Ai[loc] + _Bi[loc] * (_Ta - 25.0) );
    // compensate for thermal gradient
    V_ir_c = V_ir_c - _TGC * V_ir_cp_c;
    // compensate for emissivity
    V_ir_c = V_ir_c / _emissivity;
    // Calculate compensated alpha
    double alpha_c = (1.0 + _ksta*(_Ta-25.0)) * (_alpha[loc] - _TGC*_alpha_cp);
    // calculate Sx
    double Tak = pow(_Ta+273.15,4);
    double Sx = double(_Ks4) * pow(pow(alpha_c,3)*V_ir_c+pow(alpha_c,4)*Tak,0.25);
    // calculate final temperature
    double To = pow((V_ir_c/(alpha_c*(1-_Ks4*273.15)+Sx)) + Tak,0.25) - 273.15; 

    /*
    printf("Vir = %d\n",rawTemp);
    printf("Ta = %g\n",_Ta);
    printf("dA = %d\n",_dA[loc]);
    printf("B = %d\n",_B[loc]);
    printf("Bi = %g\n",_Bi[loc]);
    printf("Ai = %g\n",_Ai[loc]);
    printf("dalpha = %d\n",_dalpha[loc]);
    printf("Ks_s = %d\n",_Ks_s);
    printf("Ks4 = %g\n",_Ks4);
    printf("A_com = %d\n",_A_com);
    printf("A_cp = %g\n",_A_cp);
    printf("B_cp = %g\n",_B_cp);
    printf("alpha_cp = %g\n",_alpha_cp);
    printf("TGC = %g\n",_TGC);
    printf("dAs = %d\n",_dAs);
    printf("Bs = %d\n",_Bs);
    printf("alpha_0 = %d\n",_alpha0);
    printf("alpha_0_s = %d\n",_alpha0_s);
    printf("dalpha_s = %d\n",_dalpha_s);
    printf("emissivity = %g\n",_emissivity);
    printf("ksta = %g\n",_ksta);
    printf("V_ir_c = %g\n",V_ir_c);
    printf("alpha=%g\n",_alpha[loc]);
    printf("alpha_c = %g\n",alpha_c);
    printf("Tak = %g\n",Tak);
    printf("Sx=%g\n",Sx);
    printf("To=%g\n",To);
    */
}
/*
 * Data Interaction functions
*/
// output frame as text file
void MLX90621::exportText(double dataBuf[64], char *fileName){
  int i,j;
  int ret = -1;
  char buf[6];
  // open file
  int outFile = open(fileName,O_WRONLY|O_CREAT,S_IRWXO);
  // check if file is actually open
  if(outFile<0)
      printf("can't open output file");
  // write frame data to file
  for(j=0;j<16;j++){
      for(i=0;i<4;i++){
          sprintf(buf,"%0.2f,",dataBuf[4*j+i]);
          write(outFile,buf,6);
      }
      write(outFile,"\n",1);
  }
  // close file
  ret = close(outFile);
  // check if file actually closed
  if (ret == -1)
      printf("failed to close output file");
  return;
}
// reads cold junction temp from MLX
uint16_t MLX90621::readTamb(){
  printf("Reading T_amb\n");
  uint16_t T_amb;
  // buffers
  uint8_t inbuf[2];
  uint8_t outbuf[4];
  // message structs
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];
  // construct output message
  outbuf[0] = 0x02;
  outbuf[1] = 0x40;
  outbuf[2] = 0x00;
  outbuf[3] = 0x01;
  // begin i2c interface
  initI2C();
  // output struct
  messages[0].addr = MLX_ADDR;
  messages[0].flags = 0;
  messages[0].len = sizeof(outbuf);
  messages[0].buf = outbuf;

  // output struct
  messages[1].addr = MLX_ADDR;
  messages[1].flags = I2C_M_RD/* | I2C_M_NOSTARTi*/;
  messages[1].len = sizeof(inbuf);
  messages[1].buf = inbuf;

  // send request to kernel
  packets.msgs = messages;
  packets.nmsgs = 2;
  if(ioctl(_I2C, I2C_RDWR, &packets) < 0){
    // unable to send data
    printf("Unable to send data\n");
    return 0;
  }

  // LSB first
  T_amb = inbuf[0];
  T_amb = T_amb | (inbuf[1] << 8);
  // close I2C interface
  closeI2C();
  // return ambient temp
  return T_amb;
}
// read EEPROM
void MLX90621::readEEPROM(uint8_t dataBuf[EEPROM_SIZE]) {
  printf("in readEEPROM\n");
  // begin i2c interface
  initI2C();
  // read EEPROM table
  uint16_t i,j;
  // write address to begin reading at
  printf("Setting Slave Address\n");
  // use EEPROM address
  if (ioctl(_I2C, I2C_SLAVE, EEPROM_ADDR) < 0){
    // could not set device as slave
    printf("Could not find device\n");
  }
  // recieve bytes and write into recieve buffer
  printf("Reading Data\n");
  dataBuf[0] = 0x00;
  if (write(_I2C, dataBuf, 1) != 1){
    // write failed
    printf("Write Failed\n");
  }
  printf("Reading Data\n");
  if (read(_I2C, dataBuf, EEPROM_SIZE) != EEPROM_SIZE){
    // read failed
    printf("Read Failed\n");
  }
  // output table for debugging
  /*
  printf("EEPROM Contents:\n");
  for(j=0;j<32;j++){
    for(i=0;i<8;i++){
      printf("%X,",dataBuf[8*j+i]);
    }
    printf("\n");
  }
  */
  //printf("Done Reading\n");
  // close I2C interface
  closeI2C();
}
// full frame read
void MLX90621::readFrame(int16_t dataBuf[64]){
  uint8_t i;
  int16_t temp;
  // message structs
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];
  // buffers
  uint8_t inBuf[128];
  uint8_t outBuf[4];
  // initialize I2C interface
  initI2C();
  // construct output message
  outBuf[0] = 0x02; // command
  outBuf[1] = 0x00; // start address
  outBuf[2] = 0x01; // address step
  outBuf[3] = 0x40; // number of reads

  // output struct
  messages[0].addr = MLX_ADDR;
  messages[0].flags = 0;
  messages[0].len = sizeof(outBuf);
  messages[0].buf = outBuf;

  // input struct
  messages[1].addr = MLX_ADDR;
  messages[1].flags = I2C_M_RD/* | I2C_M_NOSTARTi*/;
  messages[1].len = sizeof(inBuf);
  messages[1].buf = inBuf;

  // send request to kernel
  packets.msgs = messages;
  packets.nmsgs = 2;
  if(ioctl(_I2C, I2C_RDWR, &packets) < 0){
    // unable to send data
    printf("Unable to send data\n");
    return;
  }

  // join 8 bit transactions into 16 bit values
  for(i=0;i<64;i++){
    temp = inBuf[2*i];
    temp = temp | (inBuf[2*i+1] << 8);
    dataBuf[i] = temp;
  }
  // close I2C interface
  closeI2C();
}

// write command to MLX sensor
void MLX90621::writeCmd(uint8_t cmd, uint8_t offset, uint8_t ad_step, uint8_t nReads) {
  // begin i2c interface
  initI2C();
  // use MLX address
  if (ioctl(_I2C, I2C_SLAVE, MLX_ADDR) < 0){
    // could not set device as slave
    printf("Could not find device\n");
  }
  // write command
  uint8_t buf[4];
  buf[0] = cmd;
  buf[1] = offset;
  buf[2] = ad_step;
  buf[3] = nReads;
  if(write(_I2C, buf, 4) != 4){
    // write failed
    printf("Write Failed\n");
  }
}
// write data to MLX sensor
void MLX90621::writeData(uint8_t cmd, uint16_t data, uint8_t check) {
  // initialize i2c interface
  initI2C();
  // use MLX address
  if (ioctl(_I2C, I2C_SLAVE, MLX_ADDR) < 0){
    // could not set device as slave
    printf("Could not find device\n");
  }
  // write command
  uint8_t buf[5];
  buf[0] = cmd;
  buf[1] = (data&0xff)-check;
  buf[2] = data&0xff;
  buf[3] = (data>>8)-check;
  buf[4] = data>>8;
  if(write(_I2C, buf, 5) != 5){
    // write failed
    printf("Write Failed\n");
  }
}


