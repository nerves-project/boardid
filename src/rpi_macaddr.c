// SPDX-FileCopyrightText: 2021 Nerves Project Developers
//
// SPDX-License-Identifier: Apache-2.0

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

// See https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define VC_MBOX_DEVICE_PATH "/dev/vcio"
#define VC_MBOX_IOCTL_PROPERTY _IOWR(100, 0, char *)

#define VC_MBOX_GET_MAC_ADDRESS 0x00010003
#define VC_MBOX_GET_MAC_ADDRESS_VALUE_SIZE 6
#define VC_MBOX_GET_MAC_ADDRESS_BUFFER_SIZE 32

static bool vc_read_mac_address(uint8_t *mac_address)
{
    bool success = false;
    int fd = open(VC_MBOX_DEVICE_PATH, O_RDONLY);
    if (fd < 0)
        return success;

    uint32_t words[VC_MBOX_GET_MAC_ADDRESS_BUFFER_SIZE / 4] =
        {VC_MBOX_GET_MAC_ADDRESS_BUFFER_SIZE, 0, VC_MBOX_GET_MAC_ADDRESS, VC_MBOX_GET_MAC_ADDRESS_VALUE_SIZE, 0, 0, 0, 0};
    int rc = ioctl(fd, VC_MBOX_IOCTL_PROPERTY, &words);
    if (rc >= 0 &&
        words[0] == VC_MBOX_GET_MAC_ADDRESS_BUFFER_SIZE &&
        words[1] == 0x80000000 &&
        words[2] == VC_MBOX_GET_MAC_ADDRESS &&
        words[3] == VC_MBOX_GET_MAC_ADDRESS_VALUE_SIZE) {
        memcpy(mac_address, &words[5], 6);
        success = true;
    }

    close(fd);
    return success;
}

static bool rpi_macaddr_id(char *buffer, bool wlan0)
{
    uint8_t mac_address[6];
    if (!vc_read_mac_address(mac_address))
        return false;

    if (wlan0) {
        // The wlan0 MAC address is computed differently depending on the
        // version of the RPi.  In particular, if the VC reports the OUI
        // B8:27:EB, the NIC part of the MAC address comes from the serial
        // number. The WiFi MAC is programmed with the NIC part XORed with
        // 0x555555.
        //
        // The Raspberry Pi 4 (and presumably newer ones) do not derive their
        // MAC addresses from the serial number and instead have them
        // programmed to OTP memory. They do not have an OUI of B8:27:EB. The
        // currently known ones have an OUI of DC:A6:32, but the RPi Foundation
        // also owns E4:5F:01. If the MAC isn't programmed, the user can
        // program it. For these devices, the procedure is to add 1 to the NIC
        // part.
        //
        // Disclaimer: I couldn't find anything official to say that any of
        // this is true, but it seems to be so far.

        if (mac_address[0] == 0xb8 && mac_address[1] == 0x27 && mac_address[2] == 0xeb) {
            mac_address[3] ^= 0x55;
            mac_address[4] ^= 0x55;
            mac_address[5] ^= 0x55;
        } else {
            uint32_t v = (mac_address[3] << 16) | (mac_address[4] << 8) | mac_address[5];
            v++;
            mac_address[3] = (v >> 16) & 0xff;
            mac_address[4] = (v >> 8) & 0xff;
            mac_address[5] = v & 0xff;
        }
    }

    snprintf(buffer, MAX_SERIALNUMBER_LEN, "%02x%02x%02x%02x%02x%02x",
        mac_address[0], mac_address[1], mac_address[2],
        mac_address[3], mac_address[4], mac_address[5]);

    return true;
}

bool rpi_wlan0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_id(buffer, true);
}

bool rpi_eth0_macaddr_id(const struct boardid_options *options, char *buffer)
{
    return rpi_macaddr_id(buffer, false);
}
