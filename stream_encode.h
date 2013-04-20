#ifndef __ADS1X9X_H__ 
#define __ADS1X9X_H__

#include "core/gpio/gpio.h"

#define STREAM_ESCAPE (0x7D)
#define STREAM_START (0x12)

void stream_write_start ();
void stream_write_byte (uint8_t b);
void stream_write_bytes (uint8_t *buf, int nbytes);

#endif
