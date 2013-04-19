#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"

void cmd_ads1x9x_ch_cfg(uint8_t argc, char **argv)
{
	int ch = atoi(argv[0]);
	int what = atoi(argv[1]);

	switch (ch) {
		case 1:
			ads1x9x_register_write (REG_CH1SET, what);
			break;
		case 2:
			ads1x9x_register_write (REG_CH2SET, what);
			break;
		default:
			printf ("ERROR: invalid channel\r\n");
	}


}
