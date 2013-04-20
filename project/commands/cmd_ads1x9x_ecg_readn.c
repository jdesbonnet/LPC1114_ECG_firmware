#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

void cmd_ads1x9x_ecg_readn (uint8_t argc, char **argv)
{
	int i;
	uint8_t buf[12];

	bool binary_flag = false;

	for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1]=='b') {
				binary_flag = true;
			}
		}
	}

	int n = atoi (argv[0]);

	ads1x9x_register_write (REG_CONFIG2, 0xA3);
	ads1x9x_register_write (REG_CH2SET, 0x05);

	while (n--) {

		ads1x9x_command(CMD_START);

		ads1x9x_drdy_wait(0);

		ads1x9x_command(CMD_RDATA); 

		ads1x9x_ecg_read (&buf);

		if (binary_flag) {
			stream_write_start();
			stream_write_bytes(buf,9);
		} else {

			printf ("h=%x ", (buf[0]<<16 | buf[1]<<8 | buf[2]) );
			printf ("ch1=%x ", (buf[3]<<16 | buf[4]<<8 | buf[5]) );
			printf ("ch2=%x ", (buf[6]<<16 | buf[7]<<8 | buf[8]) );
			printf ("\r\n");
		}

		cmdPoll();

		if (cmdIsEscape()) {
			cmdResetEscape();
			printf ("\r\nESCAPE\r\n");
			break;
		}

	}

}
