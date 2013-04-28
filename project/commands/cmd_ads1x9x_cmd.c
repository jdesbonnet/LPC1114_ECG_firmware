#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_cmd(uint8_t argc, char **argv)
{
	int cmd;
	if (strncmp(argv[0],"START",5)==0) {
		cmd = CMD_START;
	} else if (strncmp(argv[0],"STANDBY",7)==0) {
		cmd = CMD_STANDBY;
	} else if (strncmp(argv[0],"WAKEUP",6)==0) {
		cmd = CMD_WAKEUP;
	}

	ads1x9x_command(cmd);
	
}
