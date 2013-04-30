/* */

#include <stdint.h>

#include "core/gpio/gpio.h"

#include "sram_23a1024.h"

void sram_select(void) {
	// Assert SRAM /CS line
	gpioSetValue(1,3,0);
}
void sram_deselect(void) {
	gpioSetValue(1,3,0);
}

void sram_record_write (uint32_t base_addr, uint8_t *buf, uint32_t record_size) {
	uint8_t request[4];
	sram_select();
	request[0] = 0x02;  // write command
	request[1] = (base_addr>>16)&0xff; 
	request[2] = (base_addr>>8)&0xff;
	request[3] = base_addr & 0xff;
	sspSend(0, (uint8_t *)&request, 4);

	// Write record to memory
	sspSend(0, buf, record_size);

	sramDeselect();
}

void sram_record_read (uint32_t base_addr, uint8_t *buf, uint32_t record_size) {
	uint8_t request[4];
	sram_select();

	request[0] = 0x03;  // read command
	request[1] = (base_addr>>16)&0xff; 
	request[2] = (base_addr>>8)&0xff;
	request[3] = base_addr & 0xff;
	sspSend(0, (uint8_t *)&request, 4);

	sspReceive (0, buf, record_size);

	sramDeselect();
}

int sram_test (void) {

	#ifdef SRAM_23LC1024

	// 
	// Test byte mode read/write
	//

	// Set byte mode
	sramSelect();
	request[0] = 0x01;  // WRMR command
	request[1] = 0x00; // Byte mode
	gpioSetValue(1,8,1);
	sspSend(0, (uint8_t *)&request, 2);
	sramDeselect();

	// Test by writing to memory
	sramSelect();
	request[0] = 0x02;  // write command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	request[4] = 0x55;
	sspSend(0, (uint8_t *)&request, 5);
	sramDeselect();

	// Read it back
	sramSelect();
	request[0] = 0x03;  // read command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	sspReceive (0, (uint8_t *)&response, 1);
	sramDeselect();

	printf ("Memory test:\n ");
	if (response[0] != 0x55) {
		return -1;
	}

	//
	// Test sequential mode read/write
	//

	// Set sequential mode
	sramSelect();
	request[0] = 0x01; // WRMR command
	request[1] = 0x40; // Sequential mode
	sspSend(0, (uint8_t *)&request, 2);
	sramDeselect();


	// Write to memory
	sramSelect();
	request[0] = 0x02;  // write command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	j=0;
	for (i = 1; i < 1999; i++) {
		request[0] = i;
		sspSend(0, (uint8_t *)&request, 1);
		j += i;
	}
	sramDeselect();

	// Read back	
	sramSelect();
	request[0] = 0x03;  // read command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	for (i = 1; i < 1999; i++) {
		request[0] = i;
		sspReceive(0, (uint8_t *)&response, 1);
		j -= i;
	}
	sramDeselect();
	if (j!=0) {
		return -1;
	}

	#endif

	return 0;
	
}


