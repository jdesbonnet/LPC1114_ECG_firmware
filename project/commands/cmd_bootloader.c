#include <stdio.h>
#include <string.h>
#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "ads1x9x.h"
#include "parse_hex.h"
#include "cmd_ads1x9x.h"

//#include "LPC11xx.h"

//typedef int (*FN)();

// http://www.lpcware.com/content/blog/calling-lpc11xx-isp-user-code

typedef void (*IAP)(unsigned int[], unsigned int[]);


// static prefix limits scope to this compilation unit.

void cmd_bootloader(uint8_t argc, char **argv)
{

	uint32_t temp;

	IAP iap_entry = (IAP) 0x1fff1ff1;
	uint32_t command[5], result[4];

	/* Disable UART interrupts */
	//LPC_UART->IER = 0;
	UART_U0IER = 0;

	/* Disable UART interrupts in NVIC */
	NVIC_DisableIRQ(UART_IRQn);

  /* Ensure a clean start, no data in either TX or RX FIFO. */
/*
  while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
  while ( LPC_UART->LSR & LSR_RDR )
  {
	temp = LPC_UART->RBR;	// Dump data from RX FIFO 
  }
*/

	/* Read to clear the line status. */
	//temp = LPC_UART->LSR;
	temp = UART_U0LSR;


 /* make sure 32-bit Timer 1 is turned on before calling ISP */
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x00400;

  /* make sure GPIO clock is turned on before calling ISP */
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x00040;

  /* make sure IO configuration clock is turned on before calling ISP */
  //LPC_SYSCON->SYSAHBCLKCTRL |= 0x10000;

  /* make sure AHB clock divider is 1:1 */
  //LPC_SYSCON->SYSAHBCLKDIV = 1;

  /* Send Reinvoke ISP command to ISP entry point*/
  command[0] = 57;

  /* Set stack pointer to ROM value (reset default).
     This must be the last piece of code executed before calling ISP,
     because most C expressions and function returns will fail after
     the stack pointer is changed.
   */
  __set_MSP(*((uint32_t *) 0x1FFF0000)); /* inline asm */

  /* Invoke ISP. We call "iap_entry" to invoke ISP because the ISP entry
     is done through the same command interface as IAP. */
  iap_entry(command, result);

  // Code will never return!

}



