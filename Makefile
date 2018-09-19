
VERSION=1.1.1

boardid: $(wildcard src/*.c)
	$(CC) -Wall -O2 -DPROGRAM_VERSION=$(VERSION) -o $@ $^

check: boardid
	tests/run_tests.sh

clean:
	-rm -fr boardid tests/work

.PHONY: clean check
