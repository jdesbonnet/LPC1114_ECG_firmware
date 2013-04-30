#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"
#include "sram_23a1024.h"

//#define SINGLE_SHOT_MODE

#define OUTPUT_BINARY  ('B')
#define OUTPUT_TEXT  ('A')
#define STORE_TO_SRAM  ('S')

/**
 * Playback a recorded ECG from SRAM memory.
 */
void cmd_ads1x9x_ecg_playback (uint8_t argc, char **argv)
{
	int recordIndex=1,status;
	uint8_t buf[12];

	char outputFormat = argv[0][0];

	uint32_t n;
	sram_record_read(0,&n,sizeof(n));

	printf ("Found %u ECG records.\r\n", (unsigned int)n);
	

	while (n--) {

		sram_record_read(recordIndex*8, buf+1, 8);
		status = buf[2];
		switch (outputFormat) {
			case OUTPUT_BINARY:
			stream_write_start();
			stream_write_byte(0x01); // ECG record frame
			// Lead Off Status + GPIO
			stream_write_byte(status);
			stream_write_bytes(buf+3,6);
			break;

			case OUTPUT_TEXT:
			printf ("s=%x ", status );
			printf ("ch1=%x ", (buf[3]<<16 | buf[4]<<8 | buf[5]) );
			printf ("ch2=%x ", (buf[6]<<16 | buf[7]<<8 | buf[8]) );
			printf ("\r\n");
			break;
		}

		cmdPoll();

		// Check for ESC from host
		if (cmdIsEscape()) {
			cmdResetEscape();
			printf ("\r\nESCAPE\r\n");
			break;
		}

	}

}
