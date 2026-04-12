#ifndef SRIO_H
#define SRIO_H

#include <ti/csl/csl_types.h>
#include <ti/csl/csl_srio.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>

//Byte0-7:5A5A 5A5A 5A5A 5A5A, byte8:start01 stop00, byte9-10: freq/10, byte11:Amp, byte12-63:reserve
typedef struct{
	uint8_t frameHead[8];
	uint8_t ADC_sampEnable;
	uint8_t DAC_outEnable;
	uint16_t DAC_freq;
	uint8_t DAC_amp;
	uint8_t extractFlag;
	uint8_t reserved[50];
}FPGAParams;

// Device parameter structure
#pragma pack(push, 1)
typedef struct {
    uint8_t  algorithmMode;       // Algorithm mode  0��MAD + LMS, 1: MAD + FFT_SLIP
    uint8_t  fft_time_window;     // FFT time window (0: 0.5s, 1: 1s, 2: 2s)

    uint16_t  MADThreshold; // MAD threshold   actual = raw/1000
    uint16_t  LMSThreshold;  // LMS threshold  actual = raw/1000
    uint16_t  FFTThreshold;  // FFT threshold  actual = raw/1000

    uint8_t  MADTimes;  // MAD times
    uint8_t  LMSTimes;  // LMS times
    uint8_t  FFTTimes;  // FFT times

    uint16_t  signalFrequency;    // Signal frequency
    uint8_t  effectiveValue;      // Effective value (0-128)

    uint8_t LMSOrder;
    uint8_t LMSStep; //���� /1000

    uint16_t MADStopFreq;   //0.1Hz, 0.5Hz, 1.0Hz corresponding to 0, 1, 2

    //uint16_t  sensitivity_Fluxgate;        // Sensitivity coefficient
    //uint16_t  sensitivity_induction;        // Sensitivity coefficient

    uint8_t  reserve[36];         // Reserved
} DeviceParams;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint16_t targetStatus;
	uint16_t targetSNR;
} TargetStatus;
#pragma pack(pop)

extern DeviceParams devParams;

extern volatile FPGAParams fpgaParams;

extern volatile TargetStatus targetStatus;

extern volatile float madThresholdSquare;

extern CSL_SrioHandle hSrio;

/* ISR and public API */
void srio_db_isr(UArg arg);
int  srio_interrupt_init(void);
Int32 srio_device_init(void);
int  init_srio(void);
void srio_data_task(UArg arg0, UArg arg1);

int32_t srio_send(void);
int32_t srio_receive(uint32_t *local_addr, uint32_t size);

#endif /* SRIO_H */
