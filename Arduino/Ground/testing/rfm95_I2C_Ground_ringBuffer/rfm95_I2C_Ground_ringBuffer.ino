// I2C Slave RFM96 Downlink

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>

#define ADDRESS 0x0F
#define BUFSIZE 672
#define RFBUFSIZE 255
#define FLAGPIN 4
#define TIMEOUT 500

// command definitions
#define READ 0
#define PEEK 1
#define CHECKEOF 2

// Singleton instance of the radio driver
RH_RF95 rf95;

// utility functions
uint16_t bytesAvailable(uint16_t readPos,uint16_t writePos,uint16_t bufSize);
uint16_t updateReadPos(uint16_t readPos,uint16_t nBytes,uint16_t bufSize);

// Data Buffer
uint8_t dataBuf[BUFSIZE];
uint8_t rfBuf[RFBUFSIZE];
uint8_t len = BUFSIZE; 
uint8_t rfLen = RFBUFSIZE; 
uint32_t totalLength = 0;
uint32_t timeOut = millis()+TIMEOUT;
uint8_t newFile = 0;
uint8_t noFile = 1;
volatile uint16_t nBytesTransmit = 1;
volatile uint16_t readPos = 0;
volatile uint16_t writePos = 0;
volatile uint8_t command = 0;
uint8_t newData = 0;
uint8_t newDataLen = 0;

void setup() 
{
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
  pinMode(FLAGPIN,OUTPUT);
  // low to indicate no data
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
  rf95.setTxPower(5, false);
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
  int i;
  if (rf95.available())
  {
    digitalWrite(FLAGPIN,LOW);
    newFile = 0;
    noFile = 0;
    timeOut = millis() + TIMEOUT;
    if (rf95.recv(rfBuf, &rfLen))
    {
      Serial.print("RF:"); Serial.println(rfLen);
      totalLength = totalLength+rfLen;
      newData = 1;
      newDataLen = rfLen;
      rfLen = RFBUFSIZE;
    }
    else
    {
      Serial.println("Recieve Failed");
      delay(100);
    }
  }
  if((millis() > timeOut) && !noFile){
    digitalWrite(FLAGPIN,HIGH);
    Serial.print("Total File Size: "); Serial.println(totalLength);
    totalLength = 0;
    newFile = 1;
    noFile = 1;
  }
  if(newData){
    digitalWrite(FLAGPIN,LOW);
    Serial.println("ND");
    // check if there is space for the data
    if(newDataLen < (BUFSIZE - bytesAvailable(readPos,writePos,BUFSIZE))){
      for(i=0;i<newDataLen;i++){
        dataBuf[writePos] = rfBuf[i];
        writePos = (writePos + 1) % BUFSIZE;
      }
      // raise data available flag
      digitalWrite(FLAGPIN,HIGH);
      newData = 0;
    }
    else{
      // raise data available flag
      digitalWrite(FLAGPIN,HIGH);
      Serial.println("FULL");
    }
  }
  else if(bytesAvailable(readPos,writePos,BUFSIZE)>0){
    digitalWrite(FLAGPIN,HIGH);
  }
  else{
    digitalWrite(FLAGPIN,LOW);
  }
}

// Executed on data request from master
void requestHandler(){
  uint16_t i,transBytes;
  // data read command
  if(command == READ){
    digitalWrite(FLAGPIN,LOW);
    //Serial.print("READ\t");
    // check if there is data to send
    uint16_t nBytes = bytesAvailable(readPos,writePos,nBytesTransmit);
    // if all bytes to be transmitted are linear and they fit in the rfbuffer
    if((readPos+nBytes) < BUFSIZE && nBytes <= nBytesTransmit){
      transBytes = nBytes;
    }
    // if all bytes to be transmitted are linear and they dont fit in the rfbuffer
    else if((readPos+nBytes) < BUFSIZE && nBytes > nBytesTransmit){
      transBytes = nBytesTransmit;
    }
    // if all bytes to be transmitted are not linear and they fit in the rfbuffer
    else if((readPos+nBytes) >= BUFSIZE && (BUFSIZE-readPos) <= nBytesTransmit){
      transBytes = BUFSIZE-readPos;
    }
    // if all bytes to be transmitted are not linear and they dont fit in the rfbuffer
    else if((readPos+nBytes) >= BUFSIZE && (BUFSIZE-readPos) > nBytesTransmit){
      transBytes = nBytesTransmit;
    }
    for(i=0;i<transBytes;i++){
      //Serial.print(char(dataBuf[readPos+i]));
      Wire.write(dataBuf[readPos]);
      readPos = updateReadPos(readPos,1,BUFSIZE);
    }
    //Serial.println();
    //Serial.println(transBytes);
  }
  // check nBytes Available command
  else if(command == PEEK){
    //Serial.println("PEEK");
    uint16_t temp = bytesAvailable(readPos,writePos,BUFSIZE);
    Wire.write(temp >> 8);
    Wire.write(temp & 0xFF);
  }
  // check EOF command
  else if(command == CHECKEOF){
    //Serial.println("EOF");
    if(newFile){
      Wire.write(1);
      newFile = 0;
      // disable data available flag
      digitalWrite(FLAGPIN,LOW);
    }
    else{
      Wire.write(0);
    }
  }
}

// Executed on data received from master
void receiveHandler(int nBytes){
  uint8_t tempBuf[3];
  // accept at maximum three bytes of command
  if(nBytes > 3){
    nBytes = 3;
  }
  // get data
  uint16_t i;
  for(i=0;i<nBytes;i++){
    tempBuf[i] = Wire.read();
  }
  command = tempBuf[0];
  // if read command get number of bytes to read
  if(command == READ){
    nBytesTransmit = (tempBuf[1] << 8) | tempBuf[2];
  }
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
