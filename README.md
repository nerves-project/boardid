# boardid

[![CircleCI](https://circleci.com/gh/nerves-project/boardid.svg?style=svg)](https://circleci.com/gh/nerves-project/boardid)
[![REUSE status](https://api.reuse.software/badge/github.com/nerves-project/boardid)](https://api.reuse.software/info/github.com/nerves-project/boardid)

This program looks up a device-specific serial number and prints it. The
original use was to provide some non-changing and unique material for
dynamically creating device names on a local LAN.

Supported boards:

* Raspberry Pi (all varieties)
* [BeagleBone (all varieties) and BeaglePlay](http://beagleboard.org/)
* [Lego EV3](https://www.ev3dev.org/)
* [LinkIt Smart 7688](https://www.seeedstudio.com/LinkIt-Smart-7688-p-2573.html)
* Next Thing Co - C.H.I.P.
* Many off-the-shelf industrial x86 boards via SMBIOS/DMI
* nVidia Jetson

If your board isn't listed above, it may be supported via one of the generic
mechanisms:

* Reading a serial number from `/proc/cpuinfo`
* The default Raspberry Pi `eth0` and `wlan0` MAC addresses
* Reading the MAC address of `eth0`
* Reading bytes from a file
* Reading a key from the U-Boot environment
* Reading an [ATECC508A/608A](https://www.microchip.com/wwwproducts/en/ATECC508A)'s serial number via an I2C bus
* Reading the serial number stored in a [NervesKey](https://github.com/nerves-hub/nerves_key/)'s OTP memory
* Reading the serial number from [SMBIOS/DMI](https://www.dmtf.org/standards/smbios)
* Reading the serial number via the device tree

If your board isn't supported, please consider sending a pull request.

## Building

Run `make`. To run the unit tests, run `make check`.

## Usage

```text
Usage: boardid [OPTION]...

Options:
  -b <board/method> Use the specified board or detection method for
                    reading the ID.
  -f <path>         The file to read for the 'binfile'/'uenv' methods
  -k <offset>       The offset in bytes for the 'binfile'/`uenv' methods
  -l <count>        The number of bytes to read for the 'binfile'/'uenv' methods
  -u <varname>      U-boot environment variable name for the 'uenv' method
  -n <count>        Print out count characters (least significant ones)
  -p <string>       Prefix an ID with the specific string
  -a <i2c address>  I2C address for the `atecc508a`/`nerves_key` methods
  -r <prefix>       Root directory prefix (used for unit tests)
  -X                Print capital hex digits for `binfile`/`atecc508a` methods
  -v                Print out the program version

'-b' can be specified multiple times to try more than one method.

Supported boards/methods:
  rpi          Raspberry Pi (all models)
    rpi_eth0   Raspberry Pi eth0 MAC address
    rpi_wlan0  Raspberry Pi wlan0 MAC address
  ev3          Lego EV3
  chip         Next Thing Co - C.H.I.P.
  cpuinfo      Read /proc/cpuinfo
  jetson       nVidia Jetson
  device_tree  Read /proc/device-tree/serial-number
  bbb          Beaglebone Black and BeaglePlay
  macaddr      Read eth0's MAC address
  linkit       LinkIt Smart (WLAN MAC address)
  binfile      Read '-l' bytes from the file '-f' at offset '-k'
  uboot_env    Read a U-Boot environment (file '-f', offset '-k', length '-l') and use the variable '-u'
  atecc508a    Read an ATECC508A (I2C device '-f', I2C address '-a')
  nerves_key   Read a NervesKey (I2C device '-f', I2C address '-a')
  dmi          Read the system ID out of the SMBIOS/DMI
  force        Force the ID (Specify ID with '-f')

Without the `-b` option, `boardid` will try a few methods of determining an ID.

## Example

Here's an example run on a Lego Mindstorms EV3 brick running ev3dev:

```sh
robot@ev3dev:~$ boardid
00000016534b129d
robot@ev3dev:~$
```

## Config file

It's possible to come up with fairly long and complicated commandlines for some
devices. `boardid` supports reading arguments from `/etc/boardid.config` so that
you need not duplicate the commandline arguments everywhere `boardid` is used.
A config file can have comments. Here's an example:

```txt
# /etc/boardid.config

# Try the nerves_serial_number environment variable first
-b uboot_env -u nerves_serial_number

# Try the legacy serial_number environment variable
-b uboot_env -u serial_number

# Check the EEPROM for a serial number (BBB and BBG)
-b bbb -n 4

# If the EEPROM wasn't programmed, use the MAC address
-b macaddr -n 4
```

## Caveats

This utility doesn't provide any assurance of the uniqueness of returned IDs. It
is not unheard of for board manufacturers to accidentally reuse IDs or forget to
program them entirely. Additionally, these IDs may even be guessable so using
them in a cryptographic sense is not advised without an understanding for how
they're assigned.

## License

This work is licensed under multiple licenses. Individual files are labeled
following the [REUSE recommendations](https://reuse.software).

For following summarizes the licensing:

* All original source code is licensed under Apache-2.0
* CRC32 calculation code states "You may use this program, or code or tables
  extracted from it, as desired without restriction."
* Configuration and data files are licensed under CC0-1.0
* Documentation is CC-BY-4.0

