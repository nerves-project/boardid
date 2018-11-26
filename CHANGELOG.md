# Changelog

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

