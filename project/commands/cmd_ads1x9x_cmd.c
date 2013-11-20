#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

/**
 * Send ADS1x9x commands to ADS1x9x IC.
 */
void cmd_ads1x9x_cmd(uint8_t argc, char **argv)
{
	int cmd;
	if (strncmp(argv[0],"STANDBY",7)==0) {
		cmd = CMD_STANDBY;
	} else if (strncmp(argv[0],"WAKEUP",6)==0) {
		cmd = CMD_WAKEUP;
	} else if (strncmp(argv[0],"RESET",5)==0) {
		cmd = CMD_RESET;
	} else if (strncmp(argv[0],"START",5)==0) {
		cmd = CMD_START;
	} else if (strncmp(argv[0],"STOP",4)==0) {
		cmd = CMD_STOP;
	} else if (strncmp(argv[0],"OFFSETCAL",9)==0) {
		cmd = CMD_OFFSETCAL;
	} else if (strncmp(argv[0],"RDATAC",6)==0) {
		cmd = CMD_RDATAC;
	} else if (strncmp(argv[0],"SDATAC",6)==0) {
		cmd = CMD_SDATAC;
	} else if (strncmp(argv[0],"RDATA",5)==0) {
		cmd = CMD_RDATA;
	} else {
		printf ("ERROR: unrecognized command\r\n");
	}

	// RREG, WREG handled as separate commands.

	ads1x9x_command(cmd);
	
}
