FLAGS=-w -Werror -pedantic -std=c++23 -O3

all: draughts

clean:
	rm -f draughts

draughts: draughts.cpp
	g++ $(FLAGS) -I../../template_based -o draughts draughts.cpp

