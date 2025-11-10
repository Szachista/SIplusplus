#include "graph_state.hpp"
#include <cmath>

iterable_graph_state::iterable_graph_state()
	: parent {nullptr},
	  fscore(std::nan("")),
	  gscore(0),
	  hscore(std::nan(""))
{
}

iterable_graph_state::~iterable_graph_state()
{
}

bool iterable_graph_state::operator== (const iterable_graph_state &s) const
{
	return is_equal(s);
}

double iterable_graph_state::get_f() const
{
	return fscore;
}

double iterable_graph_state::get_g() const
{
	return gscore;
}

double iterable_graph_state::get_h() const
{
	return hscore;
}

void iterable_graph_state::update_score(double new_g)
{
	gscore = new_g;
	hscore = get_heuristic_grade();
	fscore = gscore + hscore;
}

const iterable_graph_state* iterable_graph_state::get_parent() const
{
	return parent;
}

void iterable_graph_state::set_parent(const iterable_graph_state *parent)
{
	this->parent = parent;
}

double iterable_graph_state::get_heuristic_grade() const
{
	return 0;
}

namespace std
{
	ostream& operator << (ostream& out, const iterable_graph_state &s)
	{
		out << s.to_string();
		return out;
	}
}
