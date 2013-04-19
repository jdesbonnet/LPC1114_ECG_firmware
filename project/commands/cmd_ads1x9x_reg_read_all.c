
#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"       // Generic helper functions
#include "core/adc/adc.h"
#include "core/systick/systick.h"
#include "core/gpio/gpio.h"
#include "core/iap/iap.h"
#include "ads1x9x.h"

void cmd_ads1x9x_reg_read_all(uint8_t argc, char **argv)
{
	int i;


	// Read all registers
	for (i = 0; i < 12; i++) {
		printf ("%x ", ads1x9x_register_read(i));
	}
	printf ("\r\n");

}
