#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "core/systick/systick.h"
#include "commands.h"
#include "parse_hex.h"



void cmd_xbee(uint8_t argc, char **argv)
{

	printf ("Sending AT%s\r\n", argv[0]);

	// Wait guard period (1s)
	systickDelay(1100); // Delay 1.1s

	// Escape sequence
	printf("+++");

	// Wait guard period (1s)
	systickDelay(1100); // Delay 1.1s

	// Send AT command
	printf ("AT%s\r\n",argv[0]);

	// Read response
	// Currently not possible as response will be interpreted as a command.
	// However the commands and errors will be echoed back over the air.

	// Exit command mode (ATCN)
	printf ("ATCN\r\n");

	// Echo back response after exiting AT mode
	
}
