// I2C Slave RFM96 Downlink

#define debugPrint

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>

#define ADDRESS 0x0F
#define BUFSIZE 700
#define FLAGPIN 4

// Singleton instance of the radio driver
RH_RF95 rf95;

// utility functions
uint16_t bytesAvailable(uint16_t readPos,uint16_t writePos,uint16_t bufSize);
uint16_t updateReadPos(uint16_t readPos,uint16_t nBytes,uint16_t bufSize);

// Data Buffer
byte dataBuf[BUFSIZE];
// temp buffer
volatile byte tempBuf[32];
volatile uint8_t eof = 0;
uint16_t writePos = 1;
uint16_t readPos = 0;
uint16_t threshold = 223;
volatile uint8_t newData = 0;
uint32_t totalBytes = 0;

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
  //Serial.print(rf95.maxMessageLength());
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
  uint8_t i;
  // check number of bytes available
  uint16_t nBytes = bytesAvailable(readPos,writePos,BUFSIZE);
  if(newData > 0){
    totalBytes = totalBytes + newData;
    //Serial.println(totalBytes);
    //Serial.println(nBytes);
    //Serial.println(newData);
    // check if we can store all transmitted bytes in buffer
    if (newData > (BUFSIZE - nBytes)){
      Serial.println("Full");
    }
    else{
      for(i=0;i<newData;i++){
        dataBuf[writePos] = tempBuf[i];
        writePos = (writePos + 1) % (BUFSIZE);
      }
      newData = 0;
    }
  }
  // if buffer over threashold transmit.
  nBytes = bytesAvailable(readPos,writePos,BUFSIZE);
  if((nBytes > threshold) || eof){
    digitalWrite(FLAGPIN,HIGH);
    //Serial.println("Send");
    rf95.waitPacketSent();
    rf95.send(dataBuf, nBytes);
    // change readPos
    readPos = updateReadPos(readPos,nBytes,BUFSIZE);
    // reset eof
    if(eof){
      eof = 0;
    }
    digitalWrite(FLAGPIN,LOW);
  }
  else{
    digitalWrite(FLAGPIN,LOW);
  }
}

// Executed on data request from master
void requestHandler(){
  eof = 1;
  Serial.print("Total File Size: "); Serial.println(totalBytes);
  totalBytes = 0;
}

// Executed on data received from master
void receiveHandler(int nBytes){
  //Serial.println("R");
  uint16_t i;
  // flag so we only get one packet at a time
  digitalWrite(FLAGPIN,HIGH);
  
  for(i=0;i<nBytes;i++){
    tempBuf[i] = Wire.read();
  }
  newData = nBytes;
}

uint16_t bytesAvailable(uint16_t readPos,uint16_t writePos,uint16_t bufSize){
  if(writePos > readPos){
    return writePos - readPos -1;
  }
  else if(writePos < readPos){
    return bufSize + writePos - readPos -1;
  }
  else if(writePos == readPos){
    return bufSize;
  }
}

uint16_t updateReadPos(uint16_t readPos,uint16_t nBytes,uint16_t bufSize){
  return (readPos + nBytes) % bufSize;
}

