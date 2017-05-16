// I2C Slave RFM96 Downlink

#define debugPrint

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>

#define ADDRESS 0x0F
#define BUFSIZE 224
#define FLAGPIN 4

// Singleton instance of the radio driver
RH_RF95 rf95;

// Data Buffer
byte dataBuf_0[BUFSIZE];
byte dataBuf_1[BUFSIZE];
byte *writeBuf = dataBuf_0;
byte *readBuf = dataBuf_0;
uint8_t fullBuf_0 = 0;
uint8_t fullBuf_1 = 0;
uint8_t writeNum = 0;
uint8_t readNum = 0;
uint8_t full = 0;
uint8_t empty = 1;
uint8_t writePos = 0;
uint8_t subPacketNum = 0;

void setup() 
{
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
  pinMode(FLAGPIN,OUTPUT);
  digitalWrite(FLAGPIN,LOW);
  /*
   * RFM 96 Setup
   */
  if (!rf95.init())
    Serial.println("init failed");

  Serial.print("Max Message Length: ");
  Serial.print(rf95.maxMessageLength());
  Serial.println(" Bytes");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Set to fast mode (~500Bps)
  rf95.setModemConfig(1);
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(10, false);
  /*
   * I2C Slave Setup
   */
  // join I2C bus as slave
  Wire.begin(ADDRESS);
  // register request action
  Wire.onRequest(requestHandler);
  // register recieve action
  Wire.onReceive(receiveHandler);
}

// Main Loop
void loop()
{
  // if either buffer is full transmit entire contents
  if(fullBuf_0 == 1){
    digitalWrite(FLAGPIN,HIGH);
    Serial.println("Sending Buf 0");
    rf95.waitPacketSent();
    rf95.send(dataBuf_0, BUFSIZE);
    fullBuf_0 = 0;
    digitalWrite(FLAGPIN,LOW);
  }
  if(fullBuf_1 == 1){
    digitalWrite(FLAGPIN,HIGH);
    Serial.println("Sending Buf 1");
    rf95.waitPacketSent();
    rf95.send(dataBuf_1, BUFSIZE);
    fullBuf_1 = 0;
    digitalWrite(FLAGPIN,LOW);
  }
}

// Executed on data request from master
void requestHandler(){

}

// Executed on data received from master
void receiveHandler(int nBytes){
  // flag so we only get one packet at a time
  digitalWrite(FLAGPIN,HIGH);
  // check if we are in the middle of a packet transfer
  if(writePos == 0){
    // check which buffer is full
    if((fullBuf_0 == 1) && (fullBuf_1 == 1)){
      // cant write, don't read anything
      Serial.println("FULL");
      return;
    }
    else if(!fullBuf_0){
      Serial.println("Active Buffer 0");
      writeBuf = dataBuf_0;
      writeNum = 0;
    }
    else{
      Serial.println("Active Buffer 1");
      writeBuf = dataBuf_1;
      writeNum = 1;
    }
  }
  // print number of bytes recieved
  //Serial.print("Recieved "); Serial.print(nBytes); Serial.println(" I2C bytes");
  Serial.println('.');
  uint8_t i = 0;
  for(i=0;i<32;i++){
    writeBuf[writePos] = Wire.read();
    //Serial.print(char(dataBuf[writePos]));
    writePos = (writePos + 1) % (BUFSIZE);
  }
  // track sub packet (32 byte chunk) index
  subPacketNum = (subPacketNum + 1) % 8;
  // if buffer is full (7 sub packets) raise buffer full flag
  if(subPacketNum == 7){
    subPacketNum = 0;
    if(writeNum == 0){
      fullBuf_0 = 1;
    }
    else if(writeNum == 1){
      fullBuf_1 = 1;
    }
  }
  //Serial.println();
  digitalWrite(FLAGPIN,LOW);
}

