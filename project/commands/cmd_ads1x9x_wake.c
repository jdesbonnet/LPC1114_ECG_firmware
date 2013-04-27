#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"

void cmd_ads1x9x_wake (uint8_t argc, char **argv)
{
	ads1x9x_command(CMD_WAKEUP);
}
