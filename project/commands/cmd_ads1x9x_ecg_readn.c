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

void cmd_ads1x9x_ecg_readn (uint8_t argc, char **argv)
{
	int recordIndex=1,status;
	uint8_t buf[12];

	uint32_t i;
	uint32_t n = atoi (argv[0]);

	char outputFormat = argv[1][0];

	#ifdef SINGLE_SHOT_MODE
	// SINGLE_SHOT=1, 500SPS
	//ads1x9x_register_write(REG_CONFIG1, 0x82);
	#else
	// SINGLE_SHOT=0, 500SPS
	ads1x9x_register_write(REG_CONFIG1, 0x02);
	ads1x9x_command(CMD_RDATAC);
	#endif


	// Write number of records to SRAM
	if (outputFormat == STORE_TO_SRAM) {
		sram_record_write(0,&n,sizeof(n));
		// Read back to verify write
		sram_record_read(0,&i,sizeof(n));
		if (i != n) {
			printf ("ERROR: write to SRAM failed verify test, expected %d, but got %d\r\n", n, i);
		}
	}

	while (n--) {

		#ifdef SINGLE_SHOT_MODE
		// Issue command to start data conversion
		ads1x9x_command(CMD_START);
		#endif


		// Wait for data available
		status = ads1x9x_drdy_wait(1000000);
		if (status != 0) {
			printf ("TIMEOUT\r\n");
			break;
		}

		#ifdef SINGLE_SHOT_MODE
		// Issue command to read ECG data
		ads1x9x_command(CMD_RDATA);
		#endif

		ads1x9x_ecg_read (&buf);

		// Record comprises 24bit status + 2 x 24bit channel data
		// Status is 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000 0000 0000
		status = ((buf[0]<<8 | buf[1])>>5)&0xff;

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

			case STORE_TO_SRAM:
			// Format SRAM record in buf[]
			delay(1000);
			buf[1] = 0x00;
			buf[2] = status;
			sram_record_write(recordIndex*8,buf+1,8);
			recordIndex++;
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


	#ifndef SINGLE_SHOT_MODE
	//ads1x9x_command(CMD_SDATAC);
	#endif

}
