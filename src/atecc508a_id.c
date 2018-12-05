#include "common.h"
#include "atecc508a.h"

#include <string.h>

int atecc508a_id(const struct id_options *options, char *buffer, int len)
{
    int fd = atecc508a_open(options->filename);
    if (fd < 0)
        return 0;

    if (atecc508a_wakeup(fd) < 0) {
        atecc508a_close(fd);
        return 0;
    }

    uint8_t serial_number[9];
    if (atecc508a_read_serial(fd, serial_number) < 0) {
        atecc508a_close(fd);
        return 0;
    }

    atecc508a_sleep(fd);

    atecc508a_close(fd);

    char serial_number_hex[sizeof(serial_number) * 2 + 1];
    bin_to_hex(serial_number, sizeof(serial_number), serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    if (len > sizeof(serial_number_hex))
        len = sizeof(serial_number_hex);
    if (len < 1)
        len = 1;

    int digits = len - 1;
    memcpy(buffer, serial_number_hex, digits);
    buffer[digits] = 0;
    return 1;
}
