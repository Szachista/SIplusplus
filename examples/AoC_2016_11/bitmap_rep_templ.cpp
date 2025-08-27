#include "state.hpp"
#include <array>
#include <cassert>
#include <iomanip>
#include <unordered_map>
using namespace std;

enum : std::uint64_t
{
	Co = 1, Pm = 2, Po = 4, Ru = 8, Tm = 16, Uup = 32, Li2 = 64, G = 7
};

class puzzle_state
{
public:
	explicit puzzle_state(std::uint64_t code)
		: code(code)
	{
		std::uint64_t elevator = code >> 14*4;
		assert(elevator <= 3);
	}

	static std::size_t hash_code(const puzzle_state &s)
	{
		return s.code;
	}

	static std::vector<puzzle_state> get_successors(const puzzle_state &s)
	{
		std::vector<puzzle_state> children;

		std::uint64_t elevator = s.code >> 14*4;
		// transport pojedynczo
		for (std::uint64_t i = 1ull << 14*elevator; i <= (0x3fffull << 14*elevator); i <<= 1)
		{
			if ((s.code & i) == 0)
				continue;

			if (elevator > 0)
			{
				children.push_back(s);
				puzzle_state &cs = children.back();
				cs.code &= ~(0x3ull << 14*4);
				cs.code |= (elevator - 1) << 14*4;
				cs.code &= ~i;
				assert((cs.code & (i >> 14)) == 0);
				cs.code |= (i >> 14);
				if (!cs.is_valid())
					children.pop_back();
			}

			if (elevator < 3)
			{
				children.push_back(s);
				puzzle_state &cs = children.back();
				cs.code &= ~(0x3ull << 14*4);
				cs.code |= (elevator + 1) << 14*4;
				cs.code &= ~i;
				assert((cs.code & (i << 14)) == 0);
				cs.code |= (i << 14);
				if (!cs.is_valid())
					children.pop_back();
			}
		}

		// transport podwójnie
		for (std::uint64_t i = 1ull << 14*elevator; i <= (0x3fffull << 14*elevator); i <<= 1)
			for (std::uint64_t j = i << 1; j <= (0x3fffull << 14*elevator); j <<= 1)
			{
				if ((s.code & i) == 0 || (s.code & j) == 0)
					continue;

				if (elevator > 0)
				{
					children.push_back(s);
					puzzle_state &cs = children.back();
					cs.code &= ~(0x3ull << 14*4);
					cs.code |= (elevator - 1) << 14*4;

					cs.code &= ~i;
					assert((cs.code & (i >> 14)) == 0);
					cs.code |= (i >> 14);

					cs.code &= ~j;
					assert((cs.code & (j >> 14)) == 0);
					cs.code |= (j >> 14);

					if (!cs.is_valid())
						children.pop_back();
				}
				if (elevator < 3)
				{
					children.push_back(s);
					puzzle_state &cs = children.back();
					cs.code &= ~(0x3ull << 14*4);
					cs.code |= (elevator + 1) << 14*4;

					cs.code &= ~i;
					assert((cs.code & (i << 14)) == 0);
					cs.code |= (i << 14);

					cs.code &= ~j;
					assert((cs.code & (j << 14)) == 0);
					cs.code |= (j << 14);

					if (!cs.is_valid())
						children.pop_back();
				}
			}

		return children;
	}

	static bool is_solution(const puzzle_state &s) { return (s.code & 0x3ffffffffffull) == 0; }

	std::string to_string() const
	{
		static unordered_map<std::uint64_t, std::string> M = {{0, ""}, {Co, "CoM"}, {Pm, "PmM"}, {Po, "PoM"}, {Ru, "RuM"}, {Tm, "ThM"},
			{Co<<G, "CoG"}, {Pm<<G, "PmG"}, {Po<<G, "PoG"}, {Ru<<G, "RuG"}, {Tm<<G, "ThG"}, {Uup, "UupM"}, {Uup<<G, "UupG"}, {Li2, "Li2M"}, {Li2<<G, "Li2G"}};
		std::ostringstream out;
		int elevator = code >> 14*4;
		for (int i = 3; i >= 0; i--)
		{
			out << i << setw(2) << (elevator == i ? "E" : "");
			for (std::uint64_t j = 1ull << 14*i; j <= (0x3fffull << 14*i); j <<= 1)
				if (code & j)
					out << setw(5) << M[(code & j) >> 14*i];
			out << std::endl;
		}
		return out.str();
	}

	std::uint8_t get_heuristic_grade() const
	{
		uint8_t cnt = 0;
		for (size_t i = 1uLL << (3*14 - 1); i != 0; i >>= 1)
			if (code & i)
				++cnt;
		return cnt;
	}

	bool operator== (const puzzle_state &s) const { return code == s.code; }

private:
	bool is_valid() const
	{
		for (unsigned elevator = 0; elevator <= 3; elevator++)
		{
			uint16_t generators = (code >> (14*elevator + 7)) & 0x7f,
					chips = (code >> 14*elevator) & 0x7f;
			if (generators != 0 && (chips & generators) != chips)
				return false;
		}
		return true;
	}

	std::uint64_t code;
};

template<>
struct std::hash<puzzle_state>
{
	size_t operator()(const puzzle_state &s) const { return puzzle_state::hash_code(s); }
};

template<typename state_t, typename score_t>
class simple_heuristic
{
static_assert(std::is_same_v<state_t, puzzle_state>);
public:
	constexpr score_t operator()(const state_t &s) const
	{
		return s.get_heuristic_grade();
	}
};

auto perform_search(puzzle_state initial)
{
	/*auto searcher = informative_searcher<uint8_t>(initial, [](const graph_state<uint8_t> &a, const graph_state<uint8_t> &b) {
		return a.get_f() < b.get_f();
	});
	std::cout << searcher.get_stats() << std::endl;
	std::cout << (int)searcher.get_solution(0)->get_g() << std::endl;*/
	searcher<puzzle_state, uint8_t, delta_unit, simple_heuristic> srch(std::move(initial), puzzle_state::get_successors, puzzle_state::is_solution);
	std::cout << srch.get_summary() << std::endl;

	auto [solution, score] = srch.get_solution(0);
	std::cout << (int)score << std::endl;
	return srch;
}

int main()
{
	std::uint64_t code = (Po << G) | (Tm << G) | Tm | (Pm << G) | (Ru << G) | Ru | (Co << G) | Co;
	code |= (Po | Pm) << 14;
	std::cout << "Part 1:" << std::endl;
	perform_search(puzzle_state(code));

	std::cout << std::endl << "Part 2:" << std::endl;
	code |= (Uup << G) | Uup | (Li2 << G) | Li2;
	perform_search(puzzle_state(code));
	return 0;
}
