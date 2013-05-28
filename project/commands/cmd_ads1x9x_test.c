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
	// Page 63 of data sheet

	uint8_t buf[9];

	// Stop continuous data to allow register read.
	ads1x9x_command (CMD_SDATAC);

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

	// Bits[2:0] = 2 (500 kSPS)
	ads1x9x_register_write (REG_CONFIG1, 0x02);

	// Set channels to input short
	ads1x9x_register_write (REG_CH1SET, 0x01);
	ads1x9x_register_write (REG_CH2SET, 0x01);

	ads1x9x_command (CMD_RDATAC);

	if ( ads1x9x_drdy_wait (10000) == -1 ) {
		printf ("ERROR 1\n");
		return;
	}


	ads1x9x_ecg_read (buf);

	ads1x9x_command (CMD_SDATAC);

	// Display ECG record
	print_ecg_record (buf);

	// Activate a 1mv x Vref/2.4 square wave test signal
	ads1x9x_register_write(REG_CONFIG2, 0xA3);
	ads1x9x_register_write(REG_CH1SET, 0x05);
	ads1x9x_register_write(REG_CH2SET, 0x05);

	ads1x9x_command (CMD_RDATAC);

	int i;
	uint32_t ch1,ch2;
	uint64_t ch1_sum=0, ch2_sum=0;
	uint32_t min_ch1 = (1<<24);
	uint32_t max_ch1 = 0;
	uint32_t min_ch2 = (1<<24);
	uint32_t max_ch2 = 0;

	for (i = 0; i < 2000; i++) {
		if ( ads1x9x_drdy_wait (10000) == -1 ) {
			printf ("ERROR 1\n");
			return;
		}

		ads1x9x_ecg_read (buf);
		ch1 = (buf[3]<<16 | buf[4]<<8 | buf[5]);
		ch2 = (buf[6]<<16 | buf[7]<<8 | buf[8]);

		ch1_sum += ch1;
		ch2_sum += ch2;

		if (ch1 > max_ch1) {
			max_ch1 = ch1;
		}
		if (ch1 < min_ch1) {
			min_ch1 = ch1;
		}
		if (ch2 > max_ch2) {
			max_ch2 = ch2;
		}
		if (ch2 < min_ch2) {
			min_ch2 = ch2;
		}
		
	}

	int ch1_mean = (uint32_t)(ch1_sum/2000);
	int ch2_mean = (uint32_t)(ch2_sum/2000);
	printf ("min_ch1=%u max_ch1=%u range=%u mean=%u\r\n", min_ch1, max_ch1, (max_ch1-min_ch1), ch1_mean );
	printf ("min_ch2=%u max_ch2=%u range=%u mean=%u\r\n", min_ch2, max_ch2, (max_ch2-min_ch2), ch2_mean );

}
