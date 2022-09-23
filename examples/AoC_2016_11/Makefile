all: bitmap_rep string_rep

clean:
	rm bitmap_rep
	rm string_rep

bitmap_rep: bitmap_rep.cpp
	g++ -std=c++17 -O3 -I../.. -o bitmap_rep bitmap_rep.cpp

string_rep: string_rep.cpp
	g++ -std=c++17 -O3 -I../.. -o string_rep string_rep.cpp
