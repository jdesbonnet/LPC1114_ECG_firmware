#ifndef _ADAS1000_H_
#define _ADAS1000_H_
void adas1000_init(void);
void adas1000_register_write (uint8_t reg, uint32_t value);
uint32_t adas1000_register_read (uint8_t reg);
void adas1000_testtone_enable(void);
void adas1000_power_off (void);

void adas1000_ecg_capture (uint32_t nsamples);
void adas1000_ecg_playback (uint32_t nsamples);

#endif
