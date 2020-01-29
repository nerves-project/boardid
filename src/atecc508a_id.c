#include "common.h"
#include "atecc508a.h"

#include <string.h>

bool atecc508a_id(const struct boardid_options *options, char *buffer)
{
    int fd = atecc508a_open(options->filename);
    if (fd < 0)
        return false;

    if (atecc508a_wakeup(fd) < 0) {
        atecc508a_close(fd);
        return false;
    }

    uint8_t serial_number[9];
    if (atecc508a_read_serial(fd, serial_number) < 0) {
        atecc508a_close(fd);
        return false;
    }

    atecc508a_sleep(fd);

    atecc508a_close(fd);

    char serial_number_hex[sizeof(serial_number) * 2 + 1];
    bin_to_hex(serial_number, sizeof(serial_number), serial_number_hex);

    // The ATECC508A has a 18 character serial number (9 bytes)
    memcpy(buffer, serial_number_hex, sizeof(serial_number_hex));
    buffer[sizeof(serial_number_hex)] = 0;
    return true;
}
