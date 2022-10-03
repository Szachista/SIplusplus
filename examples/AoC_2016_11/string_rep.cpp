#include "graph_state.hpp"
#include "graph_search.hpp"
#include <set>
#include <array>
#include <cassert>
#include <iomanip>
#include <unordered_map>
#include <string_view>
using namespace std;

class state : public f_state<uint8_t>
{
public:
	state(int elevator, const std::array<std::unordered_set<std::string_view>, 4> &levels)
		: elevator(elevator), levels(levels)
	{
		assert(elevator >= 0 && elevator <= 3);
		if (hashcodes.empty())
		{
			size_t h = 4;
			for (const auto &ai : levels)
				for (const auto &s : ai)
				{
					hashcodes[s] = h;
					h <<= 1;
				}
		}
	}

	std::unique_ptr<graph_state> clone() const
	{
		return std::unique_ptr<graph_state>(new state(*this));
	}

	std::size_t hash_code() const
	{
		size_t h = elevator;
		for (int i = 0; i < 4; i++)
			for (const auto &s : levels[i])
				h += (hashcodes[s] << (hashcodes.size()*i));
		return h;
	}

	std::vector<std::unique_ptr<graph_state>> get_successors() const
	{
		std::vector<std::unique_ptr<graph_state>> children;

		// transport pojedynczo
		for (const std::string_view &s : levels[elevator])
		{
			if (elevator > 0)
			{
				auto c = clone();
				auto cs = static_cast<state*>(c.get());
				cs->elevator--;
				assert(cs->levels[elevator].erase(s) == 1);
				assert(cs->levels[cs->elevator].insert(s).second);
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
				cs->elevator++;
				assert(cs->levels[elevator].erase(s) == 1);
				assert(cs->levels[cs->elevator].insert(s).second);
				if (cs->is_valid())
				{
					c->set_parent(this);
					c->update_score(get_g() + 1);
					children.emplace_back(std::move(c));
				}
			}
		}

		// transport podwójnie
		for (auto it_i = levels[elevator].begin(); it_i != levels[elevator].end(); ++it_i)
			for (auto it_j = std::next(it_i); it_j != levels[elevator].end(); ++it_j)
			{
//				auto it_i = levels[elevator].cbegin(), it_j = it_i;
//				advance(it_i, i);
//				advance(it_j, j);
				std::string_view item_i = *it_i, item_j = *it_j;
				if (elevator > 0)
				{
					auto c = clone();
					auto cs = static_cast<state*>(c.get());
					cs->elevator--;

					assert(cs->levels[elevator].erase(item_i) == 1);
					assert(cs->levels[cs->elevator].insert(item_i).second);

					assert(cs->levels[elevator].erase(item_j) == 1);
					assert(cs->levels[cs->elevator].insert(item_j).second);

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
					cs->elevator++;

					assert(cs->levels[elevator].erase(item_i) == 1);
					assert(cs->levels[cs->elevator].insert(item_i).second);

					assert(cs->levels[elevator].erase(item_j) == 1);
					assert(cs->levels[cs->elevator].insert(item_j).second);

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

	bool is_solution() const
	{
		return levels[0].empty() && levels[1].empty() && levels[2].empty();
	}

	std::string to_string() const override
	{
		std::ostringstream out;
		for (int i = 3; i >= 0; i--)
		{
			out << i << setw(2) << (elevator == i ? "E" : "");
			for (const auto &s : levels[i])
				out << setw(5) << s;
			out << std::endl;
		}
		return out.str();
	}
protected:
	uint8_t get_heuristic_grade() const
	{
		uint8_t sum = 0;
		for (int i = 0; i < 4; i++)
			sum += levels[i].size();

		return sum - levels[3].size();
	}

	bool is_equal(const graph_state &s) const override
	{
		auto st = dynamic_cast<const state*>(&s);
		if (st == nullptr)
			return false;

		for (size_t i = 0; i < levels.size(); i++)
			if (levels[i] != st->levels[i])
				return false;

		return true;
	}

private:
	bool is_valid() const
	{
		// brak samotnego chipa lub tylko chipy
		for (int i = 0; i < 4; i++)
		{
			bool only_chips = true;
			for (const std::string_view &s : levels[i])
			{
				if (s.back() == 'G')
				{
					only_chips = false;
					break;
				}
			}
			if (only_chips)
				continue;

			for (const std::string_view &s : levels[i])
			{
				if (s.back() == 'M')
				{
					std::string gen(s.data(), s.size());
					gen.back() = 'G';
					if (levels[i].count(gen) == 0)
						return false;
				}
			}
		}
		return true;
	}

	int elevator;
	std::array<std::unordered_set<std::string_view>, 4> levels;
	static std::unordered_map<std::string_view, size_t> hashcodes;
};
std::unordered_map<std::string_view, size_t> state::hashcodes;

template<typename score_type>
bool astar_compare(const graph_state<score_type> &a, const graph_state<score_type> &b)
{
	return a.get_f() < b.get_f();
}

void perform_search(const state &initial)
{
	auto searcher = informative_searcher(initial, astar_compare<uint8_t>);
	std::cout << searcher.get_stats() << std::endl;
	std::cout << (int)searcher.get_solution(0)->get_g() << std::endl;
}

int main()
{
	std::cout << "Part 1:" << std::endl;
	perform_search(state(0, std::array<std::unordered_set<std::string_view>, 4>{std::unordered_set<std::string_view>{"PoG", "TmG", "TmM", "PmG", "RuG", "RuM", "CoG", "CoM"},{"PoM", "PmM"},{},{}}));

	std::cout << std::endl << "Part 2:" << std::endl;
	perform_search(state(0, std::array<std::unordered_set<std::string_view>, 4>{std::unordered_set<std::string_view>{"PoG", "TmG", "TmM", "PmG", "RuG", "RuM", "CoG", "CoM", "UupG", "UupM", "Li2G", "Li2M"},{"PoM", "PmM"},{},{}}));
	return 0;
}
