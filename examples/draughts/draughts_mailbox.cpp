#include "state.hpp"
#include <array>
#include <set>
#include <format>
#include <print>
#include <iostream>

enum class piece_type : uint8_t
{
	xx = 0, __ = 1, wp = 10, wk = 12, bp = 18, bk = 20
};
// cckp_

constexpr bool is_white(piece_type p) { return (static_cast<int>(p) & 24) == 8; }
constexpr bool is_black(piece_type p) { return (static_cast<int>(p) & 24) == 16; }
constexpr bool is_pawn(piece_type p) { return (static_cast<int>(p) & 6) == 2; }
constexpr bool is_king(piece_type p) { return (static_cast<int>(p) & 6) == 4; }

constexpr piece_type promote(const piece_type pc)
{
	switch (pc)
	{
	case piece_type::wp: return piece_type::wk;
	case piece_type::bp: return piece_type::bk;
	default: abort();
	}
}

constexpr auto xx = piece_type::xx;
constexpr auto __ = piece_type::__;
constexpr auto wp = piece_type::wp;
constexpr auto wk = piece_type::wk;
constexpr auto bp = piece_type::bp;
constexpr auto bk = piece_type::bk;

constexpr wchar_t pt2str(const piece_type p)
{
	switch (p)
	{
	case piece_type::wp: return L'⛀'; //'P';
	case piece_type::wk: return L'⛁'; //'K';
	case piece_type::bp: return L'⛂'; //'p';
	case piece_type::bk: return L'⛃'; //'k';
	case piece_type::__: return L'■';
	case piece_type::xx: return L'*';
	}
	return L'\0';
}

const char* toutf8(wchar_t in)
{
	static char tab[8] = {0};

	if (in <= 0x7f)
	{
		tab[1] = '\0';
		tab[0] = in;
	}
	else if (in <= 0x7ff)
	{
		tab[2] = '\0';
		tab[1] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[0] = 0xc0 | (in & 0x1f);
	}
	else if (in <= 0xffff)
	{
		tab[3] = '\0';
		tab[2] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[1] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[0] = 0xe0 | (in & 0x0f);
	}
	else if (in <= 0x10ffff)
	{
		tab[4] = '\0';
		tab[3] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[2] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[1] = 0x80 | (in & 0x3f);
		in >>= 6;
		tab[0] = 0xf0 | (in & 0x07);
	}
	else
		tab[0] = '\0';

	return tab;
}

class draughts : public game_state<draughts, std::string>
{
	static constexpr bool is_promoted(const int sq, const side_to_move stm)
	{
		return (stm == side_to_move::max_player && sq < 24) || (stm == side_to_move::min_player && sq > 120);
	}

	inline static const std::array<int, 50> pos2idx {
		14, 16, 18, 20, 22, 25, 27, 29, 31, 33,
		38, 40, 42, 44, 46, 49, 51, 53, 55, 57,
		62, 64, 66, 68, 70, 73, 75, 77, 79, 81,
		86, 88, 90, 92, 94, 97, 99,101,103,105,
		110,112,114,116,118,121,123,125,127,129
		};

	void resolve_captures(std::vector<int> &through, std::vector<std::vector<int>> &captures) const
	{
		static std::array<int, 144> frozen {0};

		const auto left_forward	= (stm == side_to_move::max_player ? -13 : 13);
		const auto left_backward  = (stm == side_to_move::max_player ? 11 : -11);
		const auto right_forward  = (stm == side_to_move::max_player ? -11 : 11);
		const auto right_backward = (stm == side_to_move::max_player ? 13 : -13);
		bool (*is_enemy)(piece_type) = (stm == side_to_move::max_player ? is_black : is_white);

		for (auto dir : {left_forward, left_backward, right_forward, right_backward})
		{
			if (is_pawn(board[through.front()]))
			{
				auto f = through.back() + dir, ff = f + dir;
				if (is_enemy(board[f]) && !frozen[f] && (board[ff] == __ || ff == through.front()))
				{	// bierka przeciwnika i dotychczas niezbita i (trafiamy na puste lub wracamy na wyjściowe)
					through.push_back(f);	// zbijany pionek zapamiętać
					through.push_back(ff);
					frozen[f] = 1;

					resolve_captures(through, captures);

					if (captures.empty() || through.size() == captures.front().size())
						captures.push_back(through);
					else if (captures.front().size() < through.size())
					{
						captures.clear();
						captures.push_back(through);
					}

					through.pop_back();
					through.pop_back();
					frozen[f] = 0;
				}
			}
			else if (is_king(board[through.front()]))
			{
				auto f = through.back() + dir;
				while (board[f] == __)
					f += dir;
				auto ff = f + dir;

				if (is_enemy(board[f]) && !frozen[f] && (board[ff] == __ || ff == through.front()))
				{
					through.push_back(f);	// zbijany pionek zapamiętać
					frozen[f] = 1;
					do
					{
						through.push_back(ff);

						resolve_captures(through, captures);

						if (captures.empty() || through.size() == captures.front().size())
							// zbite tyle samo
							captures.push_back(through);
						else if (captures.front().size() < through.size())
						{	// dłuższe bicie, więc poprzednie niedozwolone
							captures.clear();
							captures.push_back(through);
						}

						through.pop_back();

						ff += dir;
					} while (board[ff] == __);
					through.pop_back();
					frozen[f] = 0;
				}
			}
			else
				throw std::runtime_error(":(");
		}
	}
public:
	draughts() :
		game_state(side_to_move::max_player),
		/*	 00 01 02 03 04 05 06 07 08 09 10 11
		 * 00  -- -- -- -- -- -- -- -- -- -- -- --
		 * 12  -- ..  1 ..  2 ..  3 ..  4 ..  5 --
		 * 24  --  6 ..  7 ..  8 ..  9 .. 10 .. --
		 * 36  -- .. 11 .. 12 .. 13 .. 14 .. 15 --
		 * 48  -- 16 .. 17 .. 18 .. 19 .. 20 .. --
		 * 60  -- .. 21 .. 22 .. 23 .. 24 .. 25 --
		 * 72  -- 26 .. 27 .. 28 .. 29 .. 30 .. --
		 * 84  -- .. 31 .. 32 .. 33 .. 34 .. 35 --
		 * 96  -- 36 .. 37 .. 38 .. 39 .. 40 .. --
		 * 108 -- .. 41 .. 42 .. 43 .. 44 .. 45 --
		 * 120 -- 46 .. 47 .. 48 .. 49 .. 50 .. --
		 * 132 -- -- -- -- -- -- -- -- -- -- -- --
		 *
		 */
		/*board {
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx,
			xx, __, bp, __, bp, __, bp, __, bp, __, bp, xx,
			xx, bp, __, bp, __, bp, __, bp, __, bp, __, xx,
			xx, __, bp, __, bp, __, bp, __, bp, __, bp, xx,
			xx, bp, __, bp, __, bp, __, bp, __, bp, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, wp, __, wp, __, wp, __, wp, __, wp, xx,
			xx, wp, __, wp, __, wp, __, wp, __, wp, __, xx,
			xx, __, wp, __, wp, __, wp, __, wp, __, wp, xx,
			xx, wp, __, wp, __, wp, __, wp, __, wp, __, xx,
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx
		}*/
		board {
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx
		}
//		wp {31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50},
//		bp {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
		/*	 00 01 02 03 04 05 06 07 08 09 10 11
		 * 00  -- -- -- -- -- -- -- -- -- -- -- --
		 * 12  -- 00 01 02 03 04 05 06 07 08 09 --
		 * 24  -- 10 11 12 13 14 15 16 17 18 19 --
		 * 36  -- 20 21 22 23 24 25 26 27 28 29 --
		 * 48  -- 30 31 32 33 34 35 36 37 38 39 --
		 * 60  -- 40 41 42 43 44 45 46 47 48 49 --
		 * 72  -- 50 51 52 53 54 55 56 57 58 59 --
		 * 84  -- 60 61 62 63 64 65 66 67 68 69 --
		 * 96  -- 70 71 72 73 74 75 76 77 78 79 --
		 * 108 -- 80 81 82 83 84 85 86 87 88 89 --
		 * 120 -- 90 91 92 93 94 95 96 97 98 99 --
		 * 132 -- -- -- -- -- -- -- -- -- -- -- --
		 *
		 */
//		wp {86, 88, 90, 92, 94, 97, 99,101,103,105,110,112,114,116,118,121,123,125,127,129},
//		bp {14, 16, 18, 20, 22, 25, 27, 29, 31, 33, 38, 40, 42, 44, 46, 49, 51, 53, 55, 57},
//		empty {62, 64, 66, 68, 70, 73, 75, 77, 79, 81}
	{
		setup_board();
	}

	draughts(bool premove) : game_state(side_to_move::max_player)
	{
		for (auto pos : pos2idx)
			board[pos] = __;

		for (auto [pos, pc] : {std::make_pair(9, bp), {10, bp}, {12, bp}, {13, bp}, {17, bp}, {18, bp}, {20, bp}, {21, bp}, {22, bp}, {26, wp}, {28, wp}, {29, wp}, {31, wp}, {33, wp}, {34, wp}, {38, wp}, {41, wp}, {44, wp}})
			board[pos2idx[pos - 1]] = pc;

		if (premove)
			std::swap(board[pos2idx[20]], board[pos2idx[26]]);
		else
			stm = side_to_move::min_player;
	}

	draughts(const std::vector<int> &wp, const std::vector<int> &wk, const std::vector<int> &bp, const std::vector<int> &bk) : draughts()
	{
		for (auto pos : pos2idx)
			board[pos] = __;

		for (const auto &[piece, pos] : {std::make_pair(::wp, std::ref(wp)), {::wk, std::ref(wk)}, {::bp, std::ref(bp)}, {::bk, std::ref(bk)}})
			for (auto p : pos)
				if (board[pos2idx[p-1]] == __)
					board[pos2idx[p-1]] = piece;
				else
					throw std::runtime_error(":((");
	}

	bool operator== (const draughts &s) const { return board == s.board && stm == s.stm; }

	bool operator< (const draughts &s) const { return board < s.board && stm == s.stm; }

	size_t hashcode() const noexcept { return std::hash<std::string_view>{}({reinterpret_cast<const char*>(board.data()), board.size()}); }


	void clear_board()
	{
		board = {
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx
		};
	}

	void setup_board()
	{
		board = {
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx,
			xx, __, bp, __, bp, __, bp, __, bp, __, bp, xx,
			xx, bp, __, bp, __, bp, __, bp, __, bp, __, xx,
			xx, __, bp, __, bp, __, bp, __, bp, __, bp, xx,
			xx, bp, __, bp, __, bp, __, bp, __, bp, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, __, __, __, __, __, __, __, __, __, xx,
			xx, __, wp, __, wp, __, wp, __, wp, __, wp, xx,
			xx, wp, __, wp, __, wp, __, wp, __, wp, __, xx,
			xx, __, wp, __, wp, __, wp, __, wp, __, wp, xx,
			xx, wp, __, wp, __, wp, __, wp, __, wp, __, xx,
			xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx
		};
		stm = side_to_move::max_player;
	}

	void load_fen(const std::string &fen)
	{
		clear_board();
		bool side_set = false, is_king = false;
		piece_type pawn = xx, king = xx;
		int square_num = 0;
		for (const char c : fen)
		{
			if (c == 'K')
				is_king = true;
			else if (isdigit(c))
				square_num = square_num * 10 + c - '0';
			else if (c == 'W')
			{
				if (!side_set)
				{
					stm = side_to_move::max_player;
					side_set = true;
				}
				else
				{
					pawn = wp;
					king = wk;
				}
			}
			else if (c == 'B')
			{
				if (!side_set)
				{
					stm = side_to_move::min_player;
					side_set = true;
				}
				else
				{
					pawn = bp;
					king = bk;
				}
			}
			else if (c == ',' || c == '.' || c == ':')
			{
				if (square_num != 0)
					board[pos2idx.at(square_num - 1)] = (is_king ? king : pawn);
				is_king = false;
				square_num = 0;
			}
			else
			{
				std::cerr << "Unexpected '" << c << "'!" << std::endl;
				abort();
			}
		}
		if (square_num != 0)
			board[pos2idx[square_num - 1]] = (is_king ? king : pawn);

		if (!side_set)
			abort();
	}

	[[nodiscard]] std::string to_string(const bool gv=false) const
	{
		std::array<std::wstring, 10> board;
		board.fill(std::wstring(10, L'□'));

		for (auto pos : pos2idx)
		{
			auto row = pos / 12 - 1, col = pos % 12 - 1;
			board[row][col] = pt2str(this->board[pos]);
		}
//		const char *ptype = "PKpk";
//		for (auto s : {&wp, &wk, &bp, &bk})
//		{
//			for (auto i : *s)
//			{
////				auto row = (i - 1) / 5, col = 2 * ((i - 1) % 5);
////				if (!(row & 1))
////					++col;
//				auto row = i / 12 - 1, col = i % 12 - 1;
//				board[row][col] = *ptype;
//			}
//			ptype++;
//		}

		std::ostringstream out;
		if (gv)
		{
			out << "[label=<<table border=\"0\" cellspacing=\"0\">";
			bool odd = false;
			for (const auto &row : board)
			{
				bool brown = odd;
				out << "<tr>";
				for (const auto col : row)
				{
					out << "<td fixedsize=\"true\" width=\"20\" height=\"20\" align=\"center\" valign=\"middle\" bgcolor=\"" << (brown ? "chocolate" : "white") << "\">&#" << (int)col << ";</td>";
					brown = !brown;
				}
				out << "</tr>";
				odd = !odd;
			}
			out << "</table>>]";
		}
		else
		{
			for (const auto &row : board)
			{
				for (const auto col : row)
					out << toutf8(col);
				out << std::endl;
			}
//			for (size_t i = 0; i < board.size(); i++)
//				out << /*std::setw(2) << board.size() - i << ' ' <<*/ board[i].c_str() << std::endl;
//			out << "	abcdefghij" << std::endl;
		}

		return out.str();
	}

	std::vector<draughts> children() const
	{
		std::vector<draughts> result;
		bool (*is_own)(piece_type) = (stm == side_to_move::max_player ? is_white : is_black);
//		bool (*is_enemy)(piece_type) = (stm == side_to_move::max_player ? is_black : is_white);
		const auto left_forward	= (stm == side_to_move::max_player ? -13 : 13);
		const auto left_backward  = (stm == side_to_move::max_player ? 11 : -11);
		const auto right_forward  = (stm == side_to_move::max_player ? -11 : 11);
		const auto right_backward = (stm == side_to_move::max_player ? 13 : -13);

		std::vector<std::vector<int>> captures;
		std::vector<int> own_pieces, capture;
		for (auto pos : pos2idx)
		{
			auto pc = board[pos];
			if (is_own(pc))
			{
				own_pieces.push_back(pos);
				capture.push_back(pos);
				resolve_captures(capture, captures);
				capture.pop_back();
			}
		}

		if (!captures.empty())
		{
			result.clear();

			std::set<draughts> unique_captures;
			for (auto &path : captures)
			{
				auto child = *this;
				for (size_t i = 1; i < path.size(); i += 2)
					child.board[path[i]] = __;

				std::swap(child.board[path.front()], child.board[path.back()]);
				child.m_move = std::format("{}x{}", path.front() + 1, path.back() + 1);
				child.stm = !stm;
				if (is_pawn(child.board[path.back()]) && is_promoted(path.back(), stm))
					child.board[path.back()] = promote(child.board[path.back()]);

				unique_captures.emplace(std::move(child));
			}
			return std::vector(unique_captures.begin(), unique_captures.end());
		}
		for (const auto pos : own_pieces)
			if (is_pawn(board[pos]))
			{
				for (auto m : {left_forward, right_forward})
				{
					auto f = pos + m;
					if (board[f] == __)
					{
						result.push_back(*this);
						auto &child = result.back();
						child.stm = !stm;
						std::swap(child.board[pos], child.board[f]);
						child.m_move = std::format("{}-{}", pos + 1, f + 1);
						if (is_promoted(f, stm))
							child.board[f] = promote(child.board[f]);
					}
				}
			}
			else if (is_king(board[pos]))
			{
				for (auto m : {left_forward, right_forward, left_backward, right_backward})
				{
					auto f = pos + m;
					while (board[f] == __)
					{
						result.push_back(*this);
						auto &child = result.back();
						child.stm = !stm;
						std::swap(child.board[pos], child.board[f]);
						child.m_move = std::format("{}-{}", pos + 1, f + 1);
						f += m;
					}
				}
			}
			else
				abort();

		return result;
	}

	node_status is_terminal() const
	{
		return node_status::unresolved;
	}

	int evaluate() const
	{
		int men = 0, kings = 0, men_dist_from_promotion = 0;
		for (const auto i : pos2idx)
			switch (board[i])
			{
			case wp:
				++men;
				men_dist_from_promotion += i / 5;
				break;
			case wk:
				++kings;
				break;
			case bp:
				--men;
				men_dist_from_promotion -= 10 - i/5;
				break;
			case bk:
				--kings;
			default:
				break;
			}

		return 100*men + 200*kings + 10*men_dist_from_promotion;
	}
private:
	std::array<piece_type, 144> board;
};

template<>
struct std::hash<draughts>
{
	size_t operator()(const draughts &s) const noexcept { return s.hashcode(); }
};

#include <map>
template<typename T>
std::map<uint64_t, uint64_t>& dfs(const T &s, std::vector<T> (T::*children)() const, uint64_t max_lvl, uint64_t lvl = 0)
{
	static std::map<uint64_t, uint64_t> cnts;
	if (lvl == 0)
		cnts.clear();

	cnts[lvl]++;

	if (lvl == max_lvl)
		return cnts;

	for (const auto &t : (s.*children)())
		dfs(t, children, max_lvl, lvl + 1);

	return cnts;
}

template<typename T>
uint64_t dfs(uint64_t max_lvl, const T &s, std::vector<T> (T::*children)() const)
{
	if (max_lvl == 0)
		return 1;

	uint64_t cnts = 0;
	for (const auto &t : std::invoke(children, s))
		cnts += dfs(max_lvl - 1, t, children);

	return cnts;
}

template<typename T>
auto perft(const T &s, std::vector<T> (T::*children)() const, const uint64_t max_lvl)
{
	std::vector<uint64_t> cnts;
	cnts.reserve(max_lvl);
	for (uint64_t i = 1; i <= max_lvl; i++)
	{
		auto t1 = std::chrono::steady_clock::now();
		auto n = dfs(i, s, children);
		auto t2 = std::chrono::steady_clock::now();
		cnts.push_back(n);
		std::cout << "// " << std::setw(2) << i << std::setw(12) << n << ' ' << parse_time(std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1)) << ' ' << n*1000 / std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << " kn/s" << std::endl;
	}
	std::cout << std::endl;
	return cnts;
}

template<typename T>
void bfs(const T &s0, bool print=false)
{
	std::vector<std::pair<uint64_t, T>> to_visit{{0, s0}};
	uint64_t lvl = 0, id = 0, sum = 0;
	[[maybe_unused]] std::ostringstream nodes, edges;
	while (!to_visit.empty() && lvl <= 0)
	{
		std::cout << "// " << lvl++ << ":" << to_visit.size() << std::endl;
		sum += to_visit.size();
		auto old_visit = std::move(to_visit);
		for (const auto &[sid, s] : old_visit)
		{
			if (print)
				nodes << "\tnode" << sid << ' ' << s.to_string(true) << std::endl;
			for (auto &t : s.children())
			{
				if (print)
					edges << "\tnode" << sid << " -> node" << id + 1 << std::endl;
				to_visit.push_back({++id, std::move(t)});
			}
		}
	}
	std::cout << "// " << lvl << ":" << to_visit.size() << std::endl;
	sum += to_visit.size();
	std::cout << "// total: " << sum << std::endl;
	if (print)
	{
		for (const auto &[sid, s] : to_visit)
			nodes << "\tnode" << sid << ' ' << s.to_string(true) << std::endl;
		std::cout << "digraph {\n\tnode [shape=plaintext]\n" << nodes.str() << edges.str() << "}\n";
	}
}

void search_test()
{
	draughts state;
	state.load_fen("W:W28,31,32,33,34,35,39,40,46:B3,5,8,13,17,19,20,25,26");
	// state = state.children()[3].children()[0].children()[8];
	// for (const auto &t : state.children())
	// 	std::cout << t.get_move() << std::endl;
	// return;
	const int max_depth = 15;
	// for (int depth = 1; depth <= max_depth; depth++)
	//  {
	// 	alpha_beta_searcher srch(depth, &draughts_::evaluate);
	// 	srch.perform_search(draughts_::s_children, state, false);
	// 	std::cout << std::setw(10) << srch.get_best_move().second << std::setw(6) << srch.get_best_move().first << " #visited: " << srch.get_visited() << std::endl;
	// 	// std::cout << srch.get_best().second << " " << srch.get_visited() << std::endl;
	// }
	// std::cout << std::endl;
	// return;
	// for (int depth = 1; depth <= max_depth; depth++)
	// {
	// 	alpha_beta_searcher srch(depth, &draughts_::evaluate);
	// 	srch.perform_search(draughts_::s_children, state, true);
	// 	std::cout << srch.get_best().second << " " << srch.get_visited() << std::endl;
	// }
	// auto srch = search_with_tt_std(max_depth, &draughts_::children, &draughts_::evaluate);
	// auto srch = search_with_tt_fixed(max_depth, &draughts_::children, &draughts_::evaluate);
	auto srch = alpha_beta_searcher(max_depth, &draughts::children, &draughts::evaluate);
	srch.perform_search(state, true);
	std::cout << std::setw(10) << srch.get_best_move().second << std::setw(6) << srch.get_best_move().first << std::endl;
	// std::cout << "#visited: " << srch.get_visited() << std::endl;
	std::cout << srch.get_summary() << std::endl;
	srch.clear_tt();

	srch.perform_search(state, false);
	std::cout << '\n' << std::setw(10) << srch.get_best_move().second << std::setw(6) << srch.get_best_move().first << std::endl;
	std::cout << srch.get_summary() << std::endl;
}

template<typename duration_type>
void play(side_to_move player, unsigned search_depth, duration_type time_limit)
{
	draughts state;
	std::string move;
	std::vector<draughts> children;
	std::vector<draughts>::const_iterator it;
	auto srch = alpha_beta_searcher(search_depth, &draughts::children, &draughts::evaluate, time_limit);
	while (!(children = state.children()).empty())
	{
		std::cout << state.to_string() << std::endl;
		std::for_each(children.begin(), children.end(), [](const auto &s){std::cout << std::setw(6) << s.get_move();});
		std::cout << std::endl;
		if (state.get_stm() == player)
		{
			do
			{
				std::cout << "Your move: ";
				std::cin >> move;
			} while ((it = std::find_if(children.begin(), children.end(), [&move](const auto &s){return s.get_move() == move;})) == children.end());
			state = *it;
		}
		else
		{
			srch.perform_search(state);
			move = srch.get_best_move().first;
			// std::cout << "Computer's move: " << move << std::endl;
			std::println("Computer's move: {} (score: {})", move, srch.get_best_move().second);
			state = *std::find_if(children.begin(), children.end(), [&move](const auto &s){return s.get_move() == move;});
		}
	}
}

void test_cases()
{
	// some test cases taken from https://damforum.nl/viewtopic.php?t=2308
	for (const auto &[fen, depth, expected] : {
		std::make_tuple("W:W31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50:B1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20.", 11, std::vector<uint64_t>{9, 81, 658, 4265, 27117, 167140, 1049442, 6483961, 41022423, 258895763, 1665861398}),
		{"B:W6,9,10,11,20,21,22,23,30,K31,33,37,41,42,43,44,46:BK17,K24.", 9, {14, 55, 1168, 5432, 87195, 629010, 9041010, 86724219, 1216917193}},
		{"W:W25,27,28,30,32,33,34,35,37,38:B12,13,14,16,18,19,21,23,24,26.", 15, {6, 12, 30, 73, 215, 590, 1944, 6269, 22369, 88050, 377436, 1910989, 9872645, 58360286, 346184885}},
		{"W:WK31,K32,K33,K34,K35,K36,K37,K38,K39,K40,K41,K42,K43,K44,K45,K46,K47,K48,K49,K50:BK1,K2,K3,K4,K5,K6,K7,K8,K9,K10,K11,K12,K13,K14,K15,K16,K17,K18,K19,K20.", 11, {17, 79, 352, 1399, 7062, 37589, 217575, 1333217, 8558321, 58381162, 417920283}},
		{"W:W6,7,8,9,10:B41,42,43,44,45.", 9, {9, 81, 795, 7578, 86351, 936311, 11448262, 138362698, 1799526674}}
	})
	{
		draughts d;
		d.load_fen(fen);
		std::cout << fen << std::endl;
		auto got = perft(d, &draughts::children, depth);
		if (auto [it_got, it_expected] = std::ranges::mismatch(got, expected); it_got == got.end())
			std::cout << "OK" << std::endl;
		else
			std::println("differs on elem {}: got {}, expected {}", std::distance(got.begin(), it_got) + 1, *it_got, *it_expected);
		std::cout << std::string(80, '*') << std::endl;
	}
}

int main()
{
	// play(side_to_move::max_player, 32, 5s);
	// search_test();
	test_cases();
	return 0;
}

