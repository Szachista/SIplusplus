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

	[[nodiscard]] std::size_t hash_code() const { return code; }

	template<bool use_unique_ptr=false, typename result_state=std::conditional_t<use_unique_ptr, std::unique_ptr<puzzle_state>, puzzle_state>>
	[[nodiscard]] std::vector<result_state> get_children() const
	{
		// return get_successors(*this);

		std::vector<result_state> children;

		std::uint64_t elevator = code >> 14*4;
		// transport pojedynczo
		for (std::uint64_t i = 1ull << 14*elevator; i <= (0x3fffull << 14*elevator); i <<= 1)
		{
			if ((code & i) == 0)
				continue;

			if (elevator > 0)
			{
				auto new_code = code;
				new_code &= ~(0x3ull << 14*4);
				new_code |= (elevator - 1) << 14*4;
				new_code &= ~i;
				assert((new_code & (i >> 14)) == 0);
				new_code |= (i >> 14);
				if (is_valid(new_code))
				{
					if constexpr (use_unique_ptr)
						children.emplace_back(std::make_unique<puzzle_state>(new_code));
					else
						children.emplace_back(new_code);
				}
			}

			if (elevator < 3)
			{
				auto new_code = code;
				new_code &= ~(0x3ull << 14*4);
				new_code |= (elevator + 1) << 14*4;
				new_code &= ~i;
				assert((new_code & (i << 14)) == 0);
				new_code |= (i << 14);
				if (is_valid(new_code))
				{
					if constexpr (use_unique_ptr)
						children.emplace_back(std::make_unique<puzzle_state>(new_code));
					else
						children.emplace_back(new_code);
				}
			}
		}

		// transport podwójnie
		for (std::uint64_t i = 1ull << 14*elevator; i <= (0x3fffull << 14*elevator); i <<= 1)
			for (std::uint64_t j = i << 1; j <= (0x3fffull << 14*elevator); j <<= 1)
			{
				if ((code & i) == 0 || (code & j) == 0)
					continue;

				if (elevator > 0)
				{
					auto new_code = code;
					new_code &= ~(0x3ull << 14*4);
					new_code |= (elevator - 1) << 14*4;

					new_code &= ~i;
					assert((new_code & (i >> 14)) == 0);
					new_code |= (i >> 14);

					new_code &= ~j;
					assert((new_code & (j >> 14)) == 0);
					new_code |= (j >> 14);

					if (is_valid(new_code))
					{
						if constexpr (use_unique_ptr)
							children.emplace_back(std::make_unique<puzzle_state>(new_code));
						else
							children.emplace_back(new_code);
					}
				}
				if (elevator < 3)
				{
					auto new_code = code;
					new_code &= ~(0x3ull << 14*4);
					new_code |= (elevator + 1) << 14*4;

					new_code &= ~i;
					assert((new_code & (i << 14)) == 0);
					new_code |= (i << 14);

					new_code &= ~j;
					assert((new_code & (j << 14)) == 0);
					new_code |= (j << 14);

					if (is_valid(new_code))
					{
						if constexpr (use_unique_ptr)
							children.emplace_back(std::make_unique<puzzle_state>(new_code));
						else
							children.emplace_back(new_code);
					}
				}
			}

		return children;
	}

	template<bool use_unique_ptr=false, typename result_state=std::conditional_t<use_unique_ptr, std::unique_ptr<puzzle_state>, puzzle_state>>
	static std::vector<result_state> get_successors(const result_state &s)
	{
		return std::invoke(&puzzle_state::get_children<use_unique_ptr>, s);
	}

	/*static std::vector<puzzle_state> get_successors(const puzzle_state &s)
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
	}*/

	// static bool is_solution(const puzzle_state &s) { return (s.code & 0x3ffffffffffull) == 0; }

	bool is_solution() const { return (code & 0x3ffffffffffull) == 0; }

	std::string to_string() const
	{
		static unordered_map<std::uint64_t, std::string> M = {{0, ""}, {Co, "CoM"}, {Pm, "PmM"}, {Po, "PoM"}, {Ru, "RuM"}, {Tm, "ThM"},
			{Co<<G, "CoG"}, {Pm<<G, "PmG"}, {Po<<G, "PoG"}, {Ru<<G, "RuG"}, {Tm<<G, "ThG"}, {Uup, "UupM"}, {Uup<<G, "UupG"}, {Li2, "Li2M"}, {Li2<<G, "Li2G"}};
		std::ostringstream out;
		int elevator = code >> 14*4;
		for (int i = 3; i >= 0; i--)
		{
			out << i << setw(2) << (elevator == i ? "E" : "");
			for (auto j = 1ull << 14*i; j <= (0x3fffull << 14*i); j <<= 1)
				if (code & j)
					out << setw(5) << M[(code & j) >> 14*i];
			out << std::endl;
		}
		return out.str();
	}

	std::uint8_t get_heuristic_grade() const
	{
		uint8_t cnt = 0;
		for (auto i = 1uLL << (3*14 - 1); i != 0; i >>= 1)
			if (code & i)
				++cnt;
		return cnt;
	}

	template<bool use_unique_ptr=false, typename state_t=std::conditional_t<use_unique_ptr, std::unique_ptr<puzzle_state>, puzzle_state>>
	uint8_t delta(const state_t&) const { return 1; }

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

	static bool is_valid(const std::uint64_t code)
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
	size_t operator()(const puzzle_state &s) const { return s.hash_code(); }
};

template<typename score_t>
class simple_heuristic
{
public:
	template<typename state_t>
	constexpr score_t operator()(const state_t &s) const
	{
		return std::invoke(&puzzle_state::get_heuristic_grade, s);
		// return s.get_heuristic_grade();
	}
};

template<>
struct solution_predicate<puzzle_state>
{
	bool operator()(const puzzle_state &s) const { return s.is_solution(); }
};

template<>
struct solution_predicate<std::unique_ptr<puzzle_state>>
{
	bool operator()(const std::unique_ptr<puzzle_state> &s) const { return s->is_solution(); }
};

template<bool test_unique_ptr>
void perform_search(const puzzle_state initial)
{
	using state_t = std::conditional_t<test_unique_ptr, std::unique_ptr<puzzle_state>, puzzle_state>;
	auto get_state = [&initial]() {
		if constexpr (test_unique_ptr)
			return std::make_unique<puzzle_state>(initial);
		else
			return initial;
	};

	{
		std::cout << "\tgraph_searcher(delta_unit, simple_heuristic):\n";
		graph_searcher<state_t, uint8_t, delta_unit, simple_heuristic> srch(get_state(), puzzle_state::get_successors<test_unique_ptr>, true);
		std::cout << srch.get_summary();

		auto [solution, score] = srch.get_solution(0);
		std::cout << (int)score << std::endl;
	}

	{
		std::cout << "\tinformative_searcher(g, h):\n";
		auto res = informative_searcher(get_state(), &puzzle_state::get_children<test_unique_ptr>, &puzzle_state::is_solution, &puzzle_state::delta<test_unique_ptr>, &puzzle_state::get_heuristic_grade, true);
		std::cout << res.get_summary();

		auto [solution, score] = res.get_solution(0);
		std::cout << (int)score << std::endl;
	}

	{
		std::cout << "\tinformative_searcher(h):\n";
		auto res = informative_searcher(get_state(), &puzzle_state::get_children<test_unique_ptr>, &puzzle_state::is_solution, nullptr, &puzzle_state::get_heuristic_grade, true);
		std::cout << res.get_summary();

		std::cout << res.get_path(0).size() - 1 << std::endl;
	}

	{
		std::cout << "\tinformative_searcher(g):\n";
		auto res = informative_searcher(get_state(), &puzzle_state::get_children<test_unique_ptr>, &puzzle_state::is_solution, &puzzle_state::delta<test_unique_ptr>, nullptr, true);
		std::cout << res.get_summary();

		auto [solution, score] = res.get_solution(0);
		std::cout << (int)score << std::endl;
	}

	{
		std::cout << "\tinformative_searcher():\n";
		auto res = informative_searcher(get_state(), &puzzle_state::get_children<test_unique_ptr>, &puzzle_state::is_solution, nullptr, nullptr, true);
		std::cout << res.get_summary();

		std::cout << res.get_path(0).size() - 1 << std::endl;
	}
}

int main()
{
	std::uint64_t code = (Po << G) | (Tm << G) | Tm | (Pm << G) | (Ru << G) | Ru | (Co << G) | Co;
	code |= (Po | Pm) << 14;
	std::cout << "Part 1:" << std::endl;
	std::cout << "*** objects:\n";
	perform_search<false>(puzzle_state(code));
	std::cout << "*** unique_ptr:\n";
	perform_search<true>(puzzle_state(code));

	std::cout << std::string(40, '*');
	std::cout << std::endl << "Part 2:" << std::endl;
	code |= (Uup << G) | Uup | (Li2 << G) | Li2;
	std::cout << "*** objects:\n";
	perform_search<false>(puzzle_state(code));
	std::cout << "*** unique_ptr:\n";
	perform_search<true>(puzzle_state(code));
	return 0;
}
