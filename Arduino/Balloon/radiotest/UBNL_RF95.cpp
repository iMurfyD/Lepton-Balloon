#include "UBNL_RF95.h"

UBNL_RF95::UBNL_RF95(uint8_t slaveSelectPin, uint8_t bootDelay, uint8_t maxSpiClkSpeed, uint8_t txPwr) {
    this->_ssPin = slaveSelectPin;
    // Holding onto a persistant version of the SPI settings so I don't
    // Have to keep typing it
    // TODO Make this configurable
    this->_spiSettings = SPISettings(maxSpiClkSpeed, MSBFIRST, SPI_MODE0);

    // Going to assume that we don't have to cycle txPwr to the radio
    // So we're just going to halt things and wait for it to boot up
    delay(bootDelay);

    // Now let's initalize the SPI bus
    // FYI, using the Arduino SPI library
    SPI.begin();
    pinMode(slaveSelectPin, OUTPUT);
    digitalWrite(slaveSelectPin, HIGH);

    // Put into LoRA sleep mode
    uint8_t resp = spiWrite(UBNL_RF95_REG_01_OP_MODE, UBNL_RF95_MODE_SLEEP | UBNL_RF95_LONG_RANGE_MODE);
    delay(10); // Let shenanigans happens
    Serial.println(resp);

    // Do the FIFO config stuff in sleep mode

    // Set up FIFO
    // We configure so that we can use the entire 256 byte FIFO for either receive
    // or transmit, but not both at the same time
    spiWrite(UBNL_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    spiWrite(UBNL_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    // Now put into standby mode to do the rest of the config
    // RH did this, so maybe there's a good reason?
    resp = spiWrite(UBNL_RF95_REG_01_OP_MODE,UBNL_RF95_MODE_STDBY);
    delay(10); // Let shenanigans happens
    Serial.println(resp);

    // Set Modem config
    // Values taken from RH library
    spiWriteModemConfig(0x72, 0x74, 0x00);

    // Set Preamble length
    spiWrite(UBNL_RF95_REG_20_PREAMBLE_MSB, preamble_length >> 8);
    spiWrite(UBNL_RF95_REG_21_PREAMBLE_LSB, preamble_length & 0xff);

    // Set frequency
    // Taken from RH
    // Frf = FRF / FSTEP
    uint32_t frf = (UBNL_RF95_FREQ * 1000000.0) / UBNL_RF95_FSTEP;
    spiWrite(UBNL_RF95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
    spiWrite(UBNL_RF95_REG_07_FRF_MID, (frf >> 8) & 0xff);
    spiWrite(UBNL_RF95_REG_08_FRF_LSB, frf & 0xff);

    // Set TX txPwr
    // Some stuff taken from RH
    bool useRFO = false;
    // Sigh, different behaviours depending on whther the module use PA_BOOST or the RFO pin
    // for the transmitter output
    if (useRFO) {
        if (txPwr > 14)
	       txPwr = 14;
	    if (txPwr < -1)
	       txPwr = -1;
	    spiWrite(UBNL_RF95_REG_09_PA_CONFIG, UBNL_RF95_MAX_POWER | (txPwr + 1));
    }
    else {
	    if (txPwr > 23)
            txPwr = 23;
	    if (txPwr < 5)
            txPwr = 5;
    }
    // For UBNL_RF95_PA_DAC_ENABLE, manual says '+20dBm on PA_BOOST when OutputtxPwr=0xf'
	// UBNL_RF95_PA_DAC_ENABLE actually adds about 3dBm to all txPwr levels. We will us it
	// for 21, 22 and 23dBm
	if (txPwr > 20)
	{
	    spiWrite(UBNL_RF95_REG_4D_PA_DAC, UBNL_RF95_PA_DAC_ENABLE);
	    txPwr -= 3;
	}
	else
	{
	    spiWrite(UBNL_RF95_REG_4D_PA_DAC, UBNL_RF95_PA_DAC_DISABLE);
	}

    // RFM95/96/97/98 does not have RFO pins connected to anything. Only PA_BOOST
    // pin is connected, so must use PA_BOOST
    // Pout = 2 + OutputtxPwr.
    // The documentation is pretty confusing on this topic: PaSelect says the max txPwr is 20dBm,
    // but OutputtxPwr claims it would be 17dBm.
    // My measurements show 20dBm is correct
    spiWrite(UBNL_RF95_REG_09_PA_CONFIG, UBNL_RF95_PA_SELECT | (txPwr-5));
}

// Writes one byte to a register
// Does not handle setting this whole shebang up
// With the exception of handling the ss lines
// Gently borrowed from RadioHead
// returns the response from the radio
uint8_t UBNL_RF95::spiWrite(uint8_t reg, uint8_t val) {
    uint8_t ret;
    digitalWrite(_ssPin, LOW);
    SPI.beginTransaction(_spiSettings);
    SPI.transfer(reg); // Send the address
    ret = SPI.transfer(val); // Send the value
    delay(UBNL_RF95_SPI_DELAY); // No idea how long this going to take to write
    SPI.endTransaction();
    digitalWrite(_ssPin, HIGH);
    return ret;
}

// Reads one byte from the selected register
// Similarly does not set the whole schebang up
// With the exception of handling the ss lines
uint8_t UBNL_RF95::spiRead(uint8_t reg) {
    return '9';
    // I'm not sure if I need this function, so rn it's completely not
    // written. Wubba lubba dub dub
}

uint8_t UBNL_RF95::spiWriteModemConfig(uint8_t m1, uint8_t m2, uint8_t m3) {
    uint8_t ret;
    digitalWrite(_ssPin, LOW);
    SPI.beginTransaction(_spiSettings);
    SPI.transfer(UBNL_RF95_REG_1D_MODEM_CONFIG1); // addr
    ret = SPI.transfer(m1); // Send the value
    delay(UBNL_RF95_SPI_DELAY); // No idea how long this going to take to write
    SPI.transfer(UBNL_RF95_REG_1E_MODEM_CONFIG2); // addr
    ret = SPI.transfer(m2); // Send the value
    delay(UBNL_RF95_SPI_DELAY); // No idea how long this going to take to write
    SPI.transfer(UBNL_RF95_REG_26_MODEM_CONFIG3); // addr
    ret = SPI.transfer(m3); // Send the value
    delay(UBNL_RF95_SPI_DELAY); // No idea how long this going to take to write
    SPI.endTransaction();
    digitalWrite(_ssPin, HIGH);
    return ret;
}
