#include <SPI.h>

const int SS_HOPERF95 = 0; // SS line to choose the HopeRF 95 radio
const int BOOT_DELAY = 100; // ms to wait for radio to boot up
const int HOPE_RF_95_CLK; // Max clock speed for SPI line to radio

void setup() {
  // Initalize Radio SPI path
  SPI.begin();
  pinMode(SS_HOPERF95, OUTPUT);
  digitalWrite(SS_HOPERF95, HIGH);
  delay(BOOT_DELAY);

  // Send one time things to radio
  SPI.beginTransaction(SPISettings(HOPE_RF_95_CLK, MSBFIRST, SPI_MODE0));
}

void loop() {
  char* thing_to_tx = "hfahjfahjjahjdjjkljkdsafdsajkjfdkljfdkjkldafjfadjkladsfl;jkdfi";
  
}
