
/**************************************************************************/
/*! 
    @file     sram_23a1024.c
    @author   Joe Desbonnet, jdesbonnet@gmail.com

    @brief    Code load/store from Microchip 23A1024, 23LC1024

*/
/**************************************************************************/
#include <stdint.h>

#include "core/gpio/gpio.h"
#include "core/ssp/ssp.h"

#include "sram_23a1024.h"

void sram_select(void) {
	// Assert SRAM /CS line
	gpioSetValue(0,3,0);
	// Clock phase is different to ADS1x9x device, so set on each select.
	sspInit(0, sspClockPolarity_Low, sspClockPhase_RisingEdge);
}
void sram_deselect(void) {
	gpioSetValue(0,3,1);
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

	sram_deselect();
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

	sram_deselect();
}

int sram_test (void) {

	//#ifdef SRAM_23LC1024

	int i,j;
	uint8_t request[8];
	uint8_t response[8];

	// 
	// Test byte mode read/write
	//

	// Set byte mode
	sram_select();
	request[0] = 0x01;  // WRMR command
	request[1] = 0x00; // Byte mode
	sspSend(0, (uint8_t *)&request, 2);
	sram_deselect();

	// Test by writing to memory
	sram_select();
	request[0] = 0x02;  // write command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	request[4] = 0x55;
	sspSend(0, (uint8_t *)&request, 5);
	sram_deselect();

	// Read it back
	sram_select();
	request[0] = 0x03;  // read command
	request[1] = 0x00;  // addr 0x000000
	request[2] = 0x00;  //
	request[3] = 0x00;  //
	sspSend(0, (uint8_t *)&request, 4);
	sspReceive (0, (uint8_t *)&response, 1);
	sram_deselect();

	if (response[0] != 0x55) {
		printf ("SRAM test fail: expecting 0x55, got %d\r\n", response[0]);
		return -1;
	}

	//
	// Test sequential mode read/write
	//

	// Set sequential mode
	sram_select();
	request[0] = 0x01; // WRMR command
	request[1] = 0x40; // Sequential mode
	sspSend(0, (uint8_t *)&request, 2);
	sram_deselect();


	// Write to memory
	sram_select();
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
	sram_deselect();

	// Read back	
	sram_select();
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
	sram_deselect();
	if (j!=0) {
		return -1;
	}

	//#endif

	// SUCCESS
	return 0;
	
}


