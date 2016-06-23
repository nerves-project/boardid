#!/bin/bash

set -e

TESTS_DIR=$(dirname $(readlink -f $0))

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
    mkdir -p $WORK/proc
    mkdir -p $WORK/sys/bus/i2c/devices/0-0050/at24-0
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
