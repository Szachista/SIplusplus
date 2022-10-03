#include "graph_state.hpp"
#include "graph_search.hpp"
#include <array>
#include <cassert>
#include <iomanip>
#include <unordered_map>
using namespace std;

enum : std::uint64_t
{
	Co = 1, Pm = 2, Po = 4, Ru = 8, Tm = 16, Uup = 32, Li2 = 64, G = 7
};

class state : public f_state<uint8_t>
{
public:
	state(std::uint64_t code)
		: code(code)
	{
		std::uint64_t elevator = code >> 14*4;
		assert(elevator <= 3);
	}

	std::unique_ptr<graph_state> clone() const
	{
		return std::unique_ptr<graph_state>(new state(*this));
	}

	std::size_t hash_code() const
	{
		return code;
	}

	std::vector<std::unique_ptr<graph_state>> get_successors() const
	{
		std::vector<std::unique_ptr<graph_state>> children;

		std::uint64_t elevator = code >> 14*4;
		// transport pojedynczo
		for (std::uint64_t i = 1ull << 14*elevator; i <= (0x3fffull << 14*elevator); i <<= 1)
		{
			if ((code & i) == 0)
				continue;

			if (elevator > 0)
			{
				auto c = clone();
				auto cs = static_cast<state*>(c.get());
				cs->code &= ~(0x3ull << 14*4);
				cs->code |= (elevator - 1) << 14*4;
				cs->code &= ~i;
				assert((cs->code & (i >> 14)) == 0);
				cs->code |= (i >> 14);
				if (cs->is_valid())
				{
					c->set_parent(this);
					c->update_score(get_g() + 1);
					children.emplace_back(std::move(c));
				}
			}

			if (elevator < 3)
			{
				auto c = clone();
				auto cs = static_cast<state*>(c.get());
				cs->code &= ~(0x3ull << 14*4);
				cs->code |= (elevator + 1) << 14*4;
				cs->code &= ~i;
				assert((cs->code & (i << 14)) == 0);
				cs->code |= (i << 14);
				if (cs->is_valid())
				{
					c->set_parent(this);
					c->update_score(get_g() + 1);
					children.emplace_back(std::move(c));
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
					auto c = clone();
					auto cs = static_cast<state*>(c.get());
					cs->code &= ~(0x3ull << 14*4);
					cs->code |= (elevator - 1) << 14*4;

					cs->code &= ~i;
					assert((cs->code & (i >> 14)) == 0);
					cs->code |= (i >> 14);

					cs->code &= ~j;
					assert((cs->code & (j >> 14)) == 0);
					cs->code |= (j >> 14);

					if (cs->is_valid())
					{
						c->set_parent(this);
						c->update_score(get_g() + 1);
						children.emplace_back(std::move(c));
					}
				}
				if (elevator < 3)
				{
					auto c = clone();
					auto cs = static_cast<state*>(c.get());
					cs->code &= ~(0x3ull << 14*4);
					cs->code |= (elevator + 1) << 14*4;

					cs->code &= ~i;
					assert((cs->code & (i << 14)) == 0);
					cs->code |= (i << 14);

					cs->code &= ~j;
					assert((cs->code & (j <<14)) == 0);
					cs->code |= (j << 14);

					if (cs->is_valid())
					{
						c->set_parent(this);
						c->update_score(get_g() + 1);
						children.emplace_back(std::move(c));
					}
				}
			}

		return children;
	}

	bool is_solution() const { return (code & 0x3fffffffffful) == 0; }

	std::string to_string() const override
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

protected:
	std::uint8_t get_heuristic_grade() const
	{
		uint8_t cnt = 0;
		for (size_t i = 1uL << (3*14 - 1); i != 0; i >>= 1)
			if (code & i)
				++cnt;
		return cnt;
	}

	bool is_equal(const graph_state &s) const override
	{
		auto st = dynamic_cast<const state*>(&s);
		return st != nullptr && code == st->code;
	}

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

void perform_search(const state &initial)
{
	auto searcher = informative_searcher<uint8_t>(initial, [](const graph_state<uint8_t> &a, const graph_state<uint8_t> &b) {
		return a.get_f() < b.get_f();
	});
	std::cout << searcher.get_stats() << std::endl;
	std::cout << (int)searcher.get_solution(0)->get_g() << std::endl;
}

int main()
{
	std::uint64_t code = (Po << G) | (Tm << G) | Tm | (Pm << G) | (Ru << G) | Ru | (Co << G) | Co;
	code |= (Po | Pm) << 14;
	std::cout << "Part 1:" << std::endl;
	perform_search(state(code));

	std::cout << std::endl << "Part 2:" << std::endl;
	code |= (Uup << G) | Uup | (Li2 << G) | Li2;
	perform_search(state(code));
	return 0;
}
