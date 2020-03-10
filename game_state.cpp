#include "game_state.hpp"

//game_state::game_state(const game_state *parent, const Move &move) : parent(parent), move_name(move)
//{
//}

game_state::game_state()
	: /*parent(nullptr), move("")*/
{
}

game_state::~game_state()
{
}

//const game_state* game_state::get_parent() const
//{
//	return parent;
//}

//void game_state::set_parent(const game_state *parent)
//{
//	this->parent = parent;
//}

//const std::string& game_state::get_move() const
//{
//	return move;
//}

//void game_state::set_move(const std::string &move)
//{
//	this->move = move;
//}

bool game_state::operator ==(const game_state &s) const
{
	return is_equal(s);
}

namespace std
{
	ostream& operator <<(ostream& out, const game_state& s)
	{
		return (out << s.to_string());
	}
}
