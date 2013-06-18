
#ifndef __ADS1X9X_H__ 
#define __ADS1X9X_H__

#define CMD_WAKEUP (0x02)
#define CMD_STANDBY (0x04)
#define CMD_RESET (0x06)
#define CMD_START (0x08)
#define CMD_STOP (0x0a)
#define CMD_OFFSETCAL (0x1a)
#define CMD_RDATAC (0x10)
#define CMD_SDATAC (0x11)
#define CMD_RDATA (0x12)

#define REG_ID (0x00)
#define REG_CONFIG1 (0x01)
#define REG_CONFIG2 (0x02)
#define REG_CH1SET (0x04)
#define REG_CH2SET (0x05)

#define CH1 (0)
#define CH2 (1)
#define SPS (500)

void ads1x9x_init(void);
int ads1x9x_test(void);
void ads1x9x_command (uint8_t command);
void ads1x9x_ecg_read (uint8_t *buf);
int ads1x9x_drdy_wait (int timeout);
uint8_t ads1x9x_register_read (uint8_t registerId);
void ads1x9x_register_write (uint8_t registerId, uint8_t registerValue);
void ads1x9x_hw_reset (void);
void ads1x9x_measure_shorted (void);
void ads1x9x_measure_test_signal (void);
void delay(int delay);

#endif


