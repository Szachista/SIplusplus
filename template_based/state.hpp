#ifndef STATE_HPP
#define STATE_HPP

//#include <bits/stdc++.h>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <optional>
#include <functional>
//#include <future>
using namespace std::literals;

//export module state;
//import std;

template <typename...> struct is_unique_ptr final : std::false_type {};
template<class T, typename... Args>
struct is_unique_ptr<std::unique_ptr<T, Args...>> final : std::true_type {};

template <typename...> struct is_pair final : std::false_type {};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> final : std::true_type {};

static std::string parse_time(std::chrono::milliseconds t)
{
	std::ostringstream str;
	str.fill('0');

	const int64_t ms = abs(t.count() % 1000), s = abs(t.count()/1000) % 60,
			m = abs(t.count()/60000) % 60, h = t.count() / 3600000;

	str << std::setw(2) << h << ':' << std::setw(2) << m << ':'
		<< std::setw(2) << s << '.' << std::setw(3) << ms;

	return str.str();
}

//export
template<typename score_t>
class delta_zero
{
public:
	constexpr score_t operator()(const auto&, const auto&) const
	{
		return 0;
	}
};

//export
template<typename score_t>
class delta_unit
{
public:
	constexpr score_t operator()(const auto&, const auto&) const
	{
		return 1;
	}
};

//export
template<typename score_t>
class no_heuristic
{
public:
	constexpr score_t operator()(const auto&) const
	{
		return 0;
	}
};

template<typename state_t>
struct hash_iterator
{
	size_t operator()(typename std::unordered_set<state_t>::const_iterator it) const
	{
		return h(*it);
	}
	static constexpr std::hash<state_t> h {};
};

template<typename state_t>
struct equal_to_iterator
{
	bool operator()(typename std::unordered_set<state_t>::const_iterator a, typename std::unordered_set<state_t>::const_iterator b) const
	{
		return *a == *b;
	}
};

template<typename state_t>
struct std::equal_to<std::unique_ptr<state_t>>
{
	bool operator()(const std::unique_ptr<state_t> &a, const std::unique_ptr<state_t> &b) const
	{
		return *a == *b;
	}
};

template<typename state_t, typename score_t, typename parent_t>
class priority_queue
{
public:
	bool empty() const { return q.empty(); }

	size_t size() const { return q.size(); }

	bool push(state_t&& t, score_t score, parent_t parent)
	{
		if (auto it = obj2prop.find(t); it != obj2prop.end())
		{
			if (score < it->second.score)
			{
				auto old_prop = it->second;
				it->second.score = score;
				it->second.parent = parent;
				up(old_prop.idx);
				down(old_prop.idx);
//				std::cout << "push: "; print();
				return true;
			}
		}
		else
		{
			auto [iter, inserted] = obj2prop.emplace(std::move(t), prop{score, parent, q.size()});
			q.push_back(iter);
			up(q.size() - 1);
//			std::cout << "push: "; print();
			return true;
		}
		return false;
	}

	std::tuple<state_t, score_t, parent_t> pop()
	{
		//if (q.empty()) throw std::domain_error("Empty queue!");

		score_t score = q[0]->second.score;
		parent_t parent = q[0]->second.parent;
		state_t state = std::move(obj2prop.extract(q[0]).key());

		if (q.size() > 1)
			std::swap(q.front(), q.back());

		q.pop_back();

		if (!q.empty())
		{
			q[0]->second.idx = 0;
			down(0);
		}
//		std::cout << "pop : "; print();

		return { std::move(state), score, parent };
	}

//	void print() const
//	{
//		for (const auto &[s, p] : obj2prop)
//			if constexpr (ptr_syntax<state_t>)
//				std::cout << "[" << s->to_string() << ", " << p.idx << "]; ";
//			else
//				std::cout << "[" << s.to_string() << ", " << p.idx << "]; ";
//		std::cout << std::endl << std::endl;
//	}

private:
	struct prop
	{
		score_t score;
		parent_t parent;
		size_t idx;
	};

	void down(size_t idx)
	{
		size_t smallest = idx;

		while (true)
		{
			if (2 * idx + 1 < q.size() && q[2 * idx + 1]->second.score < q[smallest]->second.score)
				smallest = 2 * idx + 1;

			if (2 * idx + 2 < q.size() && q[2 * idx + 2]->second.score < q[smallest]->second.score)
				smallest = 2 * idx + 2;

			if (smallest == idx)
				break;

			std::swap(q[idx]->second.idx, q[smallest]->second.idx);
			std::swap(q[idx], q[smallest]);
			idx = smallest;
		}

		validate();
	}

	void up(size_t idx)
	{
		while (idx != 0)
		{
			size_t p = (idx - 1) / 2;

			if (q[p]->second.score <= q[idx]->second.score)
				break;

			std::swap(q[p]->second.idx, q[idx]->second.idx);
			std::swap(q[p], q[idx]);
			idx = p;
		}

		validate();
	}

	void validate() const
	{
		return;
		if (q.size() != obj2prop.size())
		{
			std::cerr << "q.size() is different than obj2prop.size()!" << std::endl;
			abort();
		}
		for (size_t i = 0; i < q.size(); i++)
		{
			if (q[i]->second.idx != i)
			{
				std::cerr << "wrong idx: should be (" << i << "), but (" << q[i]->second.idx << ") is given!" << std::endl;
				abort();
			}
			if (2*i + 1 < q.size() && q[i]->second.score > q[2*i+1]->second.score)
			{
				std::cerr << "left child is smaller!" << std::endl;
				abort();
			}
			if (2*i + 2 < q.size() && q[i]->second.score > q[2*i+2]->second.score)
			{
				std::cerr << "right child is smaller!" << std::endl;
				abort();
			}
		}
	}

	std::unordered_map<state_t, prop> obj2prop;
	std::vector<typename std::unordered_map<state_t, prop>::iterator> q;
};

//export
template<typename state_t>
struct solution_predicate
{
	constexpr bool operator()(const state_t &) const { return false; }
};

//export
template<typename state_t, typename Open, typename Solutions>
class search_result
{
public:
	search_result(std::unordered_set<state_t> &&closed,
				  std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> &&parent_map,
				  Open &&open,
				  Solutions &&solutions,
				  std::chrono::milliseconds elapsed_time) :
		closed(std::move(closed)),
		parent_map(std::move(parent_map)),
		open(std::move(open)),
		solutions(std::move(solutions)),
		elapsed_time(elapsed_time)
	{
	}

	std::string get_summary() const
	{
		std::ostringstream str;

		str << "elapsed time: " << parse_time(elapsed_time) << std::endl;
		str << "#visited    : " << closed.size() << std::endl;
		str << "#open       : " << open.size() << std::endl;
		str << "nodes/second: " << (closed.size() + open.size())*1000/std::max<size_t>(elapsed_time.count(),1) << std::endl;

		return str.str();
	}

	size_t get_number_of_solutions() const { return solutions.size(); }

	auto get_solution(size_t idx) const
	{
		return solutions.at(idx);
	}

	auto get_path(size_t idx) const
	{
		std::vector<typename std::unordered_set<state_t>::const_iterator> path;

		if (idx >= solutions.size())
			return path;

		if constexpr (is_pair<typename Solutions::value_type>::value)
			for (auto it = solutions[idx].first; it != closed.end(); it = parent_map.at(it))
				path.push_back(it);
		else
			for (auto it = solutions[idx]; it != closed.end(); it = parent_map.at(it))
				path.push_back(it);

		return path;
	}

private:
	std::unordered_set<state_t> closed;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	Open open;
	Solutions solutions;
	std::chrono::milliseconds elapsed_time;
};

//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0,
						  std::vector<state_t> (state_t::*successors)() const,
						  score_t (state_t::*delta)(const state_t&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	solution_predicate<state_t> is_solution{};

	std::unordered_set<state_t> closed;
	std::conditional_t<std::is_same_v<score_t, void>, std::unordered_map<state_t, typename std::unordered_set<state_t>::const_iterator>, priority_queue<state_t, score_t, typename std::unordered_set<state_t>::const_iterator>> open;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	std::conditional_t<std::is_same_v<score_t, void>,
			std::vector<typename std::unordered_set<state_t>::const_iterator>,
			std::vector<std::pair<typename std::unordered_set<state_t>::const_iterator, score_t>>> solutions;
	bool running;
	std::chrono::milliseconds elapsed_time;
	std::chrono::steady_clock::time_point start_time, stop_time;

	std::ostream no_output {nullptr};
	running = true;

	auto progress = [&running, &start_time, &closed, &open](std::ostream &out)
	{
		while (running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)) << " #closed = " << closed.size() << ", #open = " << open.size() << "                \r";
			out.flush();
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}
		(out << std::string(50, ' ') << '\r').flush();
	};

	start_time = std::chrono::steady_clock::now();
	auto thr = std::thread(progress, std::ref(show_progress ? std::cout : no_output));

	if constexpr (std::is_same_v<score_t, void>)
	{
		// Breadth First Search
		open.emplace(std::move(s0), closed.end());
		while (!open.empty())
		{
			auto old_open = std::move(open);
			for (auto &[s, parent] : old_open)
			{
				auto s_closed = closed.insert(std::move(s)).first;
				parent_map[s_closed] = parent;

				if (is_solution(*s_closed))
				{
					solutions.push_back(s_closed);
					break;
				}

				auto children = std::move((*s_closed.*successors)());
				for (state_t &t : children)
					if (!closed.contains(t))
						open.emplace(std::move(t), s_closed);
			}
		}
	}
	else
	{
		open.push(std::move(s0), (heuristic != nullptr ? (s0.*heuristic)() : 0), closed.end());
		while (!open.empty())
		{
			auto [s, score, parent] = open.pop();
			auto s_closed = closed.insert(std::move(s)).first;
			parent_map[s_closed] = parent;

			if (is_solution(*s_closed))
			{
				solutions.push_back(std::make_pair(s_closed, score));
				break;
			}

			if (heuristic != nullptr)
				score -= (*s_closed.*heuristic)(); // zostaje samo g(s)

			auto children = std::move((*s_closed.*successors)());

			for (state_t& t : children)
			{
				if (closed.contains(t))
					continue;

				score_t h = (heuristic != nullptr ? (t.*heuristic)() : 0);
				score_t d = (delta != nullptr ? (*s_closed.*delta)(t) : 0);
				open.push(std::move(t), score + d + h, s_closed);
			}
		}
	}

	stop_time = std::chrono::steady_clock::now();
	elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	running = false;
	thr.join();

	return search_result(std::move(closed), std::move(parent_map), std::move(open), std::move(solutions), elapsed_time);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0,
						  std::vector<state_t> (state_t::*successors)() const,
						  score_t (state_t::*delta)(const state_t&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	return informative_searcher(state_t{s0}, successors, delta, heuristic, show_progress);
}

//export
template<typename state_t>
auto informative_searcher(state_t &&s0, std::vector<state_t> (state_t::*children)() const, bool show_progress=false)
{
	return informative_searcher<state_t, void>(std::move(s0), children, nullptr, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0, std::vector<state_t> (state_t::*children)() const, score_t (state_t::*delta)(const state_t&) const, bool show_progress=false)
{
	return informative_searcher<state_t, score_t>(std::move(s0), children, delta, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0, std::vector<state_t> (state_t::*children)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return informative_searcher<state_t, score_t>(std::move(s0), children, nullptr, heuristic, show_progress);
}

//export
template<typename state_t>
auto informative_searcher(const state_t &s0, std::vector<state_t> (state_t::*children)() const, bool show_progress=false)
{
	return informative_searcher<state_t, void>(state_t{s0}, children, nullptr, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0, std::vector<state_t> (state_t::*children)() const, score_t (state_t::*delta)(const state_t&) const, bool show_progress=false)
{
	return informative_searcher<state_t, score_t>(state_t{s0}, children, delta, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0, std::vector<state_t> (state_t::*children)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return informative_searcher<state_t, score_t>(state_t{s0}, children, nullptr, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t, template<typename> class Delta, template<typename> class Heuristic>
class graph_searcher
{
	static constexpr Delta<score_t> delta {};
	static constexpr Heuristic<score_t> heuristic {};
	static constexpr solution_predicate<state_t> is_solution {};

public:
	template<typename Successors>
	graph_searcher(const state_t& s0, Successors generator, bool show_progress=true) : graph_searcher(state_t{s0}, generator, show_progress)
	{
	}

	template<typename Successors>
	graph_searcher(state_t&& s0, Successors generator, bool show_progress=false)
	{
		std::ostream no_output {nullptr};
		running = true;

		start_time = std::chrono::steady_clock::now();
		auto thr = std::thread(graph_searcher::progress, std::ref(*this), std::ref(show_progress ? std::cout : no_output));

		open.push(std::move(s0), heuristic(s0), closed.end());
		while (!open.empty())
		{
			auto [s, score, parent] = open.pop();
			auto s_closed = closed.insert(std::move(s)).first;
			parent_map[s_closed] = parent;

			if (is_solution(*s_closed))
			{
				solutions.push_back(std::make_pair(s_closed, score));
				break;
			}

			score -= heuristic(*s_closed); // zostaje samo g(s)

			for (state_t& t : generator(*s_closed))
			{
				if (closed.contains(t))
					continue;

				open.push(std::move(t), score + delta(*s_closed, t) + heuristic(t), s_closed);
			}
		}

		stop_time = std::chrono::steady_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

		running = false;
		thr.join();
	}

	std::string get_summary() const
	{
		std::ostringstream str;

		str << "elapsed time: " << parse_time(elapsed_time) << std::endl;
		str << "#visited    : " << closed.size() << std::endl;
		str << "#open       : " << open.size() << std::endl;
		str << "nodes/second: " << (closed.size() + open.size())*1000/std::max<size_t>(elapsed_time.count(),1) << std::endl;

		return str.str();
	}

	size_t get_number_of_solutions() const { return solutions.size(); }

	std::pair<typename std::unordered_set<state_t>::const_iterator, score_t> get_solution(size_t idx)
	{
		return solutions.at(idx);
	}

	std::vector<typename std::unordered_set<state_t>::const_iterator> get_path(size_t idx) const
	{
		if (idx >= solutions.size())
			return {};

		std::vector<typename std::unordered_set<state_t>::const_iterator> path;

		for (auto it = solutions[idx].first; it != closed.end(); it = parent_map.at(it))
			path.push_back(it);

		return path;
	}

	const auto& get_closed() const { return closed; }

private:
	static void progress(const graph_searcher &s, std::ostream &out)
	{
		while (s.running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - s.start_time)) << " #closed = " << s.closed.size() << ", #open = " << s.open.size() << "                \r";
			out.flush();
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}
		(out << std::string(50, ' ') << '\r').flush();
	}

	std::unordered_set<state_t> closed;
	priority_queue<state_t, score_t, typename std::unordered_set<state_t>::const_iterator> open;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	std::vector<std::pair<typename std::unordered_set<state_t>::const_iterator, score_t>> solutions;
	bool running;
	std::chrono::milliseconds elapsed_time;
	std::chrono::steady_clock::time_point start_time, stop_time;
};

/*
 * rzeczy dotyczące przeszukiwania drzew gier
 */
//export
enum class node_status
{
	max_won, min_won, draw, unresolved
};

//export
enum class side_to_move
{
	max_player, min_player
};

//export
constexpr side_to_move operator!(side_to_move stm)
{
	if (stm == side_to_move::max_player)
		return side_to_move::min_player;
	else
		return side_to_move::max_player;
}

//export
template<typename state_t>
struct status_test
{
	node_status operator()(const state_t&) const
	{
		throw std::runtime_error("Unimplemented!");
		return node_status::unresolved;
	}
};

//export
template<typename state_t>
class game_state
{
public:
	game_state(side_to_move stm) : stm{ stm }
	{
	}

	side_to_move get_stm() const { return stm; }
protected:
	side_to_move stm;
};

//template<typename T>
//concept is_unique_ptr = requires(const T& a) {
//	typename T::pointer;
//	typename T::element_type;
//	typename T::deleter_type;
//	a.get();
//	a.operator->();
//};

//template<side_to_move turn, typename score_t>
//inline constexpr score_t select_better(score_t a, score_t b)
//{
//	if constexpr (turn == side_to_move::max_player)
//		return std::max(a, b);
//	else
//		return std::min(a, b);
//}

//enum class score_type
//{
//	exact, lower_bound, upper_bound
//};

//template<typename score_t>
//struct tt_entry
//{
//	score_t score;
//	score_type s_type;
//	unsigned depth;
//};

//template<typename state_t, typename score_t>
//class fake_tt_interface
//{
//public:
//	std::optional<std::ref<tt_entry<score_t>>> probe(const state_t&)
//	{
//		return {};
//	}

//	void put(const state_t&, const tt_entry&)
//	{
//	}

//	void clear_tt()
//	{
//	}
//};

//template<typename state_t, typename score_t>
//class tt_interface : fake_tt_interface<state_t, score_t>
//{
//public:
//	std::optional<tt_entry> probe(const state_t& s)
//	{
//		auto it = tt.find(s);
//		if (it != tt.end())
//			return it->second;
//		return {};
//	}

//	void put(const state_t &s, const tt_entry &e)
//	{
//		//
//	}

//	void clear_tt()
//	{
//		tt.clear();
//	}

//private:
//	mutable std::unordered_map<state_t, tt_entry> tt;
//};

//export
template<typename state_t, typename score_t, template<typename> class Heuristic, bool use_tt=false>
class alpha_beta_searcher //: std::conditional_t<use_tt, tt_interface, fake_tt_interface>
{
//	static_assert(std::derived_from<state_t, game_state<state_t>>);
	static_assert(std::numeric_limits<score_t>::min() < 0);
	static constexpr Heuristic<score_t> heuristic {};
	static constexpr status_test<state_t> is_terminal {};

public:
	alpha_beta_searcher(unsigned max_depth) : max_depth{ max_depth }, visited{ 0 }, scores{}, best_idx{ 0 }
	{
	}

	template<typename Successors>
	void perform_search(Successors generator, const /*game_state<*/state_t/*>*/ &s)
	{
		side_to_move stm;
		if constexpr (is_unique_ptr<state_t>::value)
		{
			static_assert(std::derived_from<typename state_t::element_type, game_state<typename state_t::element_type>>);
			stm = s->get_stm();
		}
		else
		{
			static_assert(std::derived_from<state_t, game_state<state_t>>);
			stm = s.get_stm();
		}

		visited = 0;
		scores.clear();
		best_idx = 0;

		if (/*s.get_*/stm/*()*/ == side_to_move::max_player)
			alpha_beta_search<side_to_move::max_player>(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), max_depth);
//			alpha_beta_max(generator, /*static_cast<const state_t&>(*/s/*)*/, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), max_depth);
		else
			alpha_beta_search<side_to_move::min_player>(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), max_depth);
//			alpha_beta_min(generator, /*static_cast<const state_t&>(*/s/*)*/, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), max_depth);
	}

//	template<typename Successors>
//	typename std::enable_if_t<is_unique_ptr<std::remove_cvref_t<state_t>>>
//	/*void*/ perform_search(Successors generator, const state_t &s)
//	{
//		visited = 0;
//		scores.clear();
//		best_idx = 0;

//		if (s->get_stm() == side_to_move::max_player)
//			alpha_beta_max(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
//		else
//			alpha_beta_min(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
//	}

//	template<typename Successors, typename T=std::enable_if_t<is_unique_ptr<state_t>>>
//	void perform_search(Successors generator, const game_state<typename state_t::element_type> &s)
//	{
//		visited = 0;
//		scores.clear();
//		best_idx = 0;

//		if (s.get_stm() == side_to_move::max_player)
//			alpha_beta_max(generator, static_cast<const state_t&>(s), std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
////			alpha_beta_max(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
//		else
//			alpha_beta_min(generator, static_cast<const state_t&>(s), std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
////			alpha_beta_min(generator, s, std::numeric_limits<score_t>::min(), std::numeric_limits<score_t>::max(), 0);
//	}

	unsigned get_visited() const { return visited; }

	auto& get_scores() const { return scores; }

	auto& get_best() const { return scores[best_idx]; }

	/*template<typename Successors, typename SolutionPredicate>
	alpha_beta_searcher(state_t&& s0, Successors generator, SolutionPredicate is_solution)
	{
		running = true;

		start_time = std::chrono::steady_clock::now();
		auto thr = std::thread(progress, this, std::ref(std::cout));

		open.push(std::move(s0), heuristic(s0), closed.end());
		while (!open.empty())
		{
			auto [s, score, parent] = open.pop();
			auto s_closed = closed.insert(std::move(s)).first;
			parent_map[s_closed] = parent;

			if (is_solution(*s_closed))
			{
				solutions.push_back(std::make_pair(s_closed, score));
				break;
			}

			score -= heuristic(*s_closed); // zostaje samo g(s)

			for (state_t& t : generator(*s_closed))
			{
				if (closed.contains(t))
					continue;

				open.push(std::move(t), score + delta(*s_closed, t) + heuristic(t), s_closed);
			}
		}

		stop_time = std::chrono::steady_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

		running = false;
		thr.join();
	}

	std::string get_summary() const
	{
		std::ostringstream str;

		str << "elapsed time: " << parse_time(elapsed_time) << std::endl;
		str << "#visited    : " << closed.size() << std::endl;
		str << "#open       : " << open.size() << std::endl;

		return str.str();
	}

	size_t get_number_of_solutions() const { return solutions.size(); }

	std::pair<typename std::unordered_set<state_t>::const_iterator, score_t> get_solution(size_t idx)
	{
		return solutions.at(idx);
	}

	std::vector<typename std::unordered_set<state_t>::const_iterator> get_path(size_t idx) const
	{
		if (idx >= solutions.size())
			return {};

		std::vector<typename std::unordered_set<state_t>::const_iterator> path;

		for (auto it = solutions[idx].first; it != closed.end(); it = parent_map.at(it))
			path.push_back(it);

		return path;
	}

	const auto& get_closed() const { return closed; }*/

private:
/// https://people.csail.mit.edu/plaat/mtdf.html#abmem
//function AlphaBetaWithMemory(n : node_type; alpha , beta , d : integer) : integer;
//	if retrieve(n) == OK then /* Transposition table lookup */
//		if n.lowerbound >= beta then return n.lowerbound;
//		if n.upperbound <= alpha then return n.upperbound;
//		alpha := max(alpha, n.lowerbound);
//		beta := min(beta, n.upperbound);
//	if d == 0 then g := evaluate(n); /* leaf node */
//	else if n == MAXNODE then
//		g := -INFINITY; a := alpha; /* save original alpha value */
//		c := firstchild(n);
//		while (g < beta) and (c != NOCHILD) do
//			g := max(g, AlphaBetaWithMemory(c, a, beta, d - 1));
//			a := max(a, g);
//			c := nextbrother(c);
//	else /* n is a MINNODE */
//		g := +INFINITY; b := beta; /* save original beta value */
//		c := firstchild(n);
//		while (g > alpha) and (c != NOCHILD) do
//			g := min(g, AlphaBetaWithMemory(c, alpha, b, d - 1));
//			b := min(b, g);
//			c := nextbrother(c);
//	/* Traditional transposition table storing of bounds */
//	/* Fail low result implies an upper bound */
//	if g <= alpha then n.upperbound := g; store n.upperbound;
//	/* Found an accurate minimax value - will not occur if called with zero window */
//	if g >  alpha and g < beta then
//		n.lowerbound := g; n.upperbound := g; store n.lowerbound, n.upperbound;
//	/* Fail high result implies a lower bound */
//	if g >= beta then n.lowerbound := g; store n.lowerbound;
//	return g;

	template<side_to_move turn, typename Successors>
	score_t alpha_beta_search(Successors generator, const state_t &s, score_t alpha, score_t beta, unsigned depth) const
	{
		++visited;

		if constexpr (use_tt)
			if (auto it = tt.find(s); it != tt.end() && depth < it->second.depth)
			{
				switch (it->second.s_type)
				{
				case score_type::lower_bound:
					if (it->second.score >= beta)
						return it->second.score;
					alpha = std::max(alpha, it->second.score);
					break;
				case score_type::upper_bound:
					if (alpha >= it->second.score)
						return it->second.score;
					beta = std::min(beta, it->second.score);
					break;
				case score_type::exact:
					if (alpha >= it->second.score || it->second.score >= beta)
						return it->second.score;
					alpha = std::max(alpha, it->second.score);
					beta = std::min(beta, it->second.score);
					break;
				}
			}
		score_t g{};
		if (auto st = is_terminal(s); st != node_status::unresolved)
			switch (st)
			{
			default:
			case node_status::draw: g = 0; break;
			case node_status::max_won: g = std::numeric_limits<score_t>::max(); break;
			case node_status::min_won: g = std::numeric_limits<score_t>::min(); break;
			}
		else if (depth == 0)
			g = heuristic(s);
		else
		{
			score_t bnd;
			if constexpr (turn == side_to_move::max_player)
			{
				g = std::numeric_limits<score_t>::min();
				bnd = alpha;
			}
			else
			{
				g = std::numeric_limits<score_t>::max();
				bnd = beta;
			}
			for (auto &t : generator(s))
			{
				if constexpr (turn == side_to_move::max_player)
				{
					auto v = alpha_beta_search<side_to_move::min_player>(generator, t, bnd, beta, depth - 1);
					if (depth == max_depth)
					{
						scores.push_back(std::make_pair(std::move(t), v));
						if (scores[best_idx].second	< v)
							best_idx = scores.size() - 1;
					}
					g = std::max(g, v);
					bnd = std::max(bnd, g);
					if (g >= beta) break;
				}
				else
				{
					auto v = alpha_beta_search<side_to_move::max_player>(generator, t, alpha, bnd, depth - 1);
					if (depth == max_depth)
					{
						scores.push_back(std::make_pair(std::move(t), v));
						if (scores[best_idx].second	> v)
							best_idx = scores.size() - 1;
					}
					g = std::min(g, v);
					bnd = std::min(bnd, g);
					if (g <= alpha) break;
				}
			}
		}

		if constexpr (use_tt)
		{
			if (g <= alpha)
				tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {g, score_type::upper_bound, depth}});
			else if (beta <= g)
				tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {g, score_type::lower_bound, depth}});
			else
				tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {g, score_type::exact, depth}});
		}

		return g;
	}

	/* https://www.chessprogramming.org/Transposition_Table
	 * A cutoff can be performed when the depth of entry is greater (or equal) to the depth of the current node and one of the following criteria is satisfied:
	 * * The entry type is EXACT
	 * * The entry type is LOWERBOUND and greater than or equal to beta
	 * * The entry type is UPPERBOUND and less than alpha
	 */
	template<typename Successors>
	score_t alpha_beta_max(Successors generator, const state_t &s, score_t alpha, score_t beta, unsigned depth) const
	{
		visited++;

		if constexpr (use_tt)
		{
			auto it = tt.find(s);
			if (it != tt.end() && it->second.depth >= depth)
			{
#ifndef NDEBUG
				if (s.get_stm() != side_to_move::max_player)
					throw std::runtime_error("Wrong side (not max)!");
#endif
//				std::cout << "Hit: key = \n" << it->first.to_string() << "\nactual:\n" << s.to_string() << std::endl;
				switch (it->second.s_type)
				{
				case score_type::exact:
					return it->second.score;
				case score_type::lower_bound:
					if (it->second.score >= beta)
						return it->second.score;
					break;
				case score_type::upper_bound:
					if (it->second.score <= alpha)
						return it->second.score;
					break;
				}
			}
		}

		auto st = is_terminal(s);
		std::optional<score_t> score;
		switch (st)
		{
		case node_status::unresolved: break;
		case node_status::draw      : score = 0; break;
		case node_status::max_won   : score = std::numeric_limits<score_t>::max(); break;
		case node_status::min_won   : score = std::numeric_limits<score_t>::min(); break;
		}
		if (score)
		{
			if constexpr (use_tt)
				if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score.value(), score_type::exact, depth}}); !ins)
				{
					std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
					throw std::runtime_error("Entry already in TT!");
				}
			return score.value();
		}

//		if (depth >= max_depth)
		if (depth == 0)
		{
			score_t score = heuristic(s);

//			if constexpr (use_tt)
//				if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score, score_type::exact, depth}}); !ins)
//				{
//					std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
//					throw std::runtime_error("Entry already in TT!");
//				}

			return score;
		}

		for (auto &t : generator(s))
		{
			auto v = alpha_beta_min(generator, t, alpha, beta, depth - 1);

			if (depth == max_depth)
			{
				scores.emplace_back(std::move(t), v);
				if (scores[best_idx].second	< v)
					best_idx = scores.size() - 1;
			}

			alpha = std::max(alpha, v);

			if (alpha >= beta)
			{
//				if constexpr (use_tt)
//					if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {v, score_type::lower_bound, depth}}); !ins)
//					{
////						std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
////						throw std::runtime_error("Entry already in TT!");
//					}

				break;
			}
		}

		return alpha;
	}

	template<typename Successors>
	score_t alpha_beta_min(Successors generator, const state_t &s, score_t alpha, score_t beta, unsigned depth) const
	{
		visited++;

//		auto st = is_terminal(s);
//		switch (st)
//		{
//		case node_status::unresolved: break;
//		case node_status::draw: return 0;
//		case node_status::max_won: return std::numeric_limits<score_t>::max();
//		case node_status::min_won: return std::numeric_limits<score_t>::min();
//		}

//		if (depth >= max_depth)
//			return heuristic(s);

		if constexpr (use_tt)
		{
			auto it = tt.find(s);
			if (it != tt.end() && it->second.depth <= depth)
			{
#ifndef NDEBUG
				if (s.get_stm() != side_to_move::min_player)
					throw std::runtime_error("Wrong side (not min)!");
#endif
//				std::cout << "Hit: key = \n" << it->first.to_string() << "\nactual:\n" << s.to_string() << std::endl;
				switch (it->second.s_type)
				{
				case score_type::exact:
					return it->second.score;
				case score_type::lower_bound:
					if (it->second.score >= beta)
						return it->second.score;
					break;
				case score_type::upper_bound:
					if (it->second.score <= alpha)
						return it->second.score;
					break;
				}
			}
		}

		auto st = is_terminal(s);
		std::optional<score_t> score;
		switch (st)
		{
		case node_status::unresolved: break;
		case node_status::draw      : score = 0; break;
		case node_status::max_won   : score = std::numeric_limits<score_t>::max(); break;
		case node_status::min_won   : score = std::numeric_limits<score_t>::min(); break;
		}
		if (score)
		{
			if constexpr (use_tt)
				if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score.value(), score_type::exact, depth}}); !ins)
				{
					std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
					throw std::runtime_error("Entry already in TT!");
				}
			return score.value();
		}

//		if (depth >= max_depth)
		if (depth == 0)
		{
			score_t score = heuristic(s);

//			if constexpr (use_tt)
//				if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score, score_type::exact, depth}}); !ins)
//				{
//					std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
//					throw std::runtime_error("Entry already in TT!");
//				}

			return score;
		}

		for (auto &t : generator(s))
		{
			auto v = alpha_beta_max(generator, t, alpha, beta, depth - 1);

			if (depth == max_depth)
			{
				scores.emplace_back(std::move(t), v);
				if (scores[best_idx].second > v)
					best_idx = scores.size() - 1;
			}

			beta = std::min(beta, v);

			if (alpha >= beta)
			{
//				if constexpr (use_tt)
//					if (auto [it, ins] = tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {v, score_type::upper_bound, depth}}); !ins)
//					{
////						std::cout << it->first.to_string() << std::endl << s.to_string() << std::endl;
////						throw std::runtime_error("Entry already in TT!");
//					}
				break;
			}
		}

		return beta;
	}

	/*static void progress(const graph_searcher *s, std::ostream &out)
	{
		while (s->running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - s->start_time)) << " #closed = " << s->closed.size() << ", #open = " << s->open.size() << "                \r";
			out.flush();
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}
		out << std::endl;
	}*/

	unsigned max_depth;
	mutable unsigned visited;
	mutable std::vector<std::pair<state_t, score_t>> scores;
	mutable size_t best_idx;

	enum class score_type
	{
		exact, lower_bound, upper_bound
	};
	struct tt_entry
	{
		score_t score;
		score_type s_type;
		unsigned depth;
	};
	mutable std::unordered_map<state_t, tt_entry> tt;
//	using tt_type = std::conditional_t<use_tt, std::unordered_map<state_t, tt_entry>, void*>;
//	mutable tt_type tt;

//	std::unordered_set<state_t> closed;
//	priority_queue<state_t, score_t, typename std::unordered_set<state_t>::const_iterator> open;
//	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
//	std::vector<std::pair<typename std::unordered_set<state_t>::const_iterator, score_t>> solutions;
//	bool running;
//	std::chrono::milliseconds elapsed_time;
//	std::chrono::steady_clock::time_point start_time, stop_time;
};
#endif
