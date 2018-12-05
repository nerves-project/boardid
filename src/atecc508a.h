#ifndef ATECC508A_H
#define ATECC508A_H

#include <stdint.h>

#define ATECC508A_ZONE_CONFIG 0
#define ATECC508A_ZONE_OTP    1
#define ATECC508A_ZONE_DATA   2

int atecc508a_open(const char *filename);
void atecc508a_close(int fd);
int atecc508a_wakeup(int fd);
int atecc508a_sleep(int fd);
int atecc508a_read_serial(int fd, uint8_t *serial_number);
int atecc508a_read_zone(int fd, uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len);

#endif // ATECC508A_H
