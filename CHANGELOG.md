# Changelog

## v1.15.0

* New features
  * The `-b binfile` option can now be passed `-t <format type>` to adjust how
    it interprets binary identifier data. The default is to convert to ASCII hex
    characters, but it can be changed to things like `-t text` when the binary
    data is already ASCII or UTF-8 text or `-t big_endian_decimal` when the
    bytes should be converted to a number and then output as decimal. See the
    docs for options.

* Changes
  * Read `/sys/firmware/devicetree/base/serial-number` first when pulling
    identifiers from the device tree. If this doesn't exist, it checks the older
    `/proc` location. The kernel has had a symlink from the `/proc` location to
    the sysfs location for what seems like a long time now.

## v1.14.1

* New features
  * U-Boot environment blocks containing board ID information can now be located
    at offsets beyond 2 GB.
  * Copyright and licensing have been updated to conform to the REUSE
    specification.

## v1.14.0

* New features
  * Add `-b script` option to support running an external script or program to
    supply the serial number. This is useful for including proprietary ID code
    in with other boardid options. Thanks to Jon Ringle for this feature.
  * Improve `-b bbb` to support reading the updated Beagleboard EEPROM format
    used on the BeaglePlay.

## v1.13.1

* Bug fixes
  * Improve ATECC retry logic to retry on a transient I2C read failure as well
    as an unexpected response. This also doubles the ATECC wakeup retries to
    4 and increases the retry timeout from 1.5 ms to 10 ms. This works around
    a rare issue where an I2C failure that only seems to happen at boot causes
    the serial number to not be readable.

## v1.13.0

* New features
  * Add `-X` option to capitalize hex digits when converting them from binary.
    This works with the `binfile` and `atecc508a` methods since those are the
    only two methods that run binary to hex conversions.

## v1.12.0

* New features
  * Support reading serial numbers from GRiSP2's onboard manufacturing
    information EEPROM

## v1.11.1

* Bug fixes
  * Allow custom CFLAGS and LDFLAGS to be passed in. This enables build systems
    to pass platform-specific build options.

## v1.11.0

* New features
  * Support returning a Raspberry Pi's eth0 or wlan0 MAC address based on
    querying the VideoCore. Ethernet and WiFi drivers don't need to be loaded
    for this to work. Thanks to Dömötör Gulyás for helping add this feature.

## v1.10.0

* New features
  * Support 16+ character serial numbers on NervesKey devices
  * Support nondefault I2C addresses for ATECC chips

## v1.9.0

* New features
  * Support serial numbers stored in redundant U-Boot environment blocks. The
    environment block location is read from `/etc/fw_env.config`.

## v1.8.0

* New features
  * Add support for the nVidia Jetson Nano and other boards that use
    /proc/device-tree/serial-number.

## v1.7.0

* New features
  * Support hardcoded serial number prefixes so that these strings can be
    centralized in `/etc/boardid.config`. See the `-p` option.

## v1.6.0

* New features
  * Read the serial number out of the DMI/SMBIOS information. This is common on
    x86 system.

## v1.5.3

* Bug fixes
  * Fix issue preventing config file options from being overridden by the
    commandline.

## v1.5.2

* Bug fixes
  * Leave the ATECC508A in a good state and recover from it being in an
    unexpected state when querying the ID.

## v1.5.1

* Bug fixes
  * Fix bug that would cause unprogrammed ATECC508A chips to report bogus
    IDs

## v1.5.0

* New features
  * Support reading options from `/etc/boardid.config`. If the file exists,
    options are read from it. The commandline still takes precedence.

* Bug fixes
  * Merge back ATECC508A improvements from the nerves_key_pkcs11 work

## v1.4.0

* New feature
  * Support reading serial numbers from a NervesKey

## v1.3.0

* New feature
  * Add fallback option when an ID can't be found anywhere (-b force)

* Bug fixes
  * Fix an off-by-one error when reading the serial number on the ATECC508A
  * Check CRC and add a retry for ATECC508A communication

## v1.2.0

* New feature
  * Add support for reading serial numbers from ATECC508A CrytoAuthentication
    Devices.

## v1.1.1

* Bug fix
  * Ignore empty U-Boot environment variables. I.e. "serial_number=" is treated
    the same as it not being listed as all.

## v1.1.0

* New feature
  * Read U-boot environment location from `/etc/fw_env.config`. This makes keeps
    the U-boot environment info in one place.

## v1.0.0

* New features
  * Support reading serial numbers from U-boot environment variables
  * Allow users to specify multiple strategies so that if one doesn't work,
    it will try the next.

## v0.4.0

* New features
  * LinkIt Smart 7688 support
  * Generic support for reading a unique ID from a file

## v0.3.0

* New features
  * Support using eth0's MAC address as ID info

## v0.2.0

* New features
  * Lego EV3 brick support

## v0.1.0

This is the first release from extracting the nerves-id helper program from
nerves-system-br.

* New features
  * Beaglebone support
  * Makefile, unit tests, and packaging

