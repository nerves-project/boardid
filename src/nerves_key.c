#include "common.h"
#include "atecc508a.h"

#include <string.h>

// See https://github.com/fhunleth/atecc508a for Nerves Key OTP format

static uint8_t nerves_key_magic[4] = {0x4e, 0x72, 0x76, 0x73};

int nerves_key_id(const struct id_options *options, char *buffer, int len)
{
    int fd = atecc508a_open(options->filename);
    if (fd < 0)
        return 0;

    if (atecc508a_wakeup(fd) < 0) {
        atecc508a_close(fd);
        return 0;
    }

    // The Nerves Key's serial number is in the OTP memory
    uint8_t otp[32];
    if (atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_OTP, 0, 0, 0, otp, 32) < 0 &&
        atecc508a_read_zone_nowake(fd, ATECC508A_ZONE_OTP, 0, 0, 0, otp, 32) < 0)
        return -1;

    atecc508a_sleep(fd);
    atecc508a_close(fd);

    if (memcmp(otp, nerves_key_magic, sizeof(nerves_key_magic)) != 0)
        return -1;

    const char *serial_number = (const char *) &otp[16];
    size_t serial_number_len = strnlen(serial_number, 16);

    // The ATECC508A has a 18 character serial number (9 bytes)
    if (len > serial_number_len)
        len = serial_number_len;
    if (len < 1)
        len = 1;

    memcpy(buffer, serial_number, len);
    buffer[len] = 0;
    return 1;
}
