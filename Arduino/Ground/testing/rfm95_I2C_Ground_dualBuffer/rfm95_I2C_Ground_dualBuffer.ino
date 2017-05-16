// I2C Slave RFM96 Downlink

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
uint8_t readPos = 0;

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
  if (rf95.available())
  {
    if(!full && readPos == 0)
    {
      // stop read requests when recieving from LoRa
      digitalWrite(FLAGPIN,LOW);
      if (rf95.recv(writeBuf, BUFSIZE))
      {
        Serial.println("Got RF95 Packet");
        // switch write buffer to zero
        if(writeNum == 1 && fullBuf_0 == 0){
          fullBuf_1 = 1;
          writeBuf = dataBuf_0;
          readBuf = dataBuf_1;
          writeNum = 0;
          readNum = 1;
          empty = 0;
        }
        // switch write buffer to one
        else if(writeNum == 0 && fullBuf_1 == 0){
          fullBuf_0 = 1;
          writeBuf = dataBuf_1;
          readBuf = dataBuf_0;
          writeNum = 1;
          readNum = 0;
          empty = 0;
        }
        // both buffers full
        else{
          full = 1;
          Serial.println("Full");
        }
        Serial.print("Write Buffer\t"); Serial.print(writeNum); Serial.print("\tRead Buffer\t"); Serial.println(readNum);
        // raise data available flag
        digitalWrite(FLAGPIN,HIGH);
      }
      else
      {
        Serial.println("Recieve Failed");
        delay(100);
      }
    }
  }
}

// Executed on data request from master
void requestHandler(){
  // debug message
  Serial.println("Got data Request");
  // check if there is data to send
  if(empty == 1 && readPos == 0){
    // return nothing.
  }
  else{
    // return 224 byte packet (7*32)
    // transmit 32 bytes
    // NOTE: THIS NEEDS TO BE A SINGLE TRANSFER. THE PI WILL MAKE 7 SEPERATE TRANSFER REQUESTS, THIS IS WASTING ALL THE DATA.
    Wire.write(readBuf+(readPos*32),32);
    readPos = (readPos + 1)%7;
    Serial.print("ReadPos: "); Serial.println(readPos);
    Serial.println("Transmitted 32 Byte packet");
    // clear full flag
    if(readNum == 0){
      fullBuf_0 = 0;
      readBuf = dataBuf_1;
      readNum = 1;
    }
    else if(readNum == 1){
      fullBuf_1 = 0;
      readBuf = dataBuf_0;
      readNum = 0;
    }
    // check if there is no more data
    if(!full){
      digitalWrite(FLAGPIN,LOW);
      empty = 1;
      Serial.println("Empty");
    }
    full = 0;
  }
}

// Executed on data received from master
void receiveHandler(int nBytes){
  
}

