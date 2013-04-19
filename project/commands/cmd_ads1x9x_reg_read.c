#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"

void cmd_ads1x9x_reg_read(uint8_t argc, char **argv)
{
	int regId = atoi(argv[0]);
	printf ("%x\r\n", ads1x9x_register_read(regId));
}
