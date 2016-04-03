# boardid
[![Build Status](https://travis-ci.org/fhunleth/boardid.svg?branch=master)](https://travis-ci.org/fhunleth/boardid)

This program looks up a device-specific serial number and prints it. The
original use was to provide some non-changing and unique material for
dynamically creating device names on a local LAN.

Supported boards:

  * Raspberry Pi (all varieties)
  * Beaglebone Black
  * Lego EV3

If your board isn't listed above, it may be supported via one of the generic
mechanisms:

  * Reading a serial number from `/proc/cpuinfo`
  * Reading the MAC address of `eth0`

If your board isn't supported, please consider sending a pull request.

## Building

Run `make`. To run the unit tests, run `make check`.

## Usage

```
Usage: boardid [OPTION]...

Options:
  -b <board/method> Use the specified board or detection method for
                    reading the ID.
  -n <count>        Print out count characters (least significant ones)
  -r <prefix>       Root directory prefix (used for unit tests)
  -v                Print out the program version

Supported boards/methods:
  rpi       Raspberry Pi (all models)
  ev3       Lego EV3
  cpuinfo   Read /proc/cpuinfo
  bbb       Beaglebone Black
  macaddr   Read eth0's MAC address
```

Without the `-b` option, `boardid` will try each method of determining an ID
until one works.

## Example

Here's an example run on a Lego Mindstorms EV3 brick running ev3dev:

```
robot@ev3dev:~$ boardid
00000016534b129d
robot@ev3dev:~$
```

## Caveats

This utility doesn't provide any assurance of the uniqueness of returned IDs. It
is not unheard of for board manufacturers to accidentally reuse IDs or forget to
program them entirely. Additionally, these IDs may even be guessible so
using them in a cryptographic sense is not advised without an understanding for
how they're assigned.
