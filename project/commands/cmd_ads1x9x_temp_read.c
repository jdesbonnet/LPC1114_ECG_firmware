#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"

void cmd_ads1x9x_temp_read (uint8_t argc, char **argv)
{
	int i,status;
	uint8_t buf[12];

	ads1x9x_register_write (REG_CONFIG2, 0xA0);
	ads1x9x_register_write (REG_CH1SET, 0x14);
	ads1x9x_register_write (REG_CH2SET, 0x14);

	ads1x9x_command(CMD_START);
	ads1x9x_drdy_wait(0);

	ads1x9x_command(CMD_RDATA); 

	ads1x9x_ecg_read (&buf);

	// Record comprises 24bit status + 2 x 24bit channel data
	// Status is 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000 0000 0000
	status = ((buf[0]<<8 | buf[1])>>5)&0xff;

		
	printf ("s=%x ", status );
	printf ("ch1=%x ", (buf[3]<<16 | buf[4]<<8 | buf[5]) );
	printf ("ch2=%x ", (buf[6]<<16 | buf[7]<<8 | buf[8]) );
	printf ("\r\n");
		

}
