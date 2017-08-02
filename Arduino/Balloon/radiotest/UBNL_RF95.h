/*
 * Created by Ian DesJardin for Lepton Ballon
 * to test horizon sensors for UBNL
 * Summer 2017
 * Inspired by (and in some places copied from) RadioHead RF95 Library
 * Liscense Shenanigans go on here
 */

#include <stdint.h> // For uint8_t type
#include <SPI.h>
#include <Arduino.h>
#include <string.h>


#ifndef UBNL_RF95_h
#define UBNL_RF95_h
#endif

// Max number of octets the LORA Rx/Tx FIFO buffer can hold
#define UBNL_RF95_FIFO_SIZE 255


// Register names (LoRa Mode, from table 85)
#define UBNL_RF95_REG_00_FIFO                                0x00
#define UBNL_RF95_REG_01_OP_MODE                             0x01
#define UBNL_RF95_REG_02_RESERVED                            0x02
#define UBNL_RF95_REG_03_RESERVED                            0x03
#define UBNL_RF95_REG_04_RESERVED                            0x04
#define UBNL_RF95_REG_05_RESERVED                            0x05
#define UBNL_RF95_REG_06_FRF_MSB                             0x06
#define UBNL_RF95_REG_07_FRF_MID                             0x07
#define UBNL_RF95_REG_08_FRF_LSB                             0x08
#define UBNL_RF95_REG_09_PA_CONFIG                           0x09
#define UBNL_RF95_REG_0A_PA_RAMP                             0x0a
#define UBNL_RF95_REG_0B_OCP                                 0x0b
#define UBNL_RF95_REG_0C_LNA                                 0x0c
#define UBNL_RF95_REG_0D_FIFO_ADDR_PTR                       0x0d
#define UBNL_RF95_REG_0E_FIFO_TX_BASE_ADDR                   0x0e
#define UBNL_RF95_REG_0F_FIFO_RX_BASE_ADDR                   0x0f
#define UBNL_RF95_REG_10_FIFO_RX_CURRENT_ADDR                0x10
#define UBNL_RF95_REG_11_IRQ_FLAGS_MASK                      0x11
#define UBNL_RF95_REG_12_IRQ_FLAGS                           0x12
#define UBNL_RF95_REG_13_RX_NB_BYTES                         0x13
#define UBNL_RF95_REG_14_RX_HEADER_CNT_VALUE_MSB             0x14
#define UBNL_RF95_REG_15_RX_HEADER_CNT_VALUE_LSB             0x15
#define UBNL_RF95_REG_16_RX_PACKET_CNT_VALUE_MSB             0x16
#define UBNL_RF95_REG_17_RX_PACKET_CNT_VALUE_LSB             0x17
#define UBNL_RF95_REG_18_MODEM_STAT                          0x18
#define UBNL_RF95_REG_19_PKT_SNR_VALUE                       0x19
#define UBNL_RF95_REG_1A_PKT_RSSI_VALUE                      0x1a
#define UBNL_RF95_REG_1B_RSSI_VALUE                          0x1b
#define UBNL_RF95_REG_1C_HOP_CHANNEL                         0x1c
#define UBNL_RF95_REG_1D_MODEM_CONFIG1                       0x1d
#define UBNL_RF95_REG_1E_MODEM_CONFIG2                       0x1e
#define UBNL_RF95_REG_1F_SYMB_TIMEOUT_LSB                    0x1f
#define UBNL_RF95_REG_20_PREAMBLE_MSB                        0x20
#define UBNL_RF95_REG_21_PREAMBLE_LSB                        0x21
#define UBNL_RF95_REG_22_PAYLOAD_LENGTH                      0x22
#define UBNL_RF95_REG_23_MAX_PAYLOAD_LENGTH                  0x23
#define UBNL_RF95_REG_24_HOP_PERIOD                          0x24
#define UBNL_RF95_REG_25_FIFO_RX_BYTE_ADDR                   0x25
#define UBNL_RF95_REG_26_MODEM_CONFIG3                       0x26

#define UBNL_RF95_REG_40_DIO_MAPPING1                        0x40
#define UBNL_RF95_REG_41_DIO_MAPPING2                        0x41
#define UBNL_RF95_REG_42_VERSION                             0x42

#define UBNL_RF95_REG_4B_TCXO                                0x4b
#define UBNL_RF95_REG_4D_PA_DAC                              0x4d
#define UBNL_RF95_REG_5B_FORMER_TEMP                         0x5b
#define UBNL_RF95_REG_61_AGC_REF                             0x61
#define UBNL_RF95_REG_62_AGC_THRESH1                         0x62
#define UBNL_RF95_REG_63_AGC_THRESH2                         0x63
#define UBNL_RF95_REG_64_AGC_THRESH3                         0x64

// UBNL_RF95_REG_01_OP_MODE                             0x01
#define UBNL_RF95_LONG_RANGE_MODE                       0x80
#define UBNL_RF95_ACCESS_SHARED_REG                     0x40
#define UBNL_RF95_MODE                                  0x07
#define UBNL_RF95_MODE_SLEEP                            0x00
#define UBNL_RF95_MODE_STDBY                            0x01
#define UBNL_RF95_MODE_FSTX                             0x02
#define UBNL_RF95_MODE_TX                               0x03
#define UBNL_RF95_MODE_FSRX                             0x04
#define UBNL_RF95_MODE_RXCONTINUOUS                     0x05
#define UBNL_RF95_MODE_RXSINGLE                         0x06
#define UBNL_RF95_MODE_CAD                              0x07

// UBNL_RF95_REG_09_PA_CONFIG                           0x09
#define UBNL_RF95_PA_SELECT                             0x80
#define UBNL_RF95_MAX_POWER                             0x70
#define UBNL_RF95_OUTPUT_POWER                          0x0f

// UBNL_RF95_REG_0A_PA_RAMP                             0x0a
#define UBNL_RF95_LOW_PN_TX_PLL_OFF                     0x10
#define UBNL_RF95_PA_RAMP                               0x0f
#define UBNL_RF95_PA_RAMP_3_4MS                         0x00
#define UBNL_RF95_PA_RAMP_2MS                           0x01
#define UBNL_RF95_PA_RAMP_1MS                           0x02
#define UBNL_RF95_PA_RAMP_500US                         0x03
#define UBNL_RF95_PA_RAMP_250US                         0x0
#define UBNL_RF95_PA_RAMP_125US                         0x05
#define UBNL_RF95_PA_RAMP_100US                         0x06
#define UBNL_RF95_PA_RAMP_62US                          0x07
#define UBNL_RF95_PA_RAMP_50US                          0x08
#define UBNL_RF95_PA_RAMP_40US                          0x09
#define UBNL_RF95_PA_RAMP_31US                          0x0a
#define UBNL_RF95_PA_RAMP_25US                          0x0b
#define UBNL_RF95_PA_RAMP_20US                          0x0c
#define UBNL_RF95_PA_RAMP_15US                          0x0d
#define UBNL_RF95_PA_RAMP_12US                          0x0e
#define UBNL_RF95_PA_RAMP_10US                          0x0f

// UBNL_RF95_REG_0B_OCP                                 0x0b
#define UBNL_RF95_OCP_ON                                0x20
#define UBNL_RF95_OCP_TRIM                              0x1f

// UBNL_RF95_REG_0C_LNA                                 0x0c
#define UBNL_RF95_LNA_GAIN                              0xe0
#define UBNL_RF95_LNA_BOOST                             0x03
#define UBNL_RF95_LNA_BOOST_DEFAULT                     0x00
#define UBNL_RF95_LNA_BOOST_150PC                       0x11

// UBNL_RF95_REG_11_IRQ_FLAGS_MASK                      0x11
#define UBNL_RF95_RX_TIMEOUT_MASK                       0x80
#define UBNL_RF95_RX_DONE_MASK                          0x40
#define UBNL_RF95_PAYLOAD_CRC_ERROR_MASK                0x20
#define UBNL_RF95_VALID_HEADER_MASK                     0x10
#define UBNL_RF95_TX_DONE_MASK                          0x08
#define UBNL_RF95_CAD_DONE_MASK                         0x04
#define UBNL_RF95_FHSS_CHANGE_CHANNEL_MASK              0x02
#define UBNL_RF95_CAD_DETECTED_MASK                     0x01

// UBNL_RF95_REG_12_IRQ_FLAGS                           0x12
#define UBNL_RF95_RX_TIMEOUT                            0x80
#define UBNL_RF95_RX_DONE                               0x40
#define UBNL_RF95_PAYLOAD_CRC_ERROR                     0x20
#define UBNL_RF95_VALID_HEADER                          0x10
#define UBNL_RF95_TX_DONE                               0x08
#define UBNL_RF95_CAD_DONE                              0x04
#define UBNL_RF95_FHSS_CHANGE_CHANNEL                   0x02
#define UBNL_RF95_CAD_DETECTED                          0x01

// UBNL_RF95_REG_18_MODEM_STAT                          0x18
#define UBNL_RF95_RX_CODING_RATE                        0xe0
#define UBNL_RF95_MODEM_STATUS_CLEAR                    0x10
#define UBNL_RF95_MODEM_STATUS_HEADER_INFO_VALID        0x08
#define UBNL_RF95_MODEM_STATUS_RX_ONGOING               0x04
#define UBNL_RF95_MODEM_STATUS_SIGNAL_SYNCHRONIZED      0x02
#define UBNL_RF95_MODEM_STATUS_SIGNAL_DETECTED          0x01

// UBNL_RF95_REG_1C_HOP_CHANNEL                         0x1c
#define UBNL_RF95_PLL_TIMEOUT                           0x80
#define UBNL_RF95_RX_PAYLOAD_CRC_IS_ON                  0x40
#define UBNL_RF95_FHSS_PRESENT_CHANNEL                  0x3f

// UBNL_RF95_REG_1D_MODEM_CONFIG1                       0x1d
#define UBNL_RF95_BW                                    0xc0
#define UBNL_RF95_BW_125KHZ                             0x00
#define UBNL_RF95_BW_250KHZ                             0x40
#define UBNL_RF95_BW_500KHZ                             0x80
#define UBNL_RF95_BW_RESERVED                           0xc0
#define UBNL_RF95_CODING_RATE                           0x38
#define UBNL_RF95_CODING_RATE_4_5                       0x00
#define UBNL_RF95_CODING_RATE_4_6                       0x08
#define UBNL_RF95_CODING_RATE_4_7                       0x10
#define UBNL_RF95_CODING_RATE_4_8                       0x18
#define UBNL_RF95_IMPLICIT_HEADER_MODE_ON               0x04
#define UBNL_RF95_RX_PAYLOAD_CRC_ON                     0x02
#define UBNL_RF95_LOW_DATA_RATE_OPTIMIZE                0x01

// UBNL_RF95_REG_1E_MODEM_CONFIG2                       0x1e
#define UBNL_RF95_SPREADING_FACTOR                      0xf0
#define UBNL_RF95_SPREADING_FACTOR_64CPS                0x60
#define UBNL_RF95_SPREADING_FACTOR_128CPS               0x70
#define UBNL_RF95_SPREADING_FACTOR_256CPS               0x80
#define UBNL_RF95_SPREADING_FACTOR_512CPS               0x90
#define UBNL_RF95_SPREADING_FACTOR_1024CPS              0xa0
#define UBNL_RF95_SPREADING_FACTOR_2048CPS              0xb0
#define UBNL_RF95_SPREADING_FACTOR_4096CPS              0xc0
#define UBNL_RF95_TX_CONTINUOUS_MOE                     0x08
#define UBNL_RF95_AGC_AUTO_ON                           0x04
#define UBNL_RF95_SYM_TIMEOUT_MSB                       0x03

// UBNL_RF95_REG_4D_PA_DAC                              0x4d
#define UBNL_RF95_PA_DAC_DISABLE                        0x04
#define UBNL_RF95_PA_DAC_ENABLE                         0x07

// Misc not included above but included from RadioHead
#define UBNL_RF95_FXOSC 32000000.0
#define UBNL_RF95_FSTEP (UBNL_RF95_FXOSC / 524288)
#define UBNL_TX_Power

// Defined by moi
#define UBNL_RF95_SPI_DELAY 2 // in ms, will delay after each SPI write
#define UBNL_RF95_FREQ 434.0 // center frequency

// Length of preamble in LoRA packets
const uint16_t preamble_length = 8;

class UBNL_RF95 {
public:
    UBNL_RF95(uint8_t slaveSelectPin, uint8_t bootDelay, uint8_t maxSpiClkSpeed, uint8_t txPwr);

private:
    uint8_t _ssPin;
    uint8_t spiWriteModemConfig(uint8_t m1, uint8_t m2, uint8_t m3);
    SPISettings _spiSettings;
    uint8_t spiWrite(uint8_t reg, uint8_t val);
    uint8_t spiRead(uint8_t register);
};
