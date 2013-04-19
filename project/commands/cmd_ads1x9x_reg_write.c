#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"

void cmd_ads1x9x_reg_write(uint8_t argc, char **argv)
{
	int regId = atoi(argv[0]);
	int regVal = atoi(argv[1]); // want hex!
	printf ("regId=%x val=%x\r\n",regId,regVal);
	ads1x9x_register_write(regId, regVal);
}
