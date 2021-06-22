
VERSION=1.11.1

SRCS = $(wildcard src/*.c)

all: boardid

boardid: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -DPROGRAM_VERSION=$(VERSION) -o $@ $^

check: boardid
	tests/run_tests.sh

clean:
	-rm -fr boardid tests/work

.PHONY: all clean check
