
VERSION=0.2.0

boardid: $(wildcard src/*.c)
	$(CC) -std=c99 -Wall -O2 -DPROGRAM_VERSION=$(VERSION) -o $@ $^

check: boardid
	tests/run_tests.sh

clean:
	-rm -fr boardid tests/work

.PHONY: clean check
