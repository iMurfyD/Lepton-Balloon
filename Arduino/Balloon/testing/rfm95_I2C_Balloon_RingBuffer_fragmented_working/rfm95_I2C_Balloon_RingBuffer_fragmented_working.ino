// I2C Slave RFM96 Downlink

#define debugPrint

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>

#define ADDRESS 0x0F
#define BUFSIZE 672
#define RFBUFSIZE 251
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
uint16_t writePos = 0;
uint16_t readPos = 0;
uint16_t threshold = 223;
volatile uint8_t newData = 0;
volatile uint8_t checkedBytes = 0;
uint32_t totalBytes = 0;
uint8_t nBytesTransmitted = 0;

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
  uint16_t transBytes;
  // number of bytes available
  uint16_t nBytes;
  if(newData > 0){
    digitalWrite(FLAGPIN,HIGH);
    //Serial.print("N:");Serial.println(newData);
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
    checkedBytes = 1;
  }
  else{
    digitalWrite(FLAGPIN,LOW);
    checkedBytes = 1;
  }
  // if buffer over threashold transmit.
  nBytes = bytesAvailable(readPos,writePos,BUFSIZE);
  //Serial.println(nBytes);
  if((nBytes > threshold) || eof){
    digitalWrite(FLAGPIN,HIGH);
    //Serial.print(readPos); Serial.print('\t'); Serial.print(nBytes);Serial.print('\t');
    // if all bytes to be transmitted are linear and they fit in the rfbuffer
    if((readPos+nBytes) < BUFSIZE && nBytes <= RFBUFSIZE){
      transBytes = nBytes;
    }
    // if all bytes to be transmitted are linear and they dont fit in the rfbuffer
    else if((readPos+nBytes) < BUFSIZE && nBytes > RFBUFSIZE){
      transBytes = RFBUFSIZE;
    }
    // if all bytes to be transmitted are not linear and they fit in the rfbuffer
    else if((readPos+nBytes) >= BUFSIZE && (BUFSIZE-readPos) <= RFBUFSIZE){
      transBytes = BUFSIZE-readPos;
    }
    // if all bytes to be transmitted are not linear and they dont fit in the rfbuffer
    else if((readPos+nBytes) >= BUFSIZE && (BUFSIZE-readPos) > RFBUFSIZE){
      transBytes = RFBUFSIZE;
    }
    else{
      transBytes = 0;
      Serial.println("Uh Oh...");
    }
    
    //Serial.println(transBytes);
    
    if(rf95.send(&dataBuf[readPos], transBytes)){
      rf95.waitPacketSent();
      Serial.print(readPos);Serial.print('\t');Serial.print(writePos);Serial.print('\t');Serial.println(transBytes);
      // change readPos
      readPos = updateReadPos(readPos,transBytes,BUFSIZE);
      // reset eof
      nBytes = bytesAvailable(readPos,writePos,BUFSIZE);
      if(eof && (nBytes == 0) && checkedBytes){
        eof = 0;
        Serial.print("Total File Size: "); Serial.println(totalBytes);
        totalBytes = 0;
      }
    }
    else
    {
      Serial.println("Transmit Failed");
      delay(100);
    }
    //digitalWrite(FLAGPIN,LOW);
  }
  else{
    digitalWrite(FLAGPIN,LOW);
  }
}

// Executed on data request from master
void requestHandler(){
  digitalWrite(FLAGPIN,HIGH);
  eof = 1;
  checkedBytes = 0;
}

// Executed on data received from master
void receiveHandler(int nBytes){
  // flag so we only get one packet at a time
  digitalWrite(FLAGPIN,HIGH);
  uint16_t i;
  
  for(i=0;i<nBytes;i++){
    tempBuf[i] = Wire.read();
  }
  newData = nBytes;
}

uint16_t bytesAvailable(uint16_t readPos,uint16_t writePos,uint16_t bufSize){
  if(writePos >= readPos){
    return writePos - readPos;
  }
  else if(writePos < readPos){
    return bufSize + writePos - readPos + 1;
  }
  else if(writePos == readPos){
    return 0;
  }
}

uint16_t updateReadPos(uint16_t readPos,uint16_t nBytes,uint16_t bufSize){
  return (readPos + nBytes) % bufSize;
}

