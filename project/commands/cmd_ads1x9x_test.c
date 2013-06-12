#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_test(uint8_t argc, char **argv)
{
	int pga_gain,i,j;

	// PGA gains to run the test for
	int test_gains[] = {1,4,8};

	for (i = 0; i < 3; i++) {
		pga_gain = test_gains[i];

		printf ("PGA Gain %d\r\n", pga_gain);
		for (j = 0; j < 3; j++) {
			printf ("Iter %d: ", j);
			ads1x9x_measure_shorted(pga_gain);
		}
		ads1x9x_measure_test_signal(pga_gain); 

		printf ("OFSETCAL\r\n");
		ads1x9x_command(CMD_OFFSETCAL); 
		delay(2000000);

		for (j = 0; j < 3; j++) {
			printf ("Iter %d: ", j);
			ads1x9x_measure_shorted(pga_gain);
		}

		ads1x9x_measure_test_signal(pga_gain);	

		printf ("----------------------------------------\r\n");

	}


}


