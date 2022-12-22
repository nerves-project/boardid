# SPDX-FileCopyrightText: 2020 Nerves Project Developers
#
# SPDX-License-Identifier: Apache-2.0

VERSION=1.13.1

SRCS = $(wildcard src/*.c)

all: boardid

boardid: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -DPROGRAM_VERSION=$(VERSION) -o $@ $^

check: boardid
	tests/run_tests.sh

clean:
	-rm -fr boardid tests/work

.PHONY: all clean check
