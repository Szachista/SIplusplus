#include "state.hpp"
#include <array>
#include <set>
#include <format>
#include <print>

enum class piece_type : uint8_t
{
	xx = 0, __ = 1, wp = 10, wk = 12, bp = 18, bk = 20
};
// cckp_

constexpr bool is_white(piece_type p) { return (static_cast<int>(p) & 24) == 8; }
constexpr bool is_black(piece_type p) { return (static_cast<int>(p) & 24) == 16; }
constexpr bool is_pawn(piece_type p) { return (static_cast<int>(p) & 6) == 2; }
constexpr bool is_king(piece_type p) { return (static_cast<int>(p) & 6) == 4; }
constexpr piece_type promote(piece_type pc)
{
	switch (pc)
	{
	case piece_type::wp: return piece_type::wk;
	case piece_type::bp: return piece_type::bk;
//	case piece_type::wk:
//	case piece_type::bk: return pc;
	default: abort();
	}
}

constexpr piece_type xx = piece_type::xx;
constexpr piece_type __ = piece_type::__;
constexpr piece_type wp = piece_type::wp;
constexpr piece_type wk = piece_type::wk;
constexpr piece_type bp = piece_type::bp;
constexpr piece_type bk = piece_type::bk;

constexpr wchar_t pt2str(piece_type p)
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
	return '\0';
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

/*template<>
struct std::less<std::vector<int>>
{
	bool operator()(const std::vector<int> &a, const std::vector<int> &b) const
	{
		if (a.size() != b.size())
			return false;
		if (a.front() != b.front())
			return a.front() < b.front();
		if (a.back() != b.back())
			return a.back() < b.back();
		return std::set(a.begin() + 1, a.end() - 1) < std::set(b.begin() + 1, b.end() - 1);
//		auto it_a = a.begin(), it_b = b.begin();
//		while (it_a != a.end() && it_b != b.end())
//		{
//			if (*it_a != *it_b)
//				return *it_a < *it_b;
//			++it_a;
//			++it_b;
//		}
//		if (it_a == a.end())
//			return it_b != b.end();
//		return false;
	}
};

template<>
struct std::hash<std::vector<int>>
{
	size_t operator()(const std::vector<int> &v) const
	{
		size_t h = 1;
		for (auto i : v)
			h *= i + 1;
		return h;
//		static const std::array<uint64_t, 144> rnds {
//			0xe8f696d7b657f9feull, 0xbebdce78879e0bb1ull, 0x218660f2bab764acull, 0xfe74bee2d8c0f7d9ull, 0x458d5cce72da412bull, 0x872ab4295aa26b78ull, 0x519ddf469b99aff0ull, 0x42aae9be0f2e537bull, 0xd86e1266da1815a1ull, 0xe173c2f9503ca085ull, 0x72366b1c2e34eca2ull, 0x04616fd1079d69eaull, 0x46e2d383caf9bd96ull, 0x0a037e0cbeb967aeull, 0x0dac645ef1d74fc3ull, 0x4f543314b5f576e5ull, 0x514160b4acc7cac3ull, 0xd9e35afd09c9f00dull, 0xfb51b7b5dc6561abull, 0x1d12565af4f5eb8bull, 0x3d6ab6395fc245c7ull, 0x9a4a50f09a0e0a6cull, 0x97765fc28c413d1full, 0x358bf8ac64d50c2eull, 0xac5498607acce205ull, 0xcd628d80e27ed797ull, 0xa7134f26ec7a2714ull, 0xf96f4d7432a41f94ull, 0x509474527ddf2e57ull, 0xb868e32396f46932ull, 0x46158973807fc903ull, 0x32fc3f7c20b5dffeull, 0x2c14b6dfe606d4e9ull, 0x5122e97d5e18c6ecull, 0x1bb0ca2dad117804ull, 0x44ee64afeae42c66ull, 0x5d6ae158f829cbdbull, 0x1fc3645f7d1d8801ull, 0x1d953fd7a42f8611ull, 0x0a3dda4550b3b846ull, 0xd2246ed68ea7760bull, 0x44562791b191b1d5ull, 0xc7f459e4b8d282daull, 0xe46aa6af08f4c804ull, 0x655cf43046b21978ull, 0x0f89b09ae6ad4c7full, 0x1266f99fc7eecac3ull, 0x8c8b5152ae8716a5ull, 0x66ff37f0bbd09e01ull, 0x96a38dbbbee1bcd9ull, 0x56b6e6a86fdf1d63ull, 0xe52efcfc2d6536acull, 0x25fcfbf1fdea78b7ull, 0xf97149831751424dull, 0xa28bea03aba1b87full, 0xde667bd7d7371b84ull, 0xa94e2313953ee84dull, 0xdf62a35bd186a3e4ull, 0x41af4ce47562775full, 0xce75664eb82d2beeull, 0x4cfd89dc5d3863bbull, 0x487a04f18510c5f4ull, 0x2734dde024d203bfull, 0xa1a7892bb1aeb6eeull, 0x2b5392dbc55fb0bcull, 0x0d130d53c66f0538ull, 0x9ecc10132e65dfebull, 0xeeaa5b063fb2cab8ull, 0x3b6eb817cd0c1d3cull, 0x29572a1a6344b7d2ull, 0x516741da4ed76be6ull, 0x946457d75ec15603ull, 0xe0226a072b60ebdbull, 0x33110f2be434d5f0ull, 0x7d01d0396d88a4c5ull, 0xfef85c115018a426ull, 0x24d5d4f09167270cull, 0x912efff302b62e1bull, 0x8d60db97032d8e85ull, 0xef433ddf459f364eull, 0xd0920a2735a23867ull, 0x381dd8688de071f7ull, 0x5a1ce3feb7085698ull, 0xbf98e1faead5c5d9ull, 0xc9bba9cf76bbf323ull, 0x4e5903bf298e9777ull, 0x3cc980b898492231ull, 0x91e9a82d140b36ceull, 0xdb9a8e53617c2759ull, 0x7b91a76ce290700dull, 0x0c1eab0f4a773407ull, 0x09bd5a46aaa7aa3aull, 0xbe678cf93afa543bull, 0x83f0f39bf4b011c8ull, 0xcc178066ae55c7c0ull, 0x11778fc49b0ae0a8ull, 0x2ed6d838337d3e81ull, 0xd3164acc0a381dc7ull, 0xfe976a8ee25ed09cull, 0x885e3e6230868194ull, 0x90aa46117435f35dull, 0x2d6214f5c35b2d82ull, 0x55e103acd78e21c0ull, 0x6d70fd90b3259a8eull, 0x8c6d0ef8c54d2556ull, 0x12b5318087cd83b3ull, 0x8d69b8b2d9cf4686ull, 0x318a00a2ab61f811ull, 0x8bae9e458dbf0875ull, 0x1adf9bcf1e2a919eull, 0x62d9c320c4a28ad9ull, 0x8e832dd525b7350cull, 0x3782c68fb4de7a24ull, 0x60b79a92ed000c90ull, 0x548a939a2befadb5ull, 0x1909c9466d4bc231ull, 0x0f85e108836e1973ull, 0x80e64b11ce47191cull, 0x5f8c295743f8354cull, 0xa374be2fbd64dffaull, 0x69f96dd58688477dull, 0xea71af6f96cf81e4ull, 0x0739a186d124a768ull, 0x7db4c8665fdd1c00ull, 0x942fd4a0667003beull, 0xbebd5aa3d8657986ull, 0xa995679c36ed8530ull, 0xa33355d168187116ull, 0x8b133079b8dd717aull, 0x02b93a8bcf8d0d28ull, 0xa3132b1582c2b93cull, 0x10c13ac96537eaa7ull, 0x98dcba2978c7b3acull, 0xf99f621c1fe0fdd3ull, 0xf97f996e8afd20dcull, 0x01d3dcd06e211181ull, 0x2a0e4fe31a7efcbdull, 0x0ffb0099ec0422c2ull, 0xd802f89c6b2ac6f9ull, 0xcd661ee176d546d9ull, 0x9e5cf0d0bbd521d6ull, 0x54834661d0ba6f49ull, 0x35c6e4830f5738ecull, 0x44df9732a288d175ull
//		};
//		uint64_t h = 0;
//		for (auto vi : v)
//			h ^= rnds[vi];
//		return (size_t)h;
	}
};

template<>
struct std::equal_to<std::vector<int>>
{
	bool operator()(const std::vector<int> &a, const std::vector<int> &b) const
	{
		if (a.size() != b.size() || a.front() != b.front() || a.back() != b.back())
			return false;

		std::unordered_set<int> a_taken, b_taken;
		for (size_t i = 2; i < a.size(); i += 2)
		{
			a_taken.insert(a[i]);
			b_taken.insert(b[i]);
		}
		return a_taken == b_taken;

//		return std::unordered_set(a.begin(), a.end()) == std::unordered_set(b.begin(), b.end());
//		return std::unordered_multiset<int>(a.begin(), a.end()) == std::unordered_multiset<int>(b.begin(), b.end());
	}
};*/

class draughts_ : public game_state<draughts_, std::string>
{
private:
	std::array<piece_type, 50> m_board;

	inline static constexpr std::array<int, 50> asc_left {
		6, 7, 8, 9, -1,
		10, 11, 12, 13, 14,
		16, 17, 18, 19, -1,
		20, 21, 22, 23, 24,
		26, 27, 28, 29, -1,
		30, 31, 32, 33, 34,
		36, 37, 38, 39, -1,
		40, 41, 42, 43, 44,
		46, 47, 48, 49, -1,
		-1, -1, -1, -1, -1
	}, desc_right {
		-1, -1, -1, -1, -1,
		-1, 0, 1, 2, 3,
		5, 6, 7, 8, 9,
		-1, 10, 11, 12, 13,
		15, 16, 17, 18, 19,
		-1, 20, 21, 22, 23,
		25, 26, 27, 28, 29,
		-1, 30, 31, 32, 33,
		35, 36, 37, 38, 39,
		-1, 40, 41, 42, 43
	}, asc_right {
		5, 6, 7, 8, 9,
		-1, 10, 11, 12, 13,
		15, 16, 17, 18, 19,
		-1, 20, 21, 22, 23,
		25, 26, 27, 28, 29,
		-1, 30, 31, 32, 33,
		35, 36, 37, 38, 39,
		-1, 40, 41, 42, 43,
		45, 46, 47, 48, 49,
		-1, -1, -1, -1, -1
	}, desc_left {
		-1, -1, -1, -1, -1,
		0, 1, 2, 3, 4,
		6, 7, 8, 9, -1,
		10, 11, 12, 13, 14,
		16, 17, 18, 19, -1,
		20, 21, 22, 23, 24,
		26, 27, 28, 29, -1,
		30, 31, 32, 33, 34,
		36, 37, 38, 39, -1,
		40, 41, 42, 43, 44
	};

	static constexpr bool is_promoted(int sq, side_to_move stm)
	{
		return (stm == side_to_move::max_player && sq < 5)
				|| (stm == side_to_move::min_player && sq > 44);
	}

	void resolve_captures(std::vector<int> &through, std::vector<std::vector<int>> &captures) const
	{
		static std::array<int, 50> frozen {0};

		bool (*is_enemy)(piece_type) = (stm == side_to_move::max_player ? is_black : is_white);

		for (auto m : {&asc_left, &asc_right, &desc_left, &desc_right})
		{
			if (is_pawn(m_board[through.front()]))
			{
				auto f = (*m)[through.back()];
				if (f == -1)
					continue;

				auto ff = (*m)[f];
				if (ff == -1)
					continue;

				if (is_enemy(m_board[f]) && !frozen[f] && (m_board[ff] == __ || ff == through.front()))
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
			else if (is_king(m_board[through.front()]))
			{
				auto f = (*m)[through.back()];
				while (f != -1 && m_board[f] == __)
					f = (*m)[f];

				if (f == -1)
					continue;

				auto ff = (*m)[f];
				if (ff == -1)
					continue;

				if (is_enemy(m_board[f]) && !frozen[f] && (m_board[ff] == __ || ff == through.front()))
				{
					through.push_back(f);	// zbijany pionek zapamiętać
					do
					{
						through.push_back(ff);
						frozen[f] = 1;

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
						frozen[f] = 0;

						ff = (*m)[ff];	// analizowane wszystkie zatrzymania za zbijanym pionkiem
					} while (ff != -1 && m_board[ff] == __);
					through.pop_back();
				}
			}
			else
				throw std::runtime_error(":(");
		}
	}

public:
	draughts_() : game_state(side_to_move::max_player)
	{
		setup_board();
	}

	bool operator== (const draughts_ &s) const { return m_board == s.m_board && stm == s.stm; }

	bool operator< (const draughts_ &s) const { return m_board < s.m_board && stm == s.stm; }

	size_t hashcode() const { return std::hash<std::string_view>{}({(char*)&m_board[0], m_board.size()}); }

	void clear_board()
	{
		m_board.fill(__);
//		cnts = counts {0,0,0,0};
	}

	void setup_board()
	{
		m_board.fill(__);
		for (size_t	i = 0; i < 20; i++)
		{
			m_board[i] = bp;
			m_board.rbegin()[i] = wp;
		}
//		cnts.white_men = cnts.black_men = 20;
//		cnts.white_kings = cnts.black_kings = 0;
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
				{
					m_board[square_num - 1] = (is_king ? king : pawn);
//					if (is_king)
//						(is_white(king) ? cnts.white_kings : cnts.black_kings)++;
//					else
//						(is_white(pawn) ? cnts.white_men : cnts.black_men)++;
				}
				is_king = false;
				square_num = 0;
			}
			else
			{
				std::cout << "Unexpected '" << c << "'!" << std::endl;
				abort();
			}
		}
		if (square_num != 0)
		{
			m_board[square_num - 1] = (is_king ? king : pawn);
//			if (is_king)
//				(is_white(king) ? cnts.white_kings : cnts.black_kings)++;
//			else
//				(is_white(pawn) ? cnts.white_men : cnts.black_men)++;
		}
		if (!side_set)
			abort();
	}

	[[nodiscard]] std::string to_string(bool gv=false) const
	{
		std::array<std::wstring, 10> board;
		board.fill(std::wstring(10, L'□'));

		size_t col_off = 0;
		for (size_t pos = 0; pos < this->m_board.size(); pos++)
		{
			if (pos % 5 == 0)
				col_off ^= 1;
			auto row = pos / 5, col = pos % 5;
			board[row][2*col + col_off] = pt2str(this->m_board[pos]);
		}

		std::ostringstream out;
		if (gv)
		{
			out << "[label=<<table border=\"0\" cellspacing=\"0\">";
			bool odd = false;
			for (const auto &row : board)
			{
				bool brown = odd;
				out << "<tr>";
				for (auto col : row)
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
			// out << m_move << std::endl;
			for (const auto &row : board)
			{
				for (auto col : row)
					out << std::setw(4) << toutf8(col);
				out << std::endl;
			}
		}

		return out.str();
	}

	[[nodiscard]] std::vector<draughts_> children() const
	{
		std::vector<draughts_> result;

		bool (*is_own)(piece_type) = (stm == side_to_move::max_player ? is_white : is_black);
		const auto &left_forward = (stm == side_to_move::max_player ? desc_right : asc_left);
		const auto &right_forward = (stm == side_to_move::max_player ? desc_left : asc_right);

		std::vector<std::vector<int>> captures;
		std::vector<int> own_pieces, capture;
		for (size_t pos = 0; pos < m_board.size(); pos++)
		{
			auto pc = m_board[pos];
			if (is_own(pc))
			{
				own_pieces.push_back(pos);

				capture.push_back((int)pos);
				resolve_captures(capture, captures);
				capture.pop_back();
			}
		}

		if (!captures.empty())
		{
			result.clear();
//			std::vector<std::pair<int, int>> duplicates;
//			std::unordered_set<std::vector<int>> duplicates;
//			std::cout << "// *** " << captures.size() << " ***" << std::endl;

			std::set<draughts_> unique_captures;
//			auto board_copy = board;
			for (auto &path : captures)
			{
//				std::cout << "//";
//				auto pc = path;
//				std::sort(pc.begin() + 1, pc.end() - 1);
//				std::for_each(pc.begin(), pc.end(), [](auto i){std::cout<<std::setw(3)<<i;});
//				std::cout << std::endl;
//				if (duplicates.contains(path))
//					continue;

//				if (std::find(duplicates.begin(), duplicates.end(), std::make_pair(path.front(), path.back())) != duplicates.end())
//					continue;
//				duplicates.emplace_back(path.front(), path.back());

//				result.push_back(*this);
//				auto &child = result.back();
				auto child = *this;
				for (size_t i = 1; i < path.size(); i += 2)
				{
//					if (stm == side_to_move::max_player)
//						(is_pawn(board[path[i]]) ? child.cnts.black_men : child.cnts.black_kings)--;
//					else
//						(is_pawn(board[path[i]]) ? child.cnts.white_men : child.cnts.white_kings)--;
					child.m_board[path[i]] = __;
				}
				std::swap(child.m_board[path.front()], child.m_board[path.back()]);
				// child.m_move = {path.front(), path.back()};
				child.m_move = std::format("{}x{}", path.front() + 1, path.back() + 1);
				child.stm = !stm;
				if (is_pawn(child.m_board[path.back()]) && !is_king(child.m_board[path.back()]) && is_promoted(path.back(), stm))
				{
					child.m_board[path.back()] = promote(child.m_board[path.back()]);
//					if (stm == side_to_move::max_player)
//					{
//						child.cnts.white_men--;
//						child.cnts.white_kings++;
//					}
//					else
//					{
//						child.cnts.black_men--;
//						child.cnts.black_kings++;
//					}
				}

				unique_captures.emplace(std::move(child));
//				duplicates.insert(std::move(path));
			}
			return std::vector<draughts_>(unique_captures.begin(), unique_captures.end());
		}
		else
			for (auto pos : own_pieces)
				if (is_pawn(m_board[pos]))
				{
					for (auto f : {left_forward[pos], right_forward[pos]})
					{
						if (f != -1 && m_board[f] == __)
						{
							result.push_back(*this);
							auto &child = result.back();
							child.stm = !stm;
							std::swap(child.m_board[pos], child.m_board[f]);
							// child.m_move = {pos, f};
							child.m_move = std::format("{}-{}", pos + 1, f + 1);
							if (is_promoted(f, stm))
							{
								child.m_board[f] = promote(child.m_board[f]);
//								if (stm == side_to_move::max_player)
//								{
//									child.cnts.white_men--;
//									child.cnts.white_kings++;
//								}
//								else
//								{
//									child.cnts.black_men--;
//									child.cnts.black_kings++;
//								}
							}
						}
					}
				}
				else if (is_king(m_board[pos]))
				{
					for (auto m : {&asc_left, &asc_right, &desc_left, &desc_right})
					{
						auto f = (*m)[pos];
						while (f != -1 && m_board[f] == __)
						{
							result.push_back(*this);
							auto &child = result.back();
							child.stm = !stm;
							std::swap(child.m_board[pos], child.m_board[f]);
							// child.m_move = {pos, f};
							child.m_move = std::format("{}-{}", pos + 1, f + 1);
							f = (*m)[f];
						}
					}
				}
				else
					abort();

		return result;
	}

	static auto s_children(const draughts_ &s) { return s.children(); }

	node_status is_terminal() const
	{
		return node_status::unresolved;
	}

	int evaluate() const
	{
//		return (cnts.white_men - cnts.black_men) + 1.75 * (cnts.white_kings - cnts.black_kings);
		int men = 0, kings = 0, men_dist_from_promotion = 0;
		for (size_t i = 0; i < m_board.size(); i++)
			switch (m_board[i])
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
};

template<>
struct std::hash<draughts_>
{
	size_t operator()(const draughts_ &s) const { return s.hashcode(); }
};

class draughts : public game_state<draughts, std::string>
{
	static constexpr bool is_promoted(int sq, side_to_move stm)
	{
		return (stm == side_to_move::max_player && sq < 24)
				|| (stm == side_to_move::min_player && sq > 120);
	}

	inline static const std::array<int, 50> pos2idx {
		14, 16, 18, 20, 22, 25, 27, 29, 31, 33,
		38, 40, 42, 44, 46, 49, 51, 53, 55, 57,
		62, 64, 66, 68, 70, 73, 75, 77, 79, 81,
		86, 88, 90, 92, 94, 97, 99,101,103,105,
		110,112,114,116,118,121,123,125,127,129
		};
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

	void load_fen(const std::string &fen)
	{
		clear_board();
		bool side_set = false;
		enum class tag {turn, colour1, colour2, k, squarenum} curr = tag::turn;
		piece_type pawn = xx, king = xx;
		int square_num = 0;
		for (const char c : fen)
		{
			if (!side_set)
			{
				if (c == 'W')
					stm = side_to_move::max_player;
				else if (c == 'B')
					stm = side_to_move::min_player;
				else
					abort();
				side_set = true;
			}
			else if (c == 'K')
				curr = tag::k;
			else if (std::isdigit(c))
				square_num = square_num * 10 + c - '0';
			else if (c == 'W')
			{
				pawn = wp;
				king = wk;
			}
			else if (c == 'B')
			{
				pawn = bp;
				king = bk;
			}
			else if (c == ',' || c == '.' || c == ':')
			{
				if (square_num != 0)
					board[pos2idx.at(square_num - 1)] = (curr == tag::k ? king : pawn);
				curr = tag::squarenum;
				square_num = 0;
			}
		}
//		board[pos2idx[square_num - 1]] = (curr == tag::k ? king : pawn);
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

	std::string to_string(bool gv=false) const
	{
		std::array<std::wstring, 10> board;
		std::for_each(board.begin(), board.end(), [](std::wstring &s){s.resize(10, L' ');});

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
				for (auto col : row)
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
				for (auto col : row)
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
		for (auto pos : pos2idx)
		{
			auto pc = board[pos];
			if (is_own(pc))
			{
				std::vector<int> capture {pos};
				resolve_captures(capture, captures);

				if (!captures.empty())
					continue;
				else if (is_pawn(pc))
				{
					for (auto m : {left_forward, right_forward})
					{
						auto f = pos + m/*, ff = f + m*/;
						/*if (is_enemy(board[f]) && board[ff] == __)
						{	// bicie
							if (!result.empty() && captures.empty())
								result.clear();
							are_there_captures = true;
							std::cerr << to_string();
							std::vector<int> capture {pos2idx[i]};
							resolve_captures(capture, captures);
//							std::unordered_set<int> frozen {f};
//							std::stack<std::pair<int, int>> s;
//							s.emplace(m, ff);
//							while (!s.empty())
//							{
//								auto [direction, curr] = s.top();
//								s.pop();
//								for (auto dir : {left_forward, left_backward, right_forward, right_backward})
//									if (dir != -direction)
//									{
//										f = curr + dir;
//										ff = f + dir;
//										if (is_enemy(board[f]) && board[ff] == __ && !frozen.contains(f))
//										{
//											frozen.insert(f);
//										}
//									}
//							}
							for (const auto &c : captures)
							{
								for (auto ci : c)
									std::cerr << ci << " ";
								std::cerr << std::endl;
							}
							std::cerr << std::endl;
							result.push_back(*this);
						}
						else*/ if (board[f] == __)
						{
							result.push_back(*this);
							auto &child = result.back();
							child.stm = !stm;
							std::swap(child.board[pos], child.board[f]);
							if (is_promoted(f, stm))
								child.board[f] = promote(child.board[f]);
						}
					}
				}
				else if (is_king(pc))
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
							f += m;
						}
					}
				}
				else
					abort();
			}
		}

		if (!captures.empty())
		{
			result.clear();
			std::vector<std::pair<int, int>> duplicates;
//			std::unordered_set<std::vector<int>> duplicates;
			for (auto &path : captures)
			{
//				if (duplicates.contains(path))
				if (std::find(duplicates.begin(), duplicates.end(), std::make_pair(path.front(), path.back())) != duplicates.end())
					continue;

//				duplicates.emplace_back(path.front(), path.back());

				result.push_back(*this);
				auto &child = result.back();
				for (size_t i = 1; i < path.size(); i+=2)
					child.board[path[i]] = __;
				std::swap(child.board[path.front()], child.board[path.back()]);
				child.stm = !stm;
				if (!is_king(child.board[path.back()]) && is_promoted(path.back(), stm))
					child.board[path.back()] = promote(child.board[path.back()]);
//				duplicates.insert(std::move(path));
			}
		}

		/*auto pawns = (stm == side_to_move::max_player ? &draughts::wp : &draughts::bp);
//		auto kings = (stm == side_to_move::max_player ? &draughts::wk : &draughts::bk);
//		std::unordered_set<int> enemy_pieces;
//		if (stm == side_to_move::max_player)
//		{
//			enemy_pieces.insert(bp.begin(), bp.end());
//			enemy_pieces.insert(bk.begin(), bk.end());
//		}
//		else
//		{
//			enemy_pieces.insert(wp.begin(), wp.end());
//			enemy_pieces.insert(wk.begin(), wk.end());
//		}
		auto enemy_pawns = (stm == side_to_move::max_player ? &draughts::bp : &draughts::wp);
		auto enemy_kings = (stm == side_to_move::max_player ? &draughts::bk : &draughts::wk);
		const auto left_forward	= (stm == side_to_move::max_player ? -13 : 13);
		const auto left_backward  = (stm == side_to_move::max_player ? 11 : -11);
		const auto right_forward  = (stm == side_to_move::max_player ? -11 : 11);
		const auto right_backward = (stm == side_to_move::max_player ? 13 : -13);

//		// bicia (na razie tylko piony)

		// klucz - gdzie się zatrzymał, wartość - zbite figury przeciwnika
//		std::unordered_map<int, std::vector<int>> bicia;
		for (auto p : this->*pawns)
			for (auto m : {left_forward, right_forward})
			{
				auto f = p + m, ff = f + m;
				if (!sentinel.contains(f) && ((this->*enemy_pawns).contains(f) || (this->*enemy_kings).contains(f)) && !sentinel.contains(ff) && empty.contains(ff))
				{
					std::cout << to_string() << std::endl;
					result.push_back(*this);
				}
			}

		if (!result.empty())
			// były bicia
			return result;

		// ruchy do przodu
		for (auto p : this->*pawns)
			for (auto m : {left_forward, right_forward})
			{
				auto f = p + m;
//				if (!sentinel.contains(f) && !(this->*pawns).contains(f) && !(this->*kings).contains(f) && !(this->*enemy_pawns).contains(f) && !(this->*enemy_kings).contains(f))
				if (!sentinel.contains(f) && !wp.contains(f) && !wk.contains(f) && !bp.contains(f) && !bk.contains(f))
				{
					result.push_back(*this);
					(result.back().*pawns).erase(p);
					(result.back().*pawns).insert(f);
					result.back().stm = !result.back().stm;
				}
			}*/

		return result;
	}

	bool operator== (const draughts &s) const { return board == s.board && stm == s.stm; }

	size_t hashcode() const { return std::hash<std::string_view>{}({(char*)&board[0], board.size()}); }
private:
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
					do
					{
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

						ff += dir;
					} while (board[ff] == __);
				}
			}
			else
				throw std::runtime_error(":(");
		}
	}

	std::array<piece_type, 144> board;
//	std::array<std::array<piece_type, 12>, 12> board;
//	std::set<int> wp, wk, bp, bk, empty;
//	inline static const std::unordered_set<int> sentinel{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 23, 24, 35, 36, 47, 48, 59, 60, 71, 72, 83, 84, 95, 96, 107, 108, 119, 120, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143};
};

template<>
struct std::hash<draughts>
{
	size_t operator()(const draughts &s) const { return s.hashcode(); }
};

#include <map>
template<typename T>
std::map<size_t, size_t>& dfs(const T &s, std::vector<T> (T::*children)() const, size_t max_lvl, size_t lvl = 0)
{
	static std::map<size_t, size_t> cnts;
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
size_t dfs(size_t max_lvl, const T &s, std::vector<T> (T::*children)() const)
{
	if (max_lvl == 0)
		return 1;

	size_t cnts = 0;
	for (const auto &t : (s.*children)())
		cnts += dfs(max_lvl - 1, t, children);

	return cnts;
}

template<typename T>
void perft(const T &s, std::vector<T> (T::*children)() const, size_t max_lvl)
{
	for (size_t i = 1; i <= max_lvl; i++)
	{
		auto t1 = std::chrono::steady_clock::now();
		auto n = dfs(i, s, children);
		auto t2 = std::chrono::steady_clock::now();
		std::cout << "// " << std::setw(2) << i << std::setw(12) << n << ' ' << parse_time(std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1)) << ' ' << n*1000 / std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << " kn/s" << std::endl;
	}
	std::cout << std::endl;
}

template<typename T>
void bfs(const T &s0, bool print=false)
{
	std::vector<std::pair<size_t, T>> to_visit{{0, s0}};
	size_t lvl = 0, id = 0, sum = 0;
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
	draughts_ state;
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
	auto srch = alpha_beta_searcher(max_depth, &draughts_::children, &draughts_::evaluate);
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
	draughts_ state;
	std::string move;
	std::vector<draughts_> children;
	std::vector<draughts_>::const_iterator it;
	auto srch = alpha_beta_searcher(search_depth, &draughts_::children, &draughts_::evaluate, time_limit);
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

int main()
{
	play(side_to_move::max_player, 32, 5s);
	// search_test();
	return 0;
	// brak damek - OK
//	const auto fen = "W:W31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50:B1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20."; const int depth = 11;
	/*
	 * perft(1) 9 nodes, 0.00 sec, 9 knodes/sec
	 * perft(2) 81 nodes, 0.00 sec, 81 knodes/sec
	 * perft(3) 658 nodes, 0.00 sec, 658 knodes/sec
	 * perft(4) 4265 nodes, 0.00 sec, 4265 knodes/sec
	 * perft(5) 27117 nodes, 0.00 sec, 27117 knodes/sec
	 * perft(6) 167140 nodes, 0.00 sec, 167140 knodes/sec
	 * perft(7) 1049442 nodes, 0.03 sec, 32795 knodes/sec
	 * perft(8) 6483961 nodes, 0.22 sec, 29473 knodes/sec
	 * perft(9) 41022423 nodes, 1.30 sec, 31629 knodes/sec
	 * perft(10) 258895763 nodes, 8.33 sec, 31080 knodes/sec
	 * perft(11) 1665861398 nodes, 55.86 sec, 29822 knodes/sec
	 */

	// OK
	const auto fen = "B:W6,9,10,11,20,21,22,23,30,K31,33,37,41,42,43,44,46:BK17,K24."; const int depth = 9;
	/* perft -d9 -f B:W6,9,10,11,20,21,22,23,30,K31,33,37,41,42,43,44,46:BK17,K24.
	 * perft(1) 14 nodes, 0.00 sec, 14 knodes/sec
	 * perft(2) 55 nodes, 0.00 sec, 55 knodes/sec
	 * perft(3) 1168 nodes, 0.00 sec, 1168 knodes/sec
	 * perft(4) 5432 nodes, 0.00 sec, 5432 knodes/sec
	 * perft(5) 87195 nodes, 0.00 sec, 87195 knodes/sec
	 * perft(6) 629010 nodes, 0.03 sec, 19657 knodes/sec
	 * perft(7) 9041010 nodes, 0.31 sec, 28885 knodes/sec
	 * perft(8) 86724219 nodes, 2.69 sec, 32251 knodes/sec
	 * perft(9) 1216917193 nodes, 30.27 sec, 40207 knodes/sec
	 */

	// FAIL - na głębokości 11 za mało o 2 stany
//	const auto fen = "W:W25,27,28,30,32,33,34,35,37,38:B12,13,14,16,18,19,21,23,24,26."; const int depth = 15;
	/* perft -d15 -f W:W25,27,28,30,32,33,34,35,37,38:B12,13,14,16,18,19,21,23,24,26.
	 * perft(1) 6 nodes, 0.00 sec, 6 knodes/sec
	 * perft(2) 12 nodes, 0.00 sec, 12 knodes/sec
	 * perft(3) 30 nodes, 0.00 sec, 30 knodes/sec
	 * perft(4) 73 nodes, 0.00 sec, 73 knodes/sec
	 * perft(5) 215 nodes, 0.00 sec, 215 knodes/sec
	 * perft(6) 590 nodes, 0.00 sec, 590 knodes/sec
	 * perft(7) 1944 nodes, 0.00 sec, 1944 knodes/sec
	 * perft(8) 6269 nodes, 0.00 sec, 6269 knodes/sec
	 * perft(9) 22369 nodes, 0.00 sec, 22369 knodes/sec
	 * perft(10) 88050 nodes, 0.02 sec, 5503 knodes/sec
	 * perft(11) 377436 nodes, 0.02 sec, 22202 knodes/sec
	 * perft(12) 1910989 nodes, 0.08 sec, 24190 knodes/sec
	 * perft(13) 9872645 nodes, 0.42 sec, 23340 knodes/sec
	 * perft(14) 58360286 nodes, 2.33 sec, 25058 knodes/sec
	 * perft(15) 346184885 nodes, 17.19 sec, 20140 knodes/sec
	 */

	// OK
//	const auto fen = "W:WK31,K32,K33,K34,K35,K36,K37,K38,K39,K40,K41,K42,K43,K44,K45,K46,K47,K48,K49,K50:BK1,K2,K3,K4,K5,K6,K7,K8,K9,K10,K11,K12,K13,K14,K15,K16,K17,K18,K19,K20.";  const int depth = 11;
	/*
	 * perft(1) 17 nodes, 0.001 sec, 0.017 Mnodes/sec
	 * perft(2) 79 nodes, 0.001 sec, 0.079 Mnodes/sec
	 * perft(3) 352 nodes, 0.001 sec, 0.352 Mnodes/sec
	 * perft(4) 1399 nodes, 0.001 sec, 1.399 Mnodes/sec
	 * perft(5) 7062 nodes, 0.017 sec, 0.415412 Mnodes/sec
	 * perft(6) 37589 nodes, 0.016 sec, 2.34931 Mnodes/sec
	 * perft(7) 217575 nodes, 0.111 sec, 1.96014 Mnodes/sec
	 * perft(8) 1333217 nodes, 0.594 sec, 2.24447 Mnodes/sec
	 * perft(9) 8558321 nodes, 3.642 sec, 2.3499 Mnodes/sec
	 * perft(10) 58381162 nodes, 23.033 sec, 2.53467 Mnodes/sec
	 * perft(11) 417920283 nodes, 198.126 sec, 2.10937 Mnodes/sec
	 */

	// FAIL - na głębokości 8 wyszło ------962
//	const auto fen = "W:W6,7,8,9,10:B41,42,43,44,45."; const int depth = 9;
	/*
	 * perft(1) 9 nodes, 0.001 sec, 0.009 Mnodes/sec
	 * perft(2) 81 nodes, 0.001 sec, 0.081 Mnodes/sec
	 * perft(3) 795 nodes, 0.001 sec, 0.795 Mnodes/sec
	 * perft(4) 7578 nodes, 0.001 sec, 7.578 Mnodes/sec
	 * perft(5) 86351 nodes, 0.001 sec, 86.351 Mnodes/sec
	 * perft(6) 936311 nodes, 0.079 sec, 11.852 Mnodes/sec
	 * perft(7) 11448262 nodes, 0.891 sec, 12.8488 Mnodes/sec
	 * perft(8) 138362698 nodes, 10.767 sec, 12.8506 Mnodes/sec
	 * perft(9) 1799526674 nodes, 144.47 sec, 12.4561 Mnodes/sec
	 */

//	const auto fen = "W:W46:B41,42,43,44,31,32,33,34,21,22,23,24,11,12,13,14.";
//	const auto fen = "W:W41:B37,38,39,40,27,28,29,30,17,18,19,20,7,8,9,10.";
//	const auto fen = "W:WK1:B8,9,10,11,12,19,20,21,22,23,30,31,32,33,34,41,42,43,44.";
//	const auto fen = "W:WK1:B9,10,11,12,13,20,21,22,23,24,31,32,33,34,41,42,43,44.";
	draughts d;
//	draughts d{{46},{},{11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44}, {}};
//	draughts d{{},{1},{9, 10, 11, 12, 13, 20, 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44}, {}};
//	draughts d{{47},{},{41, 42, 31, 32, 22, 14, 13}, {}};
	d.load_fen(fen);
//	std::cout << d.to_string() << std::endl; return 0;

//	auto t1 = std::chrono::steady_clock::now();
//	bfs(d,true); return 0;
//	auto t2 = std::chrono::steady_clock::now();
//	std::cout << parse_time(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)) << std::endl;

//	perft(d, &draughts::children, depth);
//	t1 = std::chrono::steady_clock::now();
//	auto& cnts = dfs(d, &draughts::children, depth);
//	t2 = std::chrono::steady_clock::now();
//	for (auto [lvl, cnt] : cnts)
//		std::cout << "// " << lvl << ":" << cnt << std::endl;
//	std::cout << "// total: " << std::accumulate(cnts.begin(), cnts.end(), 0, [](auto a, const auto &b){return a + b.second;}) << std::endl;
//	std::cout << "// " << parse_time(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)) << std::endl;

	draughts_ d2;
	d2.load_fen(fen);
//	bfs(d2, true); return 0;
	perft(d2, &draughts_::children, depth);
//	t1 = std::chrono::steady_clock::now();
////	bfs(d2,true); return 0;
//	dfs(d2, &draughts_::children, depth);
//	t2 = std::chrono::steady_clock::now();
//	for (auto [lvl, cnt] : cnts)
//		std::cout << "// " << lvl << ":" << cnt << std::endl;
//	std::cout << "// total: " << std::accumulate(cnts.begin(), cnts.end(), 0, [](auto a, const auto &b){return a + b.second;}) << std::endl;
//	std::cout << "// " << parse_time(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)) << std::endl;

//	bfs(d.children()[0].children()[0].children()[0].children()[0].children()[0]);
//	bfs(d.children()[4].children()[4]);
//	std::cout << d.to_string() << std::endl;
//	for (const auto &s : draughts{}.children())
//	{
//		std::cout << s.to_string() << std::endl;
//		for (const auto &t : s.children())
//			std::cout << t.to_string() << std::endl;
//		std::cout << std::string(20, '*') << std::endl;
//	}
	return 0;
}
