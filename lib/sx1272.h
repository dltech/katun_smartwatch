/* FIFO read/write access */
#define REGFIFO             0x00

/* Operating mode & LoRaTM / FSK selection */
#define REGOPMODE           0x01
// 1 - LoRaTM Mode
#define LongRangeMode       0x80
// 0 - Access LoRa registers page 0x0D: 0x3F
#define AccesSharedReg      0x80
// Device modes
#define ModeSleep           0x00
#define ModeStdBy           0x01
#define ModeFSTx            0x02
#define ModeTx              0x03
#define ModeFX              0x04
#define ModeRxContinuous    0x05
#define ModeRxSingle        0x06
#define ModeCAD             0x07

/* RF Carrier Frequency */
#define REGFRF_MSB          0x06
#define REGFRF_MID          0x07
#define REGFRF_LSB          0x08

/* PA selection and Output Power control */
#define REGPACONFIG         0x09
// Selects PA output pin
#define PaSelect        0x80
// power amplifier max output power
#define OutputPowerMSK  0x0f

/* Over Current Protection control */
#define REGPARAMP           0x0a
// 1 - Low consumption PLL is used in receive and transmit mode
#define LowPnTxPllOff   0x10
// Rise/Fall time of ramp up/down in FSK
#define PaRamp3p4m      0x00
#define PaRamp2m        0x01
#define PaRamp1m        0x02
#define PaRamp500u      0x03
#define PaRamp250u      0x04
#define PaRamp125u      0x05
#define PaRamp100u      0x06
#define PaRamp62u       0x07
#define PaRamp50u       0x08
#define PaRamp40u       0x09
#define PaRamp31u       0x0a
#define PaRamp25u       0x0b
#define PaRamp20u       0x0c
#define PaRamp15u       0x0d
#define PaRamp12u       0x0e
#define PaRamp10u       0x0f

/* Control of PA ramp time, low phase noise PLL */
#define REGOCP              0x0b
// Enables overload current protection
#define OcpOn       0x80
// Trimming of OCP current
#define OcpTrimMSK  0x1f

/* LNA settings */
#define REGLNA              0x0c
// LNA gain setting
#define LnaGainNa   0x00
#define LnaGainG1   0x20
#define LnaGainG2   0x40
#define LnaGainG3   0x60
#define LnaGainG4   0x80
#define LnaGainG4   0xa0
#define LnaGainG6   0xc0
// Boost on, 150% LNA current.
#define LnaBoost    0x03

/* FIFO SPI pointer */
#define REGFIFOADDRPTR      0x0d

/* Start Tx data */
#define REGFIFOTXBASEADDR   0x0e

/* Start Rx data */
#define REGFIFORXBASEADDR   0x0f

/* Start address of last packet received */
#define FIFORXCURRENTADDR   0x10

/* Optional IRQ flag mask */
#define REGIRQFLAGSMASK     0x11
// Timeout interrupt mask
#define RxTimeoutMask           0x80
// Packet reception complete interrupt mask
#define RxDoneMask              0x40
// Payload CRC error interrupt mask
#define PayloadCrcErrorMask     0x20
// Valid header received in Rx mask
#define ValidHeaderMask         0x10
// FIFO Payload transmission complete interrupt mask
#define TxDoneMask              0x08
// CAD complete interrupt mask
#define CadDoneMask             0x04
// FHSS change channel interrupt mask
#define FhssChangeChannelMask   0x02
// FHSS change channel interrupt mask
#define CadDetectedMask         0x01

/* IRQ flags */
#define REGIRQFLAGS         0x12
// Timeout interrupt
#define RxTimeout           0x80
// Packet reception complete interrupt
#define RxDone              0x40
// Payload CRC error interrupt
#define PayloadCrcError     0x20
// Valid header received in Rx
#define ValidHeader         0x10
// FIFO Payload transmission complete interrupt
#define TxDone              0x08
// CAD complete interrupt
#define CadDone             0x04
// FHSS change channel interrupt
#define FhssChangeChannel   0x02
// FHSS change channel interrupt
#define CadDetected         0x01

/* Number of received bytes */
#define REGRXNBYTES         0x13

/* Number of valid headers received */
#define REGRXHEADERCNTVALUE_MSB 0x14
#define REGRXHEADERCNTVALUE_LSB 0x15

/* Number of valid packets received */
#define REGRXPACKETCNTVALUE_MSB 0x16
#define REGRXPACKETCNTVALUE_LSB 0x17

/* Live LoRaTM modem status */
#define REGMODEMSTAT            0x18
// Coding rate of last header received
#define RxCodingRateShift   5
// modem status
#define ModemStatusClear              0x10
#define ModemStatusHeaderInfoValid    0x08
#define ModemStatusRxOnGoing          0x04
#define ModemStatusSignalSynchronized 0x02
#define ModemStatusSignalDetected     0x01

/* Espimation of last packet SNR */
#define REGPKTSNRVALUE          0x19

/* RSSI of last packet */
#define REGPKTRSSIVALUE         0x1a

/* Current RSSI */
#define REGRSSIVALUE            0x1b

/* FHSS start channel */
#define REGHOPCHANNEL           0x1c
// PLL failed to lock while attempting a TX/RX/CAD operation
#define PllTimeout      0x80
// CRC Information extracted from the received packet header
#define CrcOnPayload    0x40
// Current value of frequency hopping channel in use
#define FhssPresentChannelMsk 0x3f

/* Modem PHY config 1 */
#define REGMODEMCONFIG1         0x1d
// Signal bandwidth
#define Bw125                   0x00
#define Bw250                   0x40
#define Bw500                   0x80
// Error coding rate
#define CodingRate4s5           0x08
#define CodingRate4s6           0x10
#define CodingRate4s7           0x18
#define CodingRate4s8           0x20
// 1 - Implicit Header mode
#define ImplicitHeaderModeOn    0x04
// Enable CRC generation and check on payload:
#define RxPayloadCrcOn          0x02
// 1 - Enabled; mandated for SF11 and SF12 with BW = 125 kHz
#define LowDataRateOptimize     0x01

/* Modem PHY config 2 */
#define REGMODEMCONFIG2         0x1e
// SF rate (expressed as a base-2 logarithm)
#define SpreadingFactor64   0x60
#define SpreadingFactor128  0x70
#define SpreadingFactor256  0x80
#define SpreadingFactor512  0x90
#define SpreadingFactor1024 0xa0
#define SpreadingFactor2048 0xb0
#define SpreadingFactor4096 0xc0
// 1 - continuous mode, send multiple packets across the FIFO
#define TxContinuousMode    0x08
// 1 - LNA gain set by the internal AGC loop
#define AgcAutoOn           0x04
// RX Time-Out MSB
#define SymbTimeoutMSK      0x03

/* Receiver timeout value */
#define REGSYMBTIMEOUT_LSB      0x1f

/* Size of preamble */
#define REGPREAMBLE_MSB         0x20
#define REGPREAMBLE_LSB         0x21

/* LoRaTM payload length */
#define REGPAYLOADLENGTH        0x22

/* LoRaTM maximum payload length */
#define REGMAXPAYLOADLENGTH     0x23

/* FHSS Hop period */
#define REGHOPPERIOD            0x24

/* Address of last byte written in FIFO */
#define REGFIFORXBYTEADDR       0x25

/* Estimated frequency error */
#define REGFEI_MSB              0x28
#define REGFEI_MID              0x29
#define REGFEI_LSB              0x2a

/* Wideband RSSI measurement */
#define REGRSSIWIDEBAND         0x2c

/* LoRa detection Optimize for SF6 */
#define REGDETECTOPTIMIZE       0x31
// Should be set to 0x0 after each reset (POR on manual)
#define AutomaticIFOn               0x80
// LoRa detection Optimize
#define DetectionOptimizeSF7toSF12  0x03
#define DetectionOptimizeSF6        0x05

/* Invert LoRa I and Q signals */
#define REGINVERTIQ             0x32
// Invert the LoRa I and Q signals in RX path
#define InvertIQRX  0x40
// Invert the LoRa I and Q signals in TX path
#define InvertIQTX  0x01

/* Change the LoRa Detection threshold for SF6 */
#define REGDETECTIONTHRESHOLD   0x37

/* LoRa Sync Word */
#define REGSYNCWORD             0x39

/* Optimize for inverted IQ */
#define REGINVERTLQ2            0x3b

/* Mapping of pins DIO0 to DIO3 */
#define REGDIOMAPPING1      0x40
/* Mapping of pins DIO4 and DIO5, ClkOut frequency */
#define REGDIOMAPPING2      0x41

/* Semtech ID relating the silicon revision */
#define REGVERSION          0x42

/* Adjustment of the AGC thresholds */
#define REGAGCREF           0x43

#define REGAGCTHRESH1       0x44
#define REGAGCTHRESH2       0x45
#define REGAGCTHRESH3       0x46

/* Control the fast frequency hopping mode */
#define REGPLLHOP           0x4b

/* TCXO or XTAL input setting */
#define REGTCXO             0x58

/* Higher power settings of the PA */
#define REGPADAC            0x5a

/* Control of the PLL bandwidth */
#define REGPLL              0x5c

/* Control of the Low Phase Noise PLL bandwidth */
#define REGPLLLOWPN         0x5e

/* Enables manual PA control */
#define REGPAMANUAL         0x63

/* Stored temperature during the former IQ Calibration */
#define REGFORMERTEMP       0x6c

/* Fractional part in the Bit Rate division ratio */
#define REGBITRATEFRAC      0x70
