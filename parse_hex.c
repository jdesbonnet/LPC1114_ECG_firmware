#include <stdint.h>
#include "parse_hex.h"

uint32_t parse_dec_or_hex (char *buf) {
	if (buf[0]=='0' && buf[1]=='x') {	
		return parse_hex(buf+2);
	} else {
		return atoi (buf);
	}
}

uint32_t parse_hex (char *buf) {
	int d=0,res=0;
	
	while ( (d=is_hex_digit(*buf++)) != -1 ) {
		res <<= 4;
		res |= d;
	}
	return res;
}

/**
 * Return the numeric value of a hex digit or -1 if not a hex digit.
 */
int is_hex_digit(char c) {
	if (c>='0' && c<='9') {
		return c - '0';
	}
	if (c>='a' && c<='f') {
		return c - 'a' + 10;
	}
	if (c>='A' && c<='F') {
		return c - 'A' + 10;
	}
	return -1;	
}
