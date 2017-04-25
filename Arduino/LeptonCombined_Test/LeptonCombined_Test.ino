#include<SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

// Pin definitions
#define CS 10
#define LEDPIN 4
#define DISP_CS 9
#define DISP_DC 8
#define DISP_RST 7

//VoSPI packet sizes
#define PACKET_LEN 164
#define FRAME_PACKETS 60

// I2C address
#define ADDRESS  (0x2A)
// Subsystem command addresses
#define AGC (0x01)
#define SYS (0x02)
#define VID (0x03)
#define OEM (0x08)
// command type definitions
#define GET (0x00)
#define SET (0x01)
#define RUN (0x02)

// dummy variables used to tell SPI.transfer how many bytes to read at once
uint16_t read16 = 0;
uint8_t read8 = 0;

// Color lookup table

const int colormap[] = {255,4,0,255,8,0,255,12,0,255,16,0,255,20,0,255,24,0,255,28,0,255,32,0,255,36,0,255,40,0,255,44,0,255,48,0,255,52,0,255,56,0,255,60,0,255,64,0,255,68,0,255,72,0,255,76,0,255,80,0,255,84,0,255,88,0,255,92,0,255,96,0,255,100,0,255,104,0,
255,108,0,255,112,0,255,116,0,255,120,0,255,124,0,255,128,0,255,132,0,255,136,0,255,140,0,255,144,0,255,148,0,255,152,0,255,156,0,255,160,0,255,164,0,255,168,0,255,172,0,255,176,0,255,180,0,255,184,0,255,188,0,255,192,0,255,196,0,255,200,0,255,204,0,255,208,
0,255,212,0,255,216,0,255,220,0,255,224,0,255,228,0,255,232,0,255,236,0,255,240,0,255,244,0,255,248,0,255,252,0,252,255,0,248,255,0,244,255,0,240,255,0,236,255,0,232,255,0,228,255,0,224,255,0,220,255,0,216,255,0,212,255,0,208,255,0,204,255,0,200,255,0,196,255,
0,192,255,0,188,255,0,184,255,0,180,255,0,176,255,0,172,255,0,168,255,0,164,255,0,160,255,0,156,255,0,152,255,0,148,255,0,144,255,0,140,255,0,136,255,0,132,255,0,128,255,0,124,255,0,120,255,0,116,255,0,112,255,0,108,255,0,104,255,0,100,255,0,96,255,0,92,255,0,
88,255,0,84,255,0,80,255,0,76,255,0,72,255,0,68,255,0,64,255,0,60,255,0,56,255,0,52,255,0,48,255,0,44,255,0,40,255,0,36,255,0,32,255,0,28,255,0,24,255,0,20,255,0,16,255,0,12,255,0,8,255,0,4,255,0,0,255,0,0,255,4,0,255,8,0,255,12,0,255,16,0,255,20,0,255,24,0,
255,28,0,255,32,0,255,36,0,255,40,0,255,44,0,255,48,0,255,52,0,255,56,0,255,60,0,255,64,0,255,68,0,255,72,0,255,76,0,255,80,0,255,84,0,255,88,0,255,92,0,255,96,0,255,100,0,255,104,0,255,108,0,255,112,0,255,116,0,255,120,0,255,124,0,255,128,0,255,132,0,255,136,
0,255,140,0,255,144,0,255,148,0,255,152,0,255,156,0,255,160,0,255,164,0,255,168,0,255,172,0,255,176,0,255,180,0,255,184,0,255,188,0,255,192,0,255,196,0,255,200,0,255,204,0,255,208,0,255,212,0,255,216,0,255,220,0,255,224,0,255,228,0,255,232,0,255,236,0,255,240,
0,255,244,0,255,248,0,255,252,0,252,255,0,248,255,0,244,255,0,240,255,0,236,255,0,232,255,0,228,255,0,224,255,0,220,255,0,216,255,0,212,255,0,208,255,0,204,255,0,200,255,0,196,255,0,192,255,0,188,255,0,184,255,0,180,255,0,176,255,0,172,255,0,168,255,0,164,255,
0,160,255,0,156,255,0,152,255,0,148,255,0,144,255,0,140,255,0,136,255,0,132,255,0,128,255,0,124,255,0,120,255,0,116,255,0,112,255,0,108,255,0,104,255,0,100,255,0,96,255,0,92,255,0,88,255,0,84,255,0,80,255,0,76,255,0,72,255,0,68,255,0,64,255,0,60,255,0,56,255,0,
52,255,0,48,255,0,44,255,0,40,255,0,36,255,0,32,255,0,28,255,0,24,255,0,20,255,0,16,255,0,12,255,0,8,255,0,4,255,0,0,255};
/*
const int colormap[] = {255, 255, 255, 253, 253, 253, 251, 251, 251, 249, 249, 249, 247, 247, 247, 245, 245, 245, 243, 243, 243, 241, 241,
241, 239, 239, 239, 237, 237, 237, 235, 235, 235, 233, 233, 233, 231, 231, 231, 229, 229, 229, 227, 227, 227, 225, 225, 225, 223, 223, 223,
221, 221, 221, 219, 219, 219, 217, 217, 217, 215, 215, 215, 213, 213, 213, 211, 211, 211, 209, 209, 209, 207, 207, 207, 205, 205, 205, 203,
203, 203, 201, 201, 201, 199, 199, 199, 197, 197, 197, 195, 195, 195, 193, 193, 193, 191, 191, 191, 189, 189, 189, 187, 187, 187, 185, 185,
185, 183, 183, 183, 181, 181, 181, 179, 179, 179, 177, 177, 177, 175, 175, 175, 173, 173, 173, 171, 171, 171, 169, 169, 169, 167, 167, 167,
165, 165, 165, 163, 163, 163, 161, 161, 161, 159, 159, 159, 157, 157, 157, 155, 155, 155, 153, 153, 153, 151, 151, 151, 149, 149, 149, 147,
147, 147, 145, 145, 145, 143, 143, 143, 141, 141, 141, 139, 139, 139, 137, 137, 137, 135, 135, 135, 133, 133, 133, 131, 131, 131, 129, 129,
129, 126, 126, 126, 124, 124, 124, 122, 122, 122, 120, 120, 120, 118, 118, 118, 116, 116, 116, 114, 114, 114, 112, 112, 112, 110, 110, 110,
108, 108, 108, 106, 106, 106, 104, 104, 104, 102, 102, 102, 100, 100, 100, 98, 98, 98, 96, 96, 96, 94, 94, 94, 92, 92, 92, 90, 90, 90, 88,
88, 88, 86, 86, 86, 84, 84, 84, 82, 82, 82, 80, 80, 80, 78, 78, 78, 76, 76, 76, 74, 74, 74, 72, 72, 72, 70, 70, 70, 68, 68, 68, 66, 66, 66,
64, 64, 64, 62, 62, 62, 60, 60, 60, 58, 58, 58, 56, 56, 56, 54, 54, 54, 52, 52, 52, 50, 50, 50, 48, 48, 48, 46, 46, 46, 44, 44, 44, 42, 42,
42, 40, 40, 40, 38, 38, 38, 36, 36, 36, 34, 34, 34, 32, 32, 32, 30, 30, 30, 28, 28, 28, 26, 26, 26, 24, 24, 24, 22, 22, 22, 20, 20, 20, 18,
18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10, 10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2, 0, 0, 0, 0, 0, 9, 2, 0, 16, 4, 0, 24, 6, 0, 31,
8, 0, 38, 10, 0, 45, 12, 0, 53, 14, 0, 60, 17, 0, 67, 19, 0, 74, 21, 0, 82, 23, 0, 89, 25, 0, 96, 27, 0, 103, 29, 0, 111, 31, 0, 118, 36,
0, 120, 41, 0, 121, 46, 0, 122, 51, 0, 123, 56, 0, 124, 61, 0, 125, 66, 0, 126, 71, 0, 127, 76, 1, 128, 81, 1, 129, 86, 1, 130, 91, 1, 131,
96, 1, 132, 101, 1, 133, 106, 1, 134, 111, 1, 135, 116, 1, 136, 121, 1, 136, 125, 2, 137, 130, 2, 137, 135, 3, 137, 139, 3, 138, 144, 3, 138,
149, 4, 138, 153, 4, 139, 158, 5, 139, 163, 5, 139, 167, 5, 140, 172, 6, 140, 177, 6, 140, 181, 7, 141, 186, 7, 141, 189, 10, 137, 191, 13,
132, 194, 16, 127, 196, 19, 121, 198, 22, 116, 200, 25, 111, 203, 28, 106, 205, 31, 101, 207, 34, 95, 209, 37, 90, 212, 40, 85, 214, 43, 80,
216, 46, 75, 218, 49, 69, 221, 52, 64, 223, 55, 59, 224, 57, 49, 225, 60, 47, 226, 64, 44, 227, 67, 42, 228, 71, 39, 229, 74, 37, 230, 78, 34,
231, 81, 32, 231, 85, 29, 232, 88, 27, 233, 92, 24, 234, 95, 22, 235, 99, 19, 236, 102, 17, 237, 106, 14, 238, 109, 12, 239, 112, 12, 240,
116, 12, 240, 119, 12, 241, 123, 12, 241, 127, 12, 242, 130, 12, 242, 134, 12, 243, 138, 12, 243, 141, 13, 244, 145, 13, 244, 149, 13, 245,
152, 13, 245, 156, 13, 246, 160, 13, 246, 163, 13, 247, 167, 13, 247, 171, 13, 248, 175, 14, 248, 178, 15, 249, 182, 16, 249, 185, 18, 250,
189, 19, 250, 192, 20, 251, 196, 21, 251, 199, 22, 252, 203, 23, 252, 206, 24, 253, 210, 25, 253, 213, 27, 254, 217, 28, 254, 220, 29, 255,
224, 30, 255, 227, 39, 255, 229, 53, 255, 231, 67, 255, 233, 81, 255, 234, 95, 255, 236, 109, 255, 238, 123, 255, 240, 137, 255, 242, 151,
255, 244, 165, 255, 246, 179, 255, 248, 193, 255, 249, 207, 255, 251, 221, 255, 253, 235, 255, 255, 24};
*/
// frame buffer
char frameBuffer[PACKET_LEN*FRAME_PACKETS];

// display
Adafruit_ILI9340 tft = Adafruit_ILI9340(DISP_CS, DISP_DC, DISP_RST);

void setup() {
  //  Begin serial monitor
  Serial.begin(115200);
  Serial.println("Lepton Demo...");
  delay(1000);

  Wire.begin();
  SPI.begin();
  pinMode(CS,OUTPUT);

  tft.begin();
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE3));

  Serial.println("setup complete");
  Serial.println("Disabling Lepton AGC...");
  
  // read status regsiter
  read_reg(0x2);
  
  Serial.println("AGC READ");
  lepton_command(AGC, 0x00  , GET);
  read_data();

  Serial.println("AGC DISABLE");
  uint8_t agcDisable[2] = {0x00,0x00};
  write_data(agcDisable,2);
  lepton_command(AGC, 0x00  , SET);
  read_reg(0x02);
  
  Serial.println("AGC READ");
  lepton_command(AGC, 0x00  , GET);
  read_data();
}

void loop() {
  uint8_t i,j,tempVal;
  uint16_t x,y,throwaway;
  unsigned long curTime = millis();
  
  Serial.println("Lepton Frame Grab Test");
  // get packets and print contents to serial port
  while(1){
    // sync lepton and arduino
    sync();
    Serial.println("Frame");
    //not really forever
    while(1){
      // get header
      digitalWrite(CS,LOW);
      throwaway = SPI.transfer(read8) << 8;
      throwaway |= SPI.transfer(read8);
      digitalWrite(CS,HIGH);
      // if real packet escape this loop
      if(throwaway & 0x0f00 == 0x0f00){
        break;
      }
      // if not discard the rest of the throwaway packet
      for(i = 0;i<PACKET_LEN-2;i++){
        digitalWrite(CS,LOW);
        SPI.transfer(read8);
        digitalWrite(CS,HIGH);
      }
    }
    // Got real packet
    getFrame(frameBuffer);
    // Write frame to serial port
    Serial.print("ID: "); Serial.println((throwaway<<8)|throwaway,HEX);
    Serial.print("CRC: "); Serial.println((frameBuffer[0]<<8)|frameBuffer[1],HEX);
    for(i = 0;i<FRAME_PACKETS;i++){
      for(j=4;j<PACKET_LEN;j=j+2){
        y = (j-4)*2;
        x = i*4;
        //Serial.print(frameBuffer[PACKET_LEN*i+j],HEX);
        //Serial.print(',');
        tempVal = frameBuffer[PACKET_LEN*i+j+1];
        Serial.print(frameBuffer[PACKET_LEN*i+j],HEX); Serial.print(',');
        //tempVal = 255-tempVal;
        //tempVal = (tempVal << 8) | frameBuffer[PACKET_LEN*i+j+1];
        tft.fillRect(x, y, 4, 4, tft.Color565(colormap[3*tempVal], colormap[3*tempVal+1], colormap[3*tempVal+2]));
        //tft.fillRect(x, y, 4, 4, tempVal);
        //Serial.print(tempVal,HEX); Serial.print(',');
      }
      Serial.println();
    }
  }
}

// get lepton frame -2 bytes of ID data
void getFrame(char * frameBuffer){
  digitalWrite(CS,LOW);
  SPI.transfer(frameBuffer,PACKET_LEN*FRAME_PACKETS-2);
  digitalWrite(CS,HIGH);
}

// sync lepton and controller
void sync(){
  Serial.println("Syncing...");
  uint8_t i,j;
  // de-assert CS
  digitalWrite(CS,HIGH);
  // wait > 5 frame periods (timeout VoSPI interface)
  delay(200);
  // assert CS
  digitalWrite(CS,LOW);
  uint16_t data = 0xffff;
  // Read packets until new frame is ready (ID is not discard)
  while(data & 0x0f00 == 0x0f00){
    // get header
    digitalWrite(CS,LOW);
    data = SPI.transfer(read8) << 8;
    data |= SPI.transfer(read8);
    digitalWrite(CS,HIGH);
    // discard remaining bytes in packet
    for(i = 0;i<PACKET_LEN-2;i++){
      digitalWrite(CS,LOW);
      SPI.transfer(read8);
      digitalWrite(CS,HIGH);
    }
  }
  // Lepton is now synced
}

  //  Send command to lepton module over I2C interface
void lepton_command(uint8_t moduleID, uint8_t commandID, uint8_t command)
{
  byte error;
  // Start transmitting
  Wire.beginTransmission(ADDRESS);
  // Command Register is a 16-bit register located at Register Address 0x0004
  Wire.write(0x00);
  Wire.write(0x04);
  // OEM module requires OEM bit to be set for acceptable command
  if (moduleID == 0x08) //OEM module ID
  {
    // write top byte as 01001000 (OEM Module)
    Wire.write(0x48);
  }
  else
  {
    // write top byte as 0000xxxx where xxxx is the 4 bit module ID
    // AGC: 0001
    // SYS: 0010
    // VID: 0011
    Wire.write(moduleID & 0x0f);
  }
  // write command as top 6 bits of lower byte and type of command as last 2 bits
  // command can be:
  // GET: 00
  // SET: 01
  // RUN: 10
  Wire.write( ((commandID << 2 ) & 0xfc) | (command & 0x3));
  // stop transmitting
  error = Wire.endTransmission();   
  // return any errors to console 
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}

// ping register to indicate a desired read
void set_reg(uint16_t reg)
{
  byte error;
  // start transmitting
  Wire.beginTransmission(ADDRESS);
  // write upper byte
  Wire.write(reg >> 8 & 0xff);
  // write lower byte
  Wire.write(reg & 0xff);            
  // stop transmitting
  error = Wire.endTransmission(); 
  // return any errors for debugging   
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}

// Status reg 15:8 Error Code  7:3 Reserved 2:Boot Status 1:Boot Mode 0:busy
// read register and print to serial console
uint16_t read_reg(uint16_t reg)
{
  uint16_t reading = 0;
  // indicate we wish to read from register
  set_reg(reg);
  // clock out 2 bytes from slave
  Wire.requestFrom(ADDRESS, 2);
  // receive high byte (overwrites previous reading)
  reading = Wire.read();  
  // shift high byte to be high 8 bits
  reading = reading << 8;    
  // receive low byte as lower 8 bits
  reading |= Wire.read(); 
  // output to console for debugging
  Serial.print("reg:");
  Serial.print(reg);
  Serial.print("==0x");
  Serial.print(reading, HEX);
  Serial.print(" binary:");
  Serial.println(reading, BIN);
  return reading;
}

// read data returned in data register and print to serial console
void read_data()
{
  uint8_t i;
  uint16_t data;
  uint8_t payload_length;

  // wait until the previous command finishes processing (busy bit in status register cleared)
  while (read_reg(0x2) & 0x01)
  {
    Serial.println("busy");
  }
  // read number of bytes of data available from 0x06
  payload_length = read_reg(0x6);
  // output number of available bytes
  Serial.print("payload_length=");
  Serial.println(payload_length);
  // request the number of bytes from the device
  Wire.requestFrom(ADDRESS, payload_length);
  // Note: device auto increments from 0x06 (data length register) to 0x08 (data register) and beyond (for multiple data bytes)
  // read out all data bytes 1 word (2 bytes) at a time
  for (i = 0; i < (payload_length / 2); i++)
  {
    data = Wire.read() << 8;
    data |= Wire.read();
    Serial.println(data, HEX);
  }
}

// write data to data register (including data length register)
void write_data(uint8_t * data, uint8_t nBytes)
{
  uint8_t i;
  byte error;

  // wait until the previous command finishes processing (busy bit in status register cleared)
  while (read_reg(0x2) & 0x01)
  {
    Serial.println("busy");
  }
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);
  Wire.write(0x06);
  // write number of bytes of data available to 0x06
  Wire.write(0x00);
  Wire.write(nBytes);
  // Note: device auto increments from 0x06 (data length register) to 0x08 (data register) and beyond (for multiple data bytes)
  // read out all data bytes 1 word (2 bytes) at a time
  for (i = 0; i < nBytes/2; i=i+2)
  {
    Wire.write(data[i]);
    Wire.write(data[i+1]);
  }
  // stop transmitting
  error = Wire.endTransmission(); 
  // return any errors for debugging   
  if (error != 0)
  {
    Serial.print("error=");
    Serial.println(error);
  }
}
