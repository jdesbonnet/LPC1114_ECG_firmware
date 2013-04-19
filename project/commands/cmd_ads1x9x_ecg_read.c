#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"

void cmd_ads1x9x_ecg_read(uint8_t argc, char **argv)
{
	int i;
	uint8_t buf[9];

	ads1x9x_command(CMD_START);

	ads1x9x_drdy_wait(0);

	ads1x9x_command(CMD_RDATA);

	ads1x9x_ecg_read (&buf);

	for (i = 0; i < 9; i++) {
		printf ("%x ", buf[i]);
	}
	printf ("\r\n");

}
