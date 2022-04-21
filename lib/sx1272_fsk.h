/* FIFO read/write access */
#define REGFIFO             0x00

/* Operating mode & LoRaTM / FSK selection */
#define REGOPMODE           0x01

/* Bit Rate setting */
#define REGBITRATE_MSB      0x02
#define REGBITRATE_LSB      0x03

/* Frequency Deviation setting */
#define REGFDEV_MSB         0x04
#define REGFDEV_LSB         0x05

/* RF Carrier Frequency */
#define REGFRF_MSB          0x06
#define REGFRF_MID          0x07
#define REGFRF_LSB          0x08

/* PA selection and Output Power control */
#define REGPACONFIG         0x09

/* Control of PA ramp time, low phase noise PLL */
#define REGOCP              0x0b

/* Over Current Protection control */
#define REGPARAMP           0x0a

/* LNA settings */
#define REGLNA              0x0c

/* AFC, AGC, ctrl */
#define REGRXCONFIG         0x0d

/* RSSI */
#define REGRSSICONFIG       0x0e

/* RSSI Collision detector */
#define REGRSSICOLLISION    0x0f

/* RSSI Threshold control */
#define REGRSSITHRESH       0x10

/* RSSI value in dBm */
#define REGRSSIVALUE        0x11

/* Channel Filter BW Control */
#define REGRXBW             0x12

/* AFC Channel Filter BW */
#define REGAFCBF            0x13

/* OOK demodulator */
#define REGOOKPEAK          0x14

/* Threshold of the OOK demod */
#define REGOOKFIX           0x15

/* Average of the OOK demod */
#define REGOOKAVG           0x16

/* AFC and FEI control */
#define REGAFCFEI           0x1a

/* Frequency correction value of the AFC */
#define REGAFC_MSB          0x1b
#define REGAFC_LSB          0x1c

/* Value of the calculated frequency error */
#define REGFEI_MSB          0x1d
#define REGFEI_LSB          0x1e

/* Settings of the Preamble Detector */
#define REGPREAMBLEDETECT   0x1f

/* Timeout Rx request and RSSI */
#define REGRXTIMEOUT1       0x20
/* Timeout RSSI and PayloadReady */
#define REGRXTIMEOUT2       0x21
/* Timeout RSSI and SyncAddress */
#define REGRXTIMEOUT3       0x22

/* Delay between Rx cycles */
#define REGRXGELAY          0x23

/* RC Oscillators Settings, CLKOUT frequency */
#define REGOSC              0x24

/* Preamble length */
#define REGPREAMBLE_MSB     0x25
#define REGPREAMBLE_LSB     0x26

/* Sync Word Recognition control */
#define REGSYNCCONFIG       0x27

/* Sync Word bytes */
#define REGSYNCVALUE1       0x28
#define REGSYNCVALUE2       0x29
#define REGSYNCVALUE3       0x2a
#define REGSYNCVALUE4       0x2b
#define REGSYNCVALUE5       0x2c
#define REGSYNCVALUE6       0x2d
#define REGSYNCVALUE7       0x2e
#define REGSYNCVALUE8       0x2f

/* Packet mode settings */
#define REGPACKETCONFIG1    0x30
#define REGPACKETCONFIG2    0x31

/* Payload length setting */
#define REGPAYLOADLENGTH    0x32

/* Node address */
#define REGNODEADDRS        0x33

/* Broadcast address */
#define REGBROADCASTADDRS   0x34

/* Fifo threshold, Tx start condition */
#define REGFIFOTHRESH       0x35

/* Top level Sequencer settings */
#define REGSEQCONFIG1       0x36
#define REGSEQCONFIG2       0x37

/* Timer 1 and 2 resolution control */
#define REGTIMERRESOL       0x38

/* Timer setting */
#define REGTIMER1COEF       0x39
#define REGTIMER2COEF       0x3a

/* Image calibration engine control */
#define REGIMAGECAL         0x3b

/* Temperature Sensor value */
#define REGTEMP             0x3c

/* Low Battery Indicator Settings */
#define REGLOWBAT           0x3d

/* Status register: PLL Lock state, Timeout, RSSI */
#define REGIRQFLAGS1        0x3e
/* Status register: FIFO handling flags, Low Battery*/
#define REGIRQFLAGS1        0x3f

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
