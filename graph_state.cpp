#include "graph_state.hpp"
#include <cmath>

graph_state::graph_state()
	: parent {nullptr},
//	  score {std::nan(""), parent == nullptr ? 0 : std::get<1>(parent->score) + cost, std::nan("")}
	  fscore(std::nan("")),
	  gscore(0),
//	  gscore(parent == nullptr ? 0 : parent->gscore + cost),
	  hscore(std::nan(""))
{
}

graph_state::~graph_state()
{
}

bool graph_state::operator== (const graph_state &s) const
{
	return is_equal(s);
}

double graph_state::get_f() const
{
//	if (std::isnan(fscore))
//		const_cast<double&>(fscore) = gscore + get_h();
	return fscore;
}

double graph_state::get_g() const
{
	return gscore;
}

double graph_state::get_h() const
{
//	if (std::isnan(hscore))
//		const_cast<double&>(hscore) = get_heuristic_grade();
	return hscore;
}

void graph_state::update_score(double new_g)
{
	gscore = new_g;
	hscore = get_heuristic_grade();
	fscore = gscore + hscore;
}

const graph_state* graph_state::get_parent() const
{
	return parent;
}

void graph_state::set_parent(const graph_state *parent)
{
	this->parent = parent;
}

//std::string state::to_graphviz() const
//{
//	return "";
//}

double graph_state::get_heuristic_grade() const
{
	return 0;
}

namespace std
{
	ostream& operator << (ostream& out, const graph_state &s)
	{
		out << s.to_string();
		return out;
	}
}
