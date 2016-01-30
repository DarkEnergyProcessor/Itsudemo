# Itsudemo Makefile

WHERE_ZLIB?=./zlib-1.2.8
WHERE_LODEPNG?=./lodepng
CFLAGS?=

all: gcc

gcc:
	g++ -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) $(CFLAGS) -c lodepng/lodepng.cpp src/*.cpp
	gcc -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) $(CFLAGS) -c zlib-1.2.8/*.c
	g++ -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) $(CFLAGS) -o test *.o -lws2_32
	-rm *.o

.PHONY: all
