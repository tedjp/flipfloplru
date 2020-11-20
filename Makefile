all: test

test: test.cpp flipfloplru.h
	g++ -std=c++17 -Wall -fmax-errors=5 -o $@ $<

clean:
	rm -f test

.PHONY: clean all
