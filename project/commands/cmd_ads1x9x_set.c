#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

typedef int (*FN)();

void cmd_ads1x9x_set(uint8_t argc, char **argv)
{
	// Is first param 'MODE'?
	//if (argv[0][0] == 'M' && argv[0][1] == 'O' && argv[0][2] == 'D' && argv[0][3] == 'E') {
	if (strncmp("MODE",argv[0],4)==0) {

		if (argc>=2 && argv[1][0]=='B') {
			printf ("MODE BINARY\r\n");
			cmd_ads1x9x_flags = 1;
		}
		if (argc>=2 && argv[1][0]=='A') {
			printf ("MODE ASCII\r\n");
			//cmd_ads1x9x_flags &= ~(0x1);
			cmd_ads1x9x_flags = 0;
		}
	//} else if (argv[0][0] == 'T' && argv[0][1] == 'E' && argv[0][2] == 'S' && argv[0][3] == 'T') {
	} else if (strncmp("TEST",argv[0],4)==0) {
		if (argc>=2 && argv[1][0]=='O' && argv[1][1] == 'N') {
			// Gain=1, Set MUX to connect test signal to CH1, CH2
			ads1x9x_register_write (REG_CH1SET, 0x05);
			ads1x9x_register_write (REG_CH2SET, 0x05);
			// CONFIG2: 
			// PDB_LOFF_COMP=1 (lead off comp enabled), PDB_REFBUF=1 (ref buf en), 
			// VREF_4V=0, CLK_EN=1, int test
			ads1x9x_register_write (REG_CONFIG2, 0xA3);
			printf ("TEST SIGNAL ON\r\n");
		} else if (argc>=2 && argv[1][0]=='O' && argv[1][1] == 'F') {
			ads1x9x_register_write (REG_CH1SET, 0x00);
			ads1x9x_register_write (REG_CH2SET, 0x00);
			// Normal ECG mode
			printf ("TEST SIGNAL OFF\r\n");
		}
 
	} else if (strncmp("INT",argv[0],3)==0) {
		if (argc>=2 && argv[1][0]=='O' && argv[1][1] == 'N') {
			gpioIntEnable(RADIO_INT_PORT, RADIO_INT_PIN);
			printf ("INTERRUPT ON\r\n");
		} else if (argc>=2 && argv[1][0]=='O' && argv[1][1] == 'F') {
			gpioIntDisable(RADIO_INT_PORT, RADIO_INT_PIN);
			printf ("INTERRUPT OFF\r\n");
		} else {
			printf ("?\r\n");
		}


	} else if (strncmp("CONFIG",argv[0],2)==0) {
		// Set program counter (Jump to location)
		if (argc>=3 ) {
			uint32_t config_var_index = parse_dec_or_hex(argv[1]);
			config_var[config_var_index] = parse_dec_or_hex(argv[2]);
		}

	} else if (strncmp("PC",argv[0],2)==0) {
		// Set program counter (Jump to location)
		if (argc>=2 ) {
			uint32_t pc = parse_dec_or_hex(argv[1]);
			FN fnptr = (FN)(pc);
			// Call function at location 'pc'
			printf ("Jumping to %x\r\n", (int)pc);
			fnptr();
			//function *fnptr = pc;
		}
	} else if (strncmp("UART",argv[0],4)==0) {
		if (argc>=2) {
			int speed = parse_dec_or_hex(argv[1]);
			uartInit(speed);
		}
	}

}
