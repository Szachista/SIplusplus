#include "state.hpp"
#include <forward_list>
#include <array>
#include <iostream>
#include <iomanip>
#include <queue>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>

/*static std::string parse_time(int64_t t)
{
	std::ostringstream str;
	str.fill('0');

	const int ms = abs(t % 1000), s = abs(t/1000) % 60,
			m = abs(t/60000) % 60, h = t / 3600000;

	str << std::setw(2) << h << ':' << std::setw(2) << m << ':'
		<< std::setw(2) << s << '.' << std::setw(3) << ms;

	return str.str();
}*/

/*static std::string parse_time(std::chrono::seconds t)
{
	std::ostringstream str;
	str.fill('0');

	auto h = t.count() / 3600, m = (t.count() / 60) % 60, s = t.count() % 60;

	str << std::setw(2) << h << ":" << std::setw(2) << m << ":" << std::setw(2) << s;

	return str.str();
}*/

class rubik2x2
{
public:
	rubik2x2()
		: blocks {'L','L','L','L', 'F','F','F','F', 'R','R','R','R', 'D','D','D','D', 'B','B','B','B', 'U','U','U','U'}
	{
	}

	template<typename URBG>
	static rubik2x2 scramble(int repts, URBG &&g)
	{
		rubik2x2 s;
		for (int i = 0; i < repts; i++)
		{
			auto lst = children(s);
			auto it = lst.begin();
			size_t sz = std::distance(lst.begin(), lst.end());
			std::uniform_int_distribution<int> distr(0, sz - 1);
			std::advance(it, distr(g));
			s = *it;
		}
		s.move = '\0';
		return s;
	}

	static bool is_solution(const rubik2x2 &s)
	{
		for (unsigned i = 0; i < s.blocks.size(); i += 4)
			for (unsigned j = 1; j < 4; j++)
				if (s.blocks[i + j] != s.blocks[i])
					return false;
		return true;
	}

	static bool is_solution_ptr(const std::unique_ptr<rubik2x2> &s)
	{
		return is_solution(*s);
	}

	static std::forward_list<rubik2x2> children(const rubik2x2 &r)
	{
		static const std::array<std::unordered_map<int, int>, 6> moves = {
			std::unordered_map<int, int>{	// F
				{ 1, 23}, { 3, 22}, {22,  8}, {23, 10}, { 8, 13}, {10, 12}, {13,  3}, {12,  1},
				{ 4,  5}, { 5,  7}, { 7,  6}, { 6,  4}
			}, {	// B
				{ 0, 14}, { 2, 15}, {14, 11}, {15,  9}, {11, 21}, { 9, 20}, {20,  2}, {21,  0},
				{16, 17}, {17, 19}, {19, 18}, {18, 16}
			}, {	// L
				{20,  4}, {22,  6}, { 4, 12}, { 6, 14}, {12, 19}, {14, 17}, {19, 20}, {17, 22},
				{ 0,  1}, { 1,  3}, { 3,  2}, { 2,  0}
			}, {	// R
				{ 5, 21}, { 7, 23}, {21, 18}, {23, 16}, {18, 13}, {16, 15}, {13,  5}, {15,  7},
				{ 8,  9}, { 9, 11}, {11, 10}, {10,  8}
			}, {	// U
				{ 0, 16}, { 1, 17}, {16,  8}, {17,  9}, { 8,  4}, { 9,  5}, { 4,  0}, { 5,  1},
				{20, 21}, {21, 23}, {23, 22}, {22, 20}
			}, {	// D
				{ 2,  6}, { 3,  7}, { 6, 10}, { 7, 11}, {10, 18}, {11, 19}, {18,  2}, {19,  3},
				{12, 13}, {13, 15}, {15, 14}, {14, 12}
			}
		};
		const char* mvs = "FBLRUD";

		std::forward_list<rubik2x2> lst;

		for (const auto& m : moves)
		{
			rubik2x2 clockwise = r, counterclockwise = r;
			for (auto [from, to] : m)
			{
				clockwise.blocks[to] = r.blocks[from];
				counterclockwise.blocks[from] = r.blocks[to];
			}

			clockwise.move = *mvs;
			counterclockwise.move = tolower(*mvs);
			mvs++;

			lst.push_front(std::move(clockwise));
			lst.push_front(std::move(counterclockwise));
		}

		return lst;
	}

	static std::forward_list<std::unique_ptr<rubik2x2>> children_ptr(const std::unique_ptr<rubik2x2>& r)
	{
		auto lst = children(*r);
		std::forward_list<std::unique_ptr<rubik2x2>> lst_ptr;
		std::transform(lst.begin(), lst.end(), std::front_inserter(lst_ptr), [](const rubik2x2& s) {return std::make_unique<rubik2x2>(s); });
		return lst_ptr;
	}

	static size_t hashcode(const rubik2x2& r)
	{
		static const std::hash<std::string_view> h {};

		return h(std::string_view(r.blocks.begin(), r.blocks.end()));
	}

	std::string to_string() const { return std::string(blocks.begin(), blocks.end()) + " " + move; }

	bool operator== (const rubik2x2& s) const
	{
		return blocks == s.blocks;
		return std::equal(blocks.begin(), blocks.end(), s.blocks.begin());
	}

private:
	std::array<char, 24> blocks;
	char move = '*';
};

template<>
struct std::hash<rubik2x2>
{
	size_t operator()(const rubik2x2& s) const { return rubik2x2::hashcode(s); }
};

//template<>
//struct std::hash<std::unique_ptr<rubik2x2>>
//{
//	size_t operator()(const std::unique_ptr<rubik2x2>& s) const { return rubik2x2::hashcode(*s); }
//};

//static bool operator==(const std::unique_ptr<rubik2x2>& s, const std::unique_ptr<rubik2x2>& t)
//{
//	return *s == *t;
//}

template<typename state_t, typename generator>
size_t count_states_it(state_t&& initial, generator gen)
{
	std::cout << " E(#open + #closed) = 88179840" << std::endl;
	auto t1 = std::chrono::steady_clock::now();
	size_t closed_size;

	{
		std::unordered_set<state_t> visited, open;
		std::stack<typename std::unordered_set<state_t>::const_iterator> q;

		open.insert(std::move(initial));
		q.push(open.begin());

		while (!q.empty())
		{
			auto s = std::move(open.extract(q.top()).value());
			q.pop();

			for (auto&& t : gen(s))
				if (!visited.contains(t) && !open.contains(t))
					q.push(open.insert(std::move(t)).first);

			visited.insert(std::move(s));
			if (visited.size() % 1000 == 0)
				(std::cout << std::setw(8) << q.size() << " + " << std::setw(8) << visited.size() << " = " << std::setw(8) << q.size() + visited.size() << "          \r").flush();
		}
		(std::cout << std::setw(8) << q.size() << " + " << std::setw(8) << visited.size() << " = " << std::setw(8) << q.size() + visited.size() << "          \n");

		auto t2 = std::chrono::steady_clock::now();
		std::cout << "Finished after " << parse_time(std::chrono::duration_cast<std::chrono::seconds>(t2 - t1)) << std::endl;
		closed_size = visited.size();
	}

	auto t2 = std::chrono::steady_clock::now();
	std::cout << "Elapsed time: " << parse_time(std::chrono::duration_cast<std::chrono::seconds>(t2 - t1)) << std::endl;

	return closed_size;
}

template<typename state_t, typename generator>
size_t count_states(const state_t& initial, generator gen)
{
	std::cout << " E(#open + #closed) = 88179840" << std::endl;
	auto t1 = std::chrono::steady_clock::now();
	size_t closed_size;

	{
		std::unordered_set<std::unique_ptr<state_t>> visited, open;
		std::stack<state_t*> q;

		open.insert(std::make_unique<state_t>(initial));
		q.push(open.begin()->get());

		while (!q.empty())
		{
			std::unique_ptr<state_t> fake{ q.top() };
			std::unique_ptr<state_t> s = std::move(open.extract(fake).value());
			fake.release();

			q.pop();

			for (auto&& t : gen(s))
				if (!visited.contains(t) && !open.contains(t))
				{
					q.push(t.get());
					open.insert(std::move(t));
				}

			visited.insert(std::move(s));
			if (visited.size() % 1000 == 0)
				(std::cout << std::setw(8) << q.size() << " + " << std::setw(8) << visited.size() << " = " << std::setw(8) << q.size() + visited.size() << "          \r").flush();
		}
		(std::cout << std::setw(8) << q.size() << " + " << std::setw(8) << visited.size() << " = " << std::setw(8) << q.size() + visited.size() << "          \n");

		auto t2 = std::chrono::steady_clock::now();
		std::cout << "Finished after " << parse_time(std::chrono::duration_cast<std::chrono::seconds>(t2 - t1)) << std::endl;
		closed_size = visited.size();
	}

	auto t2 = std::chrono::steady_clock::now();
	std::cout << "Elapsed time: " << parse_time(std::chrono::duration_cast<std::chrono::seconds>(t2 - t1)) << std::endl;

	return closed_size;
}

int main()
{
	{
		searcher<rubik2x2, int, delta_zero, no_heuristic> srch(rubik2x2{}, rubik2x2::children, [](const auto&){return false;});
		std::cout << srch.get_summary() << std::endl << std::endl;
	}
	{
		searcher<rubik2x2, int, delta_unit, no_heuristic> srch(rubik2x2{}, rubik2x2::children, [](const rubik2x2&){return false;});
		std::cout << srch.get_summary() << std::endl;
		return 0;
	}
	//rubik2x2 r;

	//for (const auto &t : decltype(r)::children(r))
	//{
	//	std::cout << (bool)t << ' ' << t.to_string() << std::endl;
	//	for (const auto &s : decltype(r)::children(t))
	//		std::cout << '\t' << (bool)s << ' ' << s.to_string() << std::endl;
	//}
	//std::cout << "Ordinary (iterator based):\n";
	//auto states_cnt = count_states_it(rubik2x2{}, rubik2x2::children);
	//std::cout << "\nNumber of states: " << states_cnt << std::endl;

	//std::cout << "unique_ptr (iterator based):\n";
	//states_cnt = count_states_it(std::move(std::make_unique<rubik2x2>()), rubik2x2::children_ptr);
	//std::cout << "\nNumber of states: " << states_cnt << std::endl;

	//std::cout << "Original:\n";
	//states_cnt = count_states(rubik2x2{}, rubik2x2::children_ptr);
	//std::cout << "\nNumber of states: " << states_cnt << std::endl;

	std::default_random_engine g;
	auto init = rubik2x2::scramble(15, g);
	{
		searcher<std::unique_ptr<rubik2x2>, int, delta_unit, no_heuristic> srch(std::make_unique<rubik2x2>(init), rubik2x2::children_ptr, rubik2x2::is_solution_ptr);
		std::cout << srch.get_summary() << std::endl;

		for (const auto it : srch.get_path(0))
			std::cout << (*it)->to_string() << std::endl;
		std::cout << std::endl;
	}
	{
		searcher<rubik2x2, int, delta_unit, no_heuristic> srch(std::move(init), rubik2x2::children, rubik2x2::is_solution);
		std::cout << srch.get_summary() << std::endl;

		for (const auto it : srch.get_path(0))
			std::cout << it->to_string() << std::endl;
	}

	return 0;
}
