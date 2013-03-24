#include "projectconfig.h"
#include "core/ssp/ssp.h"
#include "adas1000.h"

uint32_t reverse_byte_order (uint32_t in) {
	return (in>>24) | ((in>>8)&0x0000ff00) | ((in<<8)&0x00ff0000) | (in<<24);
}

void sramSelect () {
	gpioSetValue(0,3,0);
}
void sramDeselect () {
	gpioSetValue(0,3,1);
}

void adas1000_init (void) {

	// Write to CMREFCTL Common Mode, Reference and Shield Drive Control Register
	// Bit [23:19] common mode electrode select. Selecting mean of LA, LL, RA.
	// Bit [7:4] = 0000 : select RLD_OUT pin for reference drive 
	// Bit 3 DRVCM = 1 common mode is driven out of the external common-mode pin
	// Bit 2 EXTEM = 0 use internal common mode
	// Bit 1 RLDSEL = 1 enable Right-leg drive
	// Bit 0 SHLDEN = 1 enable shield drive
	adas1000_register_write (0x05, 0xE0000B);

	// Write to FRMCTL (Frame Control Register)
	// This determines what's included in the data frame.
	//
	#define LEAD1_LA_DIS (1<<23)
	#define LEAD2_LL_DIS (1<<22)
	#define LEAD3_RA_DIS (1<<21)
	#define V1_DIS (1<<20)
	#define V2_DIS (1<<19)
	#define PACE_DIS (1<<14)
	#define RESPM_DIS (1<<13)
	#define RESPPH_DIS (1<<12)
	#define LOFF_DIS (1<<11)
	#define GPIO_DIS (1<<10)
	#define CRC_DIS (1<<9)
	#define A_DIS (1<<7)
	#define RDYRPT (1<<6)
	#define DATAFMT (1<<4)
	#define FRMCTL_SKIP_MASK (0x0000000C) // 0b...0000 1100
	#define FRMCTL_SKIP_1 (0x00000004)
	#define FRMCTL_SKIP_3 (0x00000008)
	//adas1000_register_write (0x0A, 0x079200); // 0000 0111 1001 0010 0000 0000
	//adas1000_register_write (0x0A, 0x1FB600); // 0001 1111 1011 0110 0000 0000
	adas1000_register_write (0x0A,
		 LEAD3_RA_DIS | V1_DIS | V2_DIS 
		| PACE_DIS | RESPM_DIS | RESPPH_DIS 
		| GPIO_DIS | CRC_DIS
		| FRMCTL_SKIP_3  // 1/4 frames: 500Hz rate

	);

	// Write to GPIOCTL
	// Configure GPIO0 as input 0b00000000 00000000 0000[01]00
	//adas1000_register_write (0x06, 0x000004);

	// Write to ECGCTL
	// Bits [23:19]: all 1 enable all 5 ECG channels
	// Bit 10: CHCONFIG = 1 differential input (analog lead mode)
	// Bits [9:8] GAIN = 00 (x1.4)
	// Bit 7 VREFBUF VREF buffer enable = 1
	// Bit 6 CLKEXT = 0 Use XTAL
	// Bit 5 Master = 1
	// Bit 4 Gang = 0
	// Bit 3 HP = 1 2MSP high performance / low noise
	// Bit 2 CNVEN Conversion Enable = 1
	// Bit 1 PWREN Power Enable = 1
	// Bit 0 SWRST Reset = 0
	adas1000_register_write (0x01, 0xF804AE);

}

void adas1000_testtone_enable (void) {

	// 10Hz 1mV p-p sine wave to LA
	adas1000_register_write (0x08, // TESTTONE register
		(1<<23)  // connect test toneto LA
		| (1<<2) // TONINT (connect internally)
		| (1<<0) // TONEN (enable test tone)
	);
}

void adas1000_print_diagnostics(void) {
/*
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
	printf ("CMREFCTL= %x\r\n" , adas1000_register_read(0x05));
	printf ("FRMCTL= %x\r\n" , adas1000_register_read(0x0a));
	printf ("ECGCTL= %x\r\n" , adas1000_register_read(0x01));	
	printf ("GPIOCTL= %x\r\n" , adas1000_register_read(0x06));	
	printf ("OPSTAT= %x\r\n" , adas1000_register_read(0x1f));
*/
}

void adas1000_register_write (uint8_t reg, uint32_t value) {

	uint8_t request[4];
	request[0] = 0x80 | (reg & 0x7f);
	request[1] = (value >> 16)&0xff;
	request[2] = (value >> 8) & 0xff;
	request[3] = value & 0xff;

	ssp0Select();
	sspSend(0, (uint8_t *)&request, 4);
	ssp0Deselect();
}

uint32_t adas1000_register_read (uint8_t reg) {
	uint8_t request[4];
	//uint8_t response[4];
	uint32_t response;
	ssp0Select();
	request[0] = reg & 0x7F;
	sspSend(0, (uint8_t *)&request, 4);
	sspReceive (0, (uint8_t *)&response, 4);
	ssp0Deselect();
	// Reverse byte order
	return reverse_byte_order(response);
}
void adas1000_power_off (void) {
	// Power everything off by writing 0x0 into ECGCTL
	adas1000_register_write (0x01, 0x000000);
}

int adas1000_wait_drdy (void) {
	uint32_t i;
	for (i = 0; i < 100000; i++) {
		if (gpioGetValue(0,5) == 0) {
			return 0;
		}
	}
	printf ("T\n");
	return -1; // timeout
}

volatile uint32_t adas1000_flags=0;

void adas1000_ecg_capture_stop () {
	adas1000_flags = 1 ;
}

void adas1000_ecg_capture (uint32_t nsamples) {

	uint32_t i,j,k;
	uint32_t la,ra,ll;
	uint32_t base_addr;

	uint8_t request[SSP_FIFOSIZE];

	uint32_t frame[16];


	adas1000_flags = 0;

	// This starts the data...
	adas1000_register_read (0x40);


	k = 0;
	for (j = 0; j<nsamples; j++) {

		if (j % 100 == 0) {
			cmdPoll();
			if (adas1000_flags == 1) {
				printf ("ECG capture stop\n");
				return;
			}
		}


		// Wait for /DRDY
		if (adas1000_wait_drdy() == -1) {
			printf ("DRDY timeout\n");
			return;
		}

		// Data is available

		ssp0Select();

		// Skip to frame header
		do {
			sspReceive (0, (uint8_t *)&frame[0], 4);
			i = frame[0] &0xff;
			if ( (i & 0x80) == 0) {
				printf ("*");
			}
		} while ( (i&0x80) == 0);

		for (i = 1; i < 4; i++) {
			sspReceive (0, (uint8_t *)&frame[i], 4);
		}

		ssp0Deselect();


		la = reverse_byte_order(frame[1])&0xffffff;
		//la = frame[1]&0xffffff;
		//ll = reverse_byte_order(frame[3]&0xffffff);
		ra = reverse_byte_order(frame[2])&0xffffff;
		//ra = frame[2]&0xffffff;

		//la = 0x01020304;
		//ra = 0x55667788;

		//#ifdef SRAM_23LC1024
		if ( (frame[0]&0x40) == 0) {
		// Write ECG record to memory
		sramSelect();
		base_addr = (k++) * 8;
		request[0] = 0x02;  // write command
		request[1] = (base_addr>>16)&0xff; 
		request[2] = (base_addr>>8)&0xff;
		request[3] = base_addr & 0xff;
		sspSend(0, (uint8_t *)&request, 4);
		// Write ECG header byte
		request[0] = frame[0] & 0xff;
		sspSend(0, (uint8_t *)&request, 1);
		// Write LA,RA
		sspSend(0, (uint8_t *)&la, 3);
		sspSend(0, (uint8_t *)&ra, 3);
		sramDeselect();
		}
		//#endif
	}

}

void adas1000_ecg_playback (uint32_t nsamples) {

	uint32_t i,j;
	uint32_t la,ra,ll;

	uint8_t request[SSP_FIFOSIZE];
	uint8_t response[SSP_FIFOSIZE];

	uint32_t base_addr;

	// Read back data from SRAM
	for (j = 0; j < nsamples; j++) {
		sramSelect();
		base_addr = j * 8;
		request[0] = 0x03;  // read command
		request[1] = (base_addr>>16)&0xff; 
		request[2] = (base_addr>>8)&0xff;
		request[3] = base_addr & 0xff;
		sspSend(0, (uint8_t *)&request, 4);
		sspReceive (0, (uint8_t *)&response, 1);
		printf ("%x ", response[0]);
		la=0; ra=0;
		sspReceive (0, (uint8_t *)&la, 3);
		sspReceive (0, (uint8_t *)&ra, 3);
		printf ("%x %x\n", la, ra);
		sramDeselect();
	}

}

uint32_t adas1000_sram_test (void) {

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


