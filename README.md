# boardid
[![Build Status](https://travis-ci.org/fhunleth/boardid.svg?branch=master)](https://travis-ci.org/fhunleth/boardid)

This program looks up a platform-specific serial number and prints it. If
your application needs a way identify a board, this could be it.

Supported boards:

  * Raspberry Pi (all varieties)
  * Beaglebone Black

If your board isn't supported, please consider sending a pull request.

## Building

Just run `make`. To run the unit tests, run `make check`.

## Usage

```
Usage: boardid [OPTION]...

Options:
  -b <platform>   Use the board id method for the specified platform
  -n <count>      Print out count characters (least significant ones)
  -r <prefix>     Root directory prefix (used for unit tests)
  -v              Print out the program version
```
