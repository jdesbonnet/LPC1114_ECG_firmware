#include <stdio.h>
#include <stdlib.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "stream_encode.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_temp_read (uint8_t argc, char **argv)
{
	int i,status,tC;
	int32_t ch1;
	uint8_t buf[12];
	
	double mV;

	ads1x9x_register_write (REG_CONFIG2, 0xA0);
	ads1x9x_register_write (REG_CH1SET, 0x14);

	delay (102400);

	ads1x9x_command(CMD_START);
	ads1x9x_drdy_wait(0);

	ads1x9x_command(CMD_RDATA); 

	ads1x9x_ecg_read (&buf);

	// Record comprises 24bit status + 2 x 24bit channel data
	// Status is 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000 0000 0000
	status = ((buf[0]<<8 | buf[1])>>5)&0xff;

	ch1 = ((buf[3]<<16 | buf[4]<<8 | buf[5]) << 8)/256;

	int64_t uv = ((int64_t)ch1*(int64_t)2420000)>>23;

	// Temperature in °C*100
	// From data sheet: Tc = (ch1 (µV) - 145300µV)/490µV + 25°C
	int64_t tc = (uv*10-(int64_t)1453000)/(int64_t)49 + (int64_t)2500;


	//cmd_ads1x9x_flags++;

	printf ("mode=%d\r\n", cmd_ads1x9x_flags);

	if (cmd_ads1x9x_is_binary_mode()) {
		// Cast to 32 bit int and write temp packet
		int32_t tint = (int32_t)tc;
		stream_write_start();
		stream_write_byte(0x02);
		stream_write_byte( (tint>>24) & 0xff);
		stream_write_byte( (tint>>16) & 0xff);
		stream_write_byte( (tint>>8) & 0xff);
		stream_write_byte( tint & 0xff);
	} else {
		printf ("ch1=0x%x ch1=%d uv=%d tc=%d\r\n",ch1,ch1,(int)uv,(int)tc);
	}


}
