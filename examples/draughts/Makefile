FLAGS=-w -Werror -pedantic -std=c++23 -O3

all: draughts draughts_mailbox

clean:
	rm -f draughts draughts_mailbox

draughts: draughts.cpp
	g++ $(FLAGS) -I../.. -o draughts draughts.cpp

draughts_mailbox: draughts_mailbox.cpp
	g++ $(FLAGS) -I../.. -o draughts_mailbox draughts_mailbox.cpp

