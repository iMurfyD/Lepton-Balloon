// I2C Slave RFM96 Downlink

#include <SPI.h>
#include <RH_RF95.h>

#define BUFSIZE 255
#define FLAGPIN 4
#define TIMEOUT 1000

// Singleton instance of the radio driver
RH_RF95 rf95;

// Data Buffer
byte dataBuf[BUFSIZE];
uint8_t len = BUFSIZE; 
uint32_t totalLength = 0;
uint32_t timeOut = millis()+TIMEOUT;
uint8_t newFile = 1;

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
}

// Main Loop
void loop()
{
  if (rf95.available())
  {
    newFile = 0;
    timeOut = millis() + TIMEOUT;
    // stop read requests when recieving from LoRa
    digitalWrite(FLAGPIN,LOW);
    if (rf95.recv(dataBuf, &len))
    {
      Serial.print("Got RF95 Packet: "); Serial.println(len);
      totalLength = totalLength+len;
      len = BUFSIZE;
      // raise data available flag
      digitalWrite(FLAGPIN,HIGH);
    }
    else
    {
      Serial.println("Recieve Failed");
      delay(100);
    }
  }
  if((millis() > timeOut) && !newFile){
    Serial.print("Total File Size: "); Serial.println(totalLength);
    totalLength = 0;
    newFile = 1;
  }
}

