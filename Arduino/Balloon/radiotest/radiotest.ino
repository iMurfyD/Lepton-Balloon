#include "UBNL_RF95.h"

const int SS_HOPE_RF95 = 0; // SS line to choose the HopeRF 95 radio
const int BOOT_DELAY = 100; // ms to wait for radio to boot up
const int HOPE_RF95_CLK; // Max clock speed for SPI line to radio
const int TX_PWR = 13;
UBNL_RF95* radio;

void setup() {
  // Initalize Radio path
  radio = new UBNL_RF95(SS_HOPE_RF95, BOOT_DELAY, HOPE_RF95_CLK, TX_PWR);
  Serial.begin(115200);
}

void loop() {
  char* thing_to_tx = "hfahjfahjjahjdjjkljkdsafdsajkjfdkljfdkjkldafjfadjkladsfl;jkdfi";
  
}
