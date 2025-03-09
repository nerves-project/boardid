# SPDX-FileCopyrightText: 2016 Frank Hunleth
#
# SPDX-License-Identifier: Apache-2.0

VERSION=1.14.0

SRCS = $(wildcard src/*.c)

all: boardid

boardid: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -DPROGRAM_VERSION=$(VERSION) -o $@ $^

check: boardid
	tests/run_tests.sh

clean:
	-rm -fr boardid tests/work

.PHONY: all clean check
