#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"

void cmd_ads1x9x_ecg_readn (uint8_t argc, char **argv)
{
	int i;
	uint8_t buf[12];
	uint32_t *v;

	int n = atoi (argv[0]);

	while (n--) {

		ads1x9x_command(CMD_START);

		ads1x9x_drdy_wait(0);

		ads1x9x_command(CMD_RDATA); 

		ads1x9x_ecg_read (&buf);

		v = &buf;
		printf ("h=%x ", (__builtin_bswap32(*v)>>8) );

		i = buf[3]<<16 | buf[4]<<8 | buf[5];
		printf ("ch1=%x ", i );


		i = buf[6]<<16 | buf[7]<<8 | buf[8];
		printf ("ch2=%x ", i );

		printf ("\r\n");
	}

}
