#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"

void cmd_ads1x9x_reg_write(uint8_t argc, char **argv)
{
	int regId = parse_hex(argv[0]);
	int regVal = parse_hex (argv[1]);
	printf ("regId=%x val=%x\r\n",regId,regVal);
	ads1x9x_register_write(regId, regVal);
}
