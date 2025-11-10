FLAGS=-w -Werror -pedantic -std=c++23 -O3

all: bitmap_rep string_rep bitmap_rep_templ

clean:
	rm -f bitmap_rep
	rm -f string_rep
	rm -f bitmap_rep_templ

bitmap_rep: bitmap_rep.cpp
	g++ $(FLAGS) -I../../deprecated -o bitmap_rep bitmap_rep.cpp

string_rep: string_rep.cpp
	g++ $(FLAGS) -I../../deprecated -o string_rep string_rep.cpp

bitmap_rep_templ: bitmap_rep_templ.cpp
	g++ $(FLAGS) -I../.. -o bitmap_rep_templ bitmap_rep_templ.cpp
