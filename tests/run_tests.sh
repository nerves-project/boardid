#!/bin/bash
# SPDX-FileCopyrightText: 2016 Frank Hunleth
#
# SPDX-License-Identifier: Apache-2.0

set -e

export LC_ALL=C

# Linux command line tools that may be different on other OSes
READLINK=readlink
BASE64_DECODE=-d

if [ -d "/mnt/c/Users" ]; then
    # Windows 10 bash mode
    HOST_OS=Windows
    HOST_ARCH=amd64
else
    HOST_OS=$(uname -s)
    HOST_ARCH=$(uname -m)
fi

case "$HOST_OS" in
    Darwin)
        # Not -d?
        BASE64_DECODE=-D

        READLINK=$(brew --prefix)/bin/greadlink
        [ -e $READLINK ] || ( echo "Please run 'brew install coreutils' to install greadlink"; exit 1 )
        ;;
    *)
        ;;
esac

base64_decode() {
    base64 $BASE64_DECODE
}

TESTS_DIR=$(dirname $($READLINK -f $0))

WORK=$TESTS_DIR/work
BOARDID=$TESTS_DIR/../boardid

[ -e $BOARDID ] || ( echo "Build $BOARDID first"; exit 1 )

run() {
    TEST=$1
    CONFIG=$WORK/$TEST.config
    CMDLINE=
    EXPECTED=$WORK/$TEST.expected
    ACTUAL=$WORK/$TEST.actual

    echo "Running $TEST..."

    rm -fr $WORK
    # Create some common directories so that scripts don't need to
    mkdir -p $WORK/dev
    mkdir -p $WORK/etc
    mkdir -p $WORK/proc
    mkdir -p $WORK/sys/bus/i2c/devices/0-0050/at24-0
    mkdir -p $WORK/sys/bus/i2c/devices/0-0057
    mkdir -p $WORK/sys/class/net/eth0
    mkdir -p $WORK/sys/class/net/eth1
    mkdir -p $WORK/sys/class/net/lo0

    # Run the test script to setup files for the test
    source $TESTS_DIR/$TEST

    # Prepend the "root" directory for unit test purposes
    CMDLINE="-r $WORK $CMDLINE"

    # Run
    $BOARDID $CMDLINE > $ACTUAL

    # check results
    diff -w $EXPECTED $ACTUAL
    if [ $? != 0 ]; then
        echo "Test $TEST failed!"
        exit 1
    fi
}

# Test command line arguments
TESTS=$(ls $TESTS_DIR/[0-9][0-9][0-9]_* | sort)
for TEST_CONFIG in $TESTS; do
    TEST=$(basename $TEST_CONFIG)
    run $TEST
done

rm -fr $WORK
echo "Pass!"
exit 0
