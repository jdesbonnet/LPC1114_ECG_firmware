#ifndef __CMD_ADS1X9X_H__ 
#define __CMD_ADS1X9X_H__

#define FRAME_ECG_RECORD (0x01)
extern uint32_t cmd_ads1x9x_flags;

#define cmd_ads1x9x_is_binary_mode() (cmd_ads1x9x_flags==1)

#endif
