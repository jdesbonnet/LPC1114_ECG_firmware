/**************************************************************************/
/*! 
    @file     cmd_tbl.h
    @author   K. Townsend (microBuilder.eu)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2010, microBuilder SARL
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#ifndef __CMD_TBL_H__ 
#define __CMD_TBL_H__

#define CMD_COUNT (sizeof(cmd_tbl)/sizeof(cmd_t))

#include <stdio.h>

#ifdef CFG_INTERFACE_UART
  #include "core/uart/uart.h"
#endif

// Function prototypes for the command table
void cmd_help(uint8_t argc, char **argv);         // handled by core/cmd/cmd.c
void cmd_sysinfo(uint8_t argc, char **argv);
void cmd_reset(uint8_t argc, char **argv);
void cmd_bootloader(uint8_t argc, char **argv);
void cmd_xbee(uint8_t argc, char **argv);

//void cmd_ads1x9x_reg_read_all(uint8_t argc, char **argv);
void cmd_ads1x9x_reg_read(uint8_t argc, char **argv);
void cmd_ads1x9x_reg_write(uint8_t argc, char **argv);
void cmd_ads1x9x_ecg_readn(uint8_t argc, char **argv);
void cmd_ads1x9x_ecg_pace(uint8_t argc, char **argv);
void cmd_ads1x9x_ecg_playback(uint8_t argc, char **argv);
void cmd_ads1x9x_ch_cfg(uint8_t argc, char **argv);
void cmd_ads1x9x_temp_read(uint8_t argc, char **argv);
void cmd_ads1x9x_set(uint8_t argc, char **argv);
void cmd_ads1x9x_show(uint8_t argc, char **argv);
void cmd_ads1x9x_cmd(uint8_t argc, char **argv);
void cmd_ads1x9x_test(uint8_t argc, char **argv);


#ifdef CFG_CHIBI
void cmd_chibi_addr(uint8_t argc, char **argv);
void cmd_chibi_ieeeaddr(uint8_t argc, char **argv);
void cmd_chibi_tx(uint8_t argc, char **argv);
#endif

#ifdef CFG_I2CEEPROM
void cmd_i2ceeprom_read(uint8_t argc, char **argv);
void cmd_i2ceeprom_write(uint8_t argc, char **argv);
#endif

#ifdef CFG_LM75B
void cmd_lm75b_gettemp(uint8_t argc, char **argv);
#endif

#ifdef CFG_SDCARD
void cmd_sd_dir(uint8_t argc, char **argv);
#endif

void cmd_deepsleep(uint8_t argc, char **argv);

#define CMD_NOPARAMS "This command has no parameters"

/**************************************************************************/
/*! 
    Command list for the command-line interpreter and the name of the
    corresponding method that handles the command.

    Note that a trailing ',' is required on the last entry, which will
    cause a NULL entry to be appended to the end of the table.
*/
/**************************************************************************/
cmd_t cmd_tbl[] = 
{
  // command name, min args, max args, hidden, function name, command description, syntax
  { "?",    0,  0,  0, cmd_help              , "Help"                           , CMD_NOPARAMS },
  { "V",    0,  0,  0, cmd_sysinfo           , "System Info"                    , CMD_NOPARAMS },
  { "Z",    0,  0,  0, cmd_reset             , "Reset"                          , CMD_NOPARAMS },

	// ADS1x9x commands
	//{ "REGRA",    0,  0,  0, cmd_ads1x9x_reg_read_all             , "Read all registers"   , CMD_NOPARAMS },
	{ "REGR",    1,  1,  0, cmd_ads1x9x_reg_read             , "Read registers"   , "'RREG <reg>'" },
	{ "REGW",    2,  2,  0, cmd_ads1x9x_reg_write             , "Read registers"   , "'WREG <reg> <val>'" },
	{ "CMD",    1,  1,  0, cmd_ads1x9x_cmd             , "Issue ADS1x9x command"   , "'CMD <WAKEUP|STANDBY|RESET|START|STOP|OFFSETCAL|RDATAC|SDATAC|RDATA>'" },
	{ "SET",    1,  3,  0, cmd_ads1x9x_set             , "General SET command"   , "'SET <what> <val>'" },
	{ "SHOW",    1,  1,  0, cmd_ads1x9x_show             , "General SHOW command"   , "'SHOW <SYSTICK|CONFIG|...>'" },
	{ "ECGRN",   1,  2,  0, cmd_ads1x9x_ecg_readn             , "Read/record n ECG records"   , "'ECGRN <n>'" },
	{ "ECGP",   1,  2,  0, cmd_ads1x9x_ecg_playback             , "Playback ECG from SRAM"   , "'ECGP <A|B>'" },
	{ "PACE",   0,  0,  0, cmd_ads1x9x_ecg_pace             , "Pace detection"   , "'PACE <threshold> <skip-samples>'" },
	{ "CCFG",    2,  2,  0, cmd_ads1x9x_ch_cfg, "Configure channal", "'CCFG <ch> <what>'" },
	{ "TEMP",    0,  0,  0, cmd_ads1x9x_temp_read, "Read temperature", CMD_NOPARAMS },
	{ "TEST",    0,  0,  0, cmd_ads1x9x_test, "Test ADS1x9x", CMD_NOPARAMS },
	{ "BOOTLOADER",    0,  0,  0, cmd_bootloader, "Enter Bootloader", CMD_NOPARAMS },
	{ "XBEE",    1,  1,  0, cmd_xbee, "Send AT command to XBee", "'XBEE <atcmd>'" },
  #ifdef CFG_CHIBI
  { "A",    0,  1,  0, cmd_chibi_addr        , "Get/Set node address"           , "'A [<1-65534>|<OxFFFE>]'" },
  { "S",    2, 99,  0, cmd_chibi_tx          , "Send msg to node(s)"            , "'S <destaddr> <msg>'" },
  #endif

  #ifdef CFG_I2CEEPROM
  { "e",    1,  1,  0, cmd_i2ceeprom_read    , "EEPROM Read"                    , "'e <addr>'" },
  { "w",    2,  2,  0, cmd_i2ceeprom_write   , "EEPROM Write"                   , "'w <addr> <val>'" },
  #endif

  #ifdef CFG_LM75B
  { "m",    0,  0,  0, cmd_lm75b_gettemp     , "Temperature (Celsius)"          , CMD_NOPARAMS },
  #endif

  #ifdef CFG_SDCARD
  { "d",    0,  1,  0,  cmd_sd_dir           , "Dir (SD Card)"                  , "'d [<path>]'" },
  #endif

  { "z",    0,  1,  0,  cmd_deepsleep        , "Deep sleep"     , "'z [<nsec>]'" },
};

#endif
