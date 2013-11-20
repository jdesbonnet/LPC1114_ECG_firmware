#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "parse_hex.h"
#include "commands.h"
#include "ads1x9x.h"
#include "cmd_ads1x9x.h"


void cmd_ads1x9x_test(uint8_t argc, char **argv)
{
	int pga_gain,pga_bits,i,j;

	// PGA gains to run the test for
	int test_gains[] = {1,4,8};

	// Bits[2:0] = 2 (500 kSPS)
	ads1x9x_register_write (REG_CONFIG1, 0x02);

	ads1x9x_command(CMD_START);
	ads1x9x_command(CMD_SDATAC); 

	for (i = 0; i < 3; i++) {
		pga_gain = test_gains[i];

		printf ("PGA Gain %d\r\n", pga_gain);

		// 
		// Measure shorted inputs
		//


		// Using internal reference. Set PDB_REFBUF=1 and wait for ref to settle
		// Bit 7: always 1
		// Bit 6: PDB_LOFF_COMP = 0 (Lead-off comparitor power-down)
		// Bit 5: PDB_REFBUF = 1 (reference buffer enabled)
		// Bit 4: VREF_4V = 0 (reference set to 2.42V)
		// Bit 3: CLK_EN = 0 (oscillator clock output disabled)
		// Bit 2: always 0
		// Bit 1: INT_TEST = 0 (test signal off)
		// Bit 0: TEST_FREQ = 0 (DC)
		ads1x9x_register_write (REG_CONFIG2, 0xA0);



		// Set gain and mux=short
		pga_bits = ads1x9x_get_pga_bits(pga_gain);
		ads1x9x_register_write(REG_CH1SET, 0x01 | (pga_bits<<4));
		ads1x9x_register_write(REG_CH2SET, 0x01 | (pga_bits<<4));

ads1x9x_register_write(4,0x11);
ads1x9x_register_write(5,0x11);

		ads1x9x_command(CMD_OFFSETCAL); 
		delay(2000000);


		for (j = 0; j < 3; j++) {
			printf ("Iter %d: ", j);
			ads1x9x_measure_shorted();
		}

		//
		// Measure test signal
		//

		// Activate a 1mV x Vref/2.4 square wave test signal
		ads1x9x_register_write(REG_CONFIG2, 0xA3);

		// Set gain and mux=test
		int pga_bits = ads1x9x_get_pga_bits(pga_gain);
		ads1x9x_register_write(REG_CH1SET, 0x05 | (pga_bits<<4));
		ads1x9x_register_write(REG_CH2SET, 0x05 | (pga_bits<<4));
		ads1x9x_command(CMD_OFFSETCAL); 
		delay(2000000);

		ads1x9x_measure_test_signal(); 

		printf ("----------------------------------------\r\n");

	}

	ads1x9x_command(CMD_SDATAC); 
	ads1x9x_command(CMD_STOP);

}


