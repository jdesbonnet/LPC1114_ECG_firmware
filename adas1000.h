#ifndef _ADAS1000_H_
#define _ADAS1000_H_
void adas1000_init(void);
void adas1000_register_write (uint8_t reg, uint32_t value);
uint32_t adas1000_register_read (uint8_t reg);
void adas1000_testtone_enable(int toneType);
void adas1000_testtone_disable (void);
void adas1000_power_off (void);

void adas1000_ecg_capture (uint32_t nsamples);
void adas1000_ecg_capture_stop (void);
void adas1000_ecg_playback (uint32_t nsamples);
void adas1000_reset(void);

// TESTTONE [4:3]
#define TONETYPE_MASK (0x18)
#define TONETYPE_10HZ_SINE (0x00)
#define TONETYPE_150HZ_SINE (0x08)
#define TONETYPE_1HZ_SQUARE (0x10)

#endif