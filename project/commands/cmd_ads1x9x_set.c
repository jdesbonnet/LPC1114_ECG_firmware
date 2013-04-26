#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_set(uint8_t argc, char **argv)
{
	// Is first param 'MODE'?
	if (argv[0][0] == 'M' && argv[0][1] == 'O' && argv[0][2] == 'D' && argv[0][3] == 'E') {

		if (argc>=2 && argv[1][0]=='B') {
			printf ("MODE BINARY\r\n");
			cmd_ads1x9x_flags = 1;
		}
		if (argc>=2 && argv[1][0]=='A') {
			printf ("MODE ASCII\r\n");
			//cmd_ads1x9x_flags &= ~(0x1);
			cmd_ads1x9x_flags = 0;
		}
	}
}
