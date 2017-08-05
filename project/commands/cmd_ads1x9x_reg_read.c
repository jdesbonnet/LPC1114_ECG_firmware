#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "stream_encode.h"
#include "parse_hex.h"
#include "ads1x9x.h"
#include "cmd_ads1x9x.h"

void cmd_ads1x9x_reg_read(uint8_t argc, char **argv)
{
	int regId = parse_dec_or_hex(argv[0]);
	int v = ads1x9x_register_read(regId);

	if (cmd_ads1x9x_is_binary_mode()) {
		stream_write_start();
		stream_write_byte(0x02);
		stream_write_byte(0);
		stream_write_byte(0);
		stream_write_byte(0);
		stream_write_byte( v & 0xff);
	} else {
		printf ("%x\r\n", v);
	}

}
