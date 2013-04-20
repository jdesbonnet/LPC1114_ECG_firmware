#include "projectconfig.h"
#include "sysinit.h"
#include "lpc111x.h"
#include "stream_encode.h"

void stream_write_start (void) {
	uartSendByte(STREAM_ESCAPE);
	uartSendByte(STREAM_START);
}

void stream_write_byte (uint8_t b) {

	if (b == STREAM_ESCAPE) {
		uartSendByte (STREAM_ESCAPE);
		uartSendByte (STREAM_ESCAPE ^ 0x20);
	} else {
		uartSendByte (b);
	}

}
void stream_write_bytes (uint8_t *buf, int nbytes) {
	int i;
	for (i = 0; i < nbytes; i++) {
		stream_write_byte(buf[i]);
	}
}


