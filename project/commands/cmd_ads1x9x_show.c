#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "core/systick/systick.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_show(uint8_t argc, char **argv)
{
	
	if (strncmp("SYSTICK",argv[0],7)==0) {
		printf ("%x",systickGetTicks());
	}

	if (strncmp("CONFIG",argv[0],6)==0) {
		int i;
		for (i = 0; i < MAX_CONFIG; i++) {
			printf ("config[%d] = %x\r\n",i,config_var[i]);
		}
	}

}
