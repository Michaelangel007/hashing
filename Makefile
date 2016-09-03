all: hash16 collide words qs

CFLAGS=-O2 -Wall
INC=hash16.h util_itoa.h util_timer.h

hash16: hash16.cpp $(INC)
	g++ $(CFLAGS) $< -o $@

collide: collide.cpp $(INC)
	g++ $(CFLAGS) $< -o $@

words: words.cpp $(INC)
	g++ $(CFLAGS) $< -o $@

qs: quicksort.cpp
	g++ $(CFLAGS) $< -o $@

