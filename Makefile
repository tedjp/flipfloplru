test: test.cpp flipfloplru.h
	g++ -Wall -fmax-errors=5 -o $@ $<
