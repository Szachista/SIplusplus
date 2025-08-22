all: bitmap_rep string_rep bitmap_rep_templ

clean:
	rm bitmap_rep
	rm string_rep

bitmap_rep: bitmap_rep.cpp
	g++ -w -Werror -pedantic -std=c++20 -O3 -I../.. -o bitmap_rep bitmap_rep.cpp

string_rep: string_rep.cpp
	g++ -w -Werror -pedantic -std=c++20 -O3 -I../.. -o string_rep string_rep.cpp

bitmap_rep_templ: bitmap_rep_templ.cpp
	g++ -w -Werror -pedantic -std=c++20 -O3 -I../../template_based -o bitmap_rep_templ bitmap_rep_templ.cpp
