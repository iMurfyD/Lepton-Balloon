#include <SPI.h>
#include <Wire.h>

#define ADDRESS 0x0F
#define FLAGPIN 4
#define BUFFER_SIZE 32 // Number of bytes for internal arduino buffer

const int SS_HOPERF95 = 0; // SS line to choose the HopeRF 95 radio
const int BOOT_DELAY = 1000; // ms to wait for radio to boot up
volatile uint8_t newData = 0;
volatile byte tempBuf[BUFFER_SIZE];

void setup() {
  // Initalize Pi Zero I2C path as slave
  Wire.begin(ADDRESS);
  // register request action
  Wire.onRequest(requestHandler);
  // register recieve action
  Wire.onReceive(receiveHandler);
  Serial.begin(115200);
  Serial.println("We done initalized");
  // Initalize Radio SPI path
  SPI.begin();
  pinMode(SS_HOPERF95, OUTPUT);
  delay(BOOT_DELAY);
}

void loop() {
  if(newData > 0) {
    // Oh dearie me, I got some new data
    for(int i=0;i<newData;i++)
      Serial.println(tempBuf[i]);
    // I say let's print it out and wait for more
    newData = 0; // Mark data as printed, so we don't reprint it
  }

}


// Executed on data request from master
void requestHandler(){
  Serial.println("requestHandler");
  digitalWrite(FLAGPIN,HIGH);
}

// Executed on data received from master
// Stores data sent over by master in tempBuf
// And sets newData to be a nonzero number, indicating that
// Data has been received
void receiveHandler(int nBytes){
  Serial.println("receiveHandler");
  // flag so we only get one packet at a time
  digitalWrite(FLAGPIN,HIGH);
  uint16_t i;
  
  for(i=0;i<nBytes;i++){
    tempBuf[i] = Wire.read();
  }
  newData = nBytes;
}

