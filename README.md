<!--
  SPDX-FileCopyrightText: 2016 Frank Hunleth
  SPDX-FileCopyrightText: 2021 Dömötör Gulyás
  SPDX-License-Identifier: CC-BY-4.0
-->

# boardid

[![CircleCI](https://dl.circleci.com/status-badge/img/gh/nerves-project/boardid/tree/main.svg?style=svg)](https://dl.circleci.com/status-badge/redirect/gh/nerves-project/boardid/tree/main)
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
* Reading bytes from a file and interpret as hex, decimal or text
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
  -f <path>         The file to read for the methods requiring files
  -k <offset>       The offset in bytes for the 'binfile'/`uenv' methods
  -l <count>        The number of bytes to read for the 'binfile'/'uenv' methods
  -u <varname>      U-boot environment variable name for the 'uenv' method
  -n <count>        Print out count characters (least significant ones)
  -p <string>       Prefix an ID with the specific string
  -r <prefix>       Root directory prefix (used for unit tests)
  -a <i2c address>  I2C bus address
  -t <format>       Output format when converting from binary. See below.
  -X                Print capital hex digits for `binfile`/`atecc508a` methods
  -v                Print out the program version

'-b' can be specified multiple times to try more than one method.

Supported boards/methods:
  atecc508a  Read an ATECC508* or ATECC608* (I2C device '-f', I2C address '-a')
  bbb        Beaglebone and BeaglePlay
  binfile    Read '-l' bytes from the file '-f' at offset '-k'
  rpi        Raspberry Pi (all models)
  ev3        Lego EV3
  chip       Next Thing Co - C.H.I.P.
  cpuinfo    Read /proc/cpuinfo
  jetson     nVidia Jetson
  device_tree Read /sys/firmware/devicetree/base/serial-number
  dmi        Read the system ID out of the SMBIOS/DMI
  force      Force the ID (Specify ID with '-f')
  grisp      GRiSP 2 board
  linkit     LinkIt Smart (WLAN MAC address)
  macaddr    Read eth0's MAC address
  nerves_key  Read a NervesKey (I2C device '-f', I2C address '-a')
  rpi_eth0   Raspberry Pi eth0 MAC address
  rpi_wlan0  Raspberry Pi wlan0 MAC address
  script     Run a script to get the ID (Specify script with '-f')
  uboot_env  Read a U-Boot environment (file '-f', offset '-k', length '-l') and use the
             variable '-u', defaults to values from '/etc/fw_env.config'

Supported binary to text conversions:
  hex                   Convert to a lower case hex string (default)
  uppercase_hex         Convert to a upper case hex string
  lowercase_hex         Convert to a lower case hex string
  decimal               Interpret bytes in little endian and output as a decimal
  big_endian_decimal    Interpret bytes in big endian and output as a decimal
  little_endian_decimal Interpret bytes in little endian and output as a decimal
  text                  Copy bytes directly like they are already text
```

## Example

Here's an example run on a Lego Mindstorms EV3 brick running ev3dev:

```sh
robot@ev3dev:~$ boardid
00000016534b129d
robot@ev3dev:~$
```

## Config file

It's possible to come up with fairly long and complicated command lines for some
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

## Cookbook

### Read from device tree

This is a very common location for serial numbers and if you're unsure, try this
first. It reads `/sys/firmware/devicetree/base/serial-number`.

```sh
boardid -b device_tree
```

### Read eth0's MAC address

```sh
boardid -b macaddr
```

The ID does not contain any colons.

### Read last 4 hex digits of eth0 MAC

```sh
boardid -b macaddr -n 4
```

### Read bytes as a hex string from a binary file

```sh
boardid -b binfile -f /dev/mtdblock2 -l 6 -k 0x28
```

Use `-X` for uppercase hex output.

### Read bytes as a text string from a binary file

This is similar to the previous example, but the serial
number is already text so no conversion is needed.

```sh
boardid -b binfile -f /dev/mtdblock2 -l 8 -k 0x28 -t text
```

### Read bytes as a decimal string from a binary file

This is similar to the hex example, but the serial
number should be interpreted as a decimal number. In this case, it's little endian, but it doesn't have to be.

```sh
boardid -b binfile -f /dev/mtdblock2 -l 8 -k 0x28 -t little_endian_decimal
```

### Read serial_number variable from U-Boot environment

```sh
boardid -b uboot_env -u serial_number
```

Defaults to values from `/etc/fw_env.config` if `-f`, `-k`, `-l` not specified.

### Force a specific ID

```sh
boardid -b force -f 0123456789
```

Useful for testing or overriding.

### Read from SMBIOS/DMI (x86 industrial boards)

```sh
boardid -b dmi
```

### Run custom script

```sh
boardid -b script -f /usr/local/bin/get_serial.sh
```

This is the ultimate escape hatch for getting a serial number. Please log an
issue if you run into this as it would be nice for `boardid` to support all
situations.

### Add prefix to serial number

This is useful for prepending a product abbreviation on the IDs for ease of
figuring out what kind of thing a serial number came from. It's not recommended
to use this feature for crafting hostnames if they're different from the way you
represent serial numbers everywhere else.

```sh
boardid -b rpi -p RPI
```

Output: `RPI19f2f468`

### Try multiple methods (fallback)

```sh
boardid -b uboot_env -u serial_number -b macaddr -n 4
```

Tries U-Boot first, falls back to MAC address.

### Read a Microchip ATECC508/608 cryptographic module's serial

```sh
boardid -b atecc508a -f /dev/i2c-0 -a 0x35 -X
```

Reads the 9-byte serial number (18 hexadecimal characters) from a module on bus `i2c-0` with I2C
address 0x35. Both ATECC508* and ATECC608* parts are supported. Since `boardid`
directly reads the serial number, it may try to access the device when other
code is using it. This results in a transient failure which is easily recovered
from by retrying. If you need more retries, repeat the `-b atecc508a -f
/dev/i2c-0 -a 0x35 -X` arguments more than once.

## Caveats

This utility doesn't provide any assurance of the uniqueness of returned IDs. It
is not unheard of for board manufacturers to accidentally reuse IDs or forget to
program them entirely. Additionally, these IDs may even be guessable so using
them in a cryptographic sense is not advised without an understanding for how
they're assigned.

## License

This work is licensed under multiple licenses. Individual files are labeled
following the [REUSE recommendations](https://reuse.software).
