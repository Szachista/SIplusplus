#ifndef STATE_HPP
#define STATE_HPP

#include <algorithm>
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
#include <cstring>
using namespace std::literals;

template <typename...> struct is_unique_ptr final : std::false_type {};
template<class T, typename... Args>
struct is_unique_ptr<std::unique_ptr<T, Args...>> final : std::true_type {};

template <typename...> struct is_pair final : std::false_type {};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> final : std::true_type {};

template<typename duration>
static std::string parse_time(duration dur)
{
	auto t = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
	std::ostringstream str;
	str.fill('0');

	const int64_t ms = abs(t.count() % 1000), s = abs(t.count()/1000) % 60,
			m = abs(t.count()/60000) % 60, h = t.count() / 3600000;

	str << std::setw(2) << h << ':' << std::setw(2) << m << ':'
		<< std::setw(2) << s << '.' << std::setw(3) << ms;

	return str.str();
}

// export
template<typename score_t>
class delta_zero
{
public:
	template<typename state_t>
	constexpr score_t operator()(const state_t&, const state_t&) const
	{
		return 0;
	}
};

//export
template<typename score_t>
class delta_unit
{
public:
	template<typename state_t>
	constexpr score_t operator()(const state_t&, const state_t&) const
	{
		return 1;
	}
};

//export
template<typename score_t>
class no_heuristic
{
public:
	template<typename state_t>
	constexpr score_t operator()(const state_t&) const
	{
		return 0;
	}
};

template<typename state_t>
struct hash_iterator
{
	size_t operator()(std::unordered_set<state_t>::const_iterator it) const
	{
		return h(*it);
	}
	static constexpr std::hash<state_t> h {};
};

template<typename state_t>
struct equal_to_iterator
{
	bool operator()(std::unordered_set<state_t>::const_iterator a, std::unordered_set<state_t>::const_iterator b) const
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
	[[nodiscard]] bool empty() const { return q.empty(); }

	[[nodiscard]] size_t size() const { return q.size(); }

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
				return true;
			}
		}
		else
		{
			auto [iter, inserted] = obj2prop.emplace(std::move(t), prop{score, parent, q.size()});
			q.push_back(iter);
			up(q.size() - 1);
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

		return { std::move(state), score, parent };
	}

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
				  std::chrono::steady_clock::duration elapsed_time) :
		closed(std::move(closed)),
		parent_map(std::move(parent_map)),
		open(std::move(open)),
		solutions(std::move(solutions)),
		elapsed_time(elapsed_time)
	{
	}

	[[nodiscard]] std::string get_summary() const
	{
		std::ostringstream str;

		str << "elapsed time: " << parse_time(get_elapsed_time()) << std::endl;
		str << "#visited    : " << closed.size() << std::endl;
		str << "#open       : " << open.size() << std::endl;
		str << "nodes/second: " << (closed.size() + open.size())*1000000000ull/get_elapsed_time<std::chrono::nanoseconds>().count() << std::endl;

		return str.str();
	}

	[[nodiscard]] size_t get_number_of_solutions() const { return solutions.size(); }

	auto get_solution(size_t idx) const { return solutions.at(idx); }

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

	const auto& get_closed() const { return closed; }

	const auto& get_open() const { return open; }

	template<typename Unit=std::chrono::milliseconds>
	Unit get_elapsed_time() const { return std::chrono::duration_cast<Unit>(elapsed_time); }

private:
	std::unordered_set<state_t> closed;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	Open open;
	Solutions solutions;
	std::chrono::steady_clock::duration elapsed_time;
};

// TODO złączyć obie funkcje w jedno, przemyśleć delta/heuristic (może nie trzeba sprawdzać != nullptr)

//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0,
						  std::vector<state_t>(state_t::*successors)() const,
						  bool (state_t::*is_solution)() const,
						  score_t (state_t::*delta)(const state_t&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	std::unordered_set<state_t> closed;
	std::conditional_t<std::is_same_v<score_t, void>, std::unordered_map<state_t, typename std::unordered_set<state_t>::const_iterator>, priority_queue<state_t, score_t, typename std::unordered_set<state_t>::const_iterator>> open;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	std::conditional_t<std::is_same_v<score_t, void>,
			std::vector<typename std::unordered_set<state_t>::const_iterator>,
			std::vector<std::pair<typename std::unordered_set<state_t>::const_iterator, score_t>>> solutions;
	std::chrono::steady_clock::time_point start_time;

	std::ostream no_output {nullptr};
	bool running = true;

	auto progress = [&running, &start_time, &closed, &open](std::ostream &out)
	{
		while (running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)) << " #closed = " << closed.size() << ", #open = " << open.size() << "				\r";
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
		while (!open.empty() && solutions.empty())
		{
			auto old_open = std::move(open);
			while (!old_open.empty())
			{
				auto item = old_open.extract(old_open.begin());
				auto s_closed = closed.insert(std::move(item.key())).first;
				parent_map[s_closed] = item.mapped();

				if (std::invoke(is_solution, *s_closed))
				{
					solutions.push_back(s_closed);
					break;
				}

				for (auto &t : std::invoke(successors, *s_closed))
					if (!closed.contains(t))
						open.emplace(std::move(t), s_closed);
			}
		}
	}
	else
	{
		open.push(std::move(s0), (heuristic != nullptr ? std::invoke(heuristic, s0) : 0), closed.end());
		while (!open.empty())
		{
			auto [s, score, parent] = open.pop();
			auto s_closed = closed.insert(std::move(s)).first;
			parent_map[s_closed] = parent;

			if (std::invoke(is_solution, *s_closed))
			{
				solutions.push_back(std::make_pair(s_closed, score));
				break;
			}

			if (heuristic != nullptr)
				score -= std::invoke(heuristic, *s_closed); // zostaje samo g(s)

			for (auto& t : std::invoke(successors, *s_closed))
			{
				if (closed.contains(t))
					continue;

				score_t h = (heuristic != nullptr ? std::invoke(heuristic, t) : 0);
				score_t d = (delta != nullptr ? std::invoke(delta, *s_closed, t) : 0);
				open.push(std::move(t), score + d + h, s_closed);
			}
		}
	}

	std::chrono::steady_clock::time_point stop_time = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration elapsed_time = stop_time - start_time;

	running = false;
	thr.join();

	return search_result(std::move(closed), std::move(parent_map), std::move(open), std::move(solutions), elapsed_time);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(std::unique_ptr<state_t> &&s0,
						  std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const,
						  bool (state_t::*is_solution)() const,
						  score_t (state_t::*delta)(const std::unique_ptr<state_t>&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	std::unordered_set<std::unique_ptr<state_t>> closed;
	std::conditional_t<std::is_same_v<score_t, void>,
					   std::unordered_map<std::unique_ptr<state_t>, typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator>,
					   priority_queue<std::unique_ptr<state_t>, score_t, typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator>> open;
	std::unordered_map<typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator, typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator, hash_iterator<std::unique_ptr<state_t>>, equal_to_iterator<std::unique_ptr<state_t>>> parent_map;
	std::conditional_t<std::is_same_v<score_t, void>,
					   std::vector<typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator>,
					   std::vector<std::pair<typename std::unordered_set<std::unique_ptr<state_t>>::const_iterator, score_t>>> solutions;
	std::chrono::steady_clock::time_point start_time;

	std::ostream no_output {nullptr};
	bool running = true;

	auto progress = [&running, &start_time, &closed, &open](std::ostream &out)
	{
		while (running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)) << " #closed = " << closed.size() << ", #open = " << open.size() << "				\r";
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
		while (!open.empty() && solutions.empty())
		{
			auto old_open = std::move(open);
			while (!old_open.empty())
			{
				auto item = old_open.extract(old_open.begin());
				auto s_closed = closed.insert(std::move(item.key())).first;
				parent_map[s_closed] = item.mapped();

				if (std::invoke(is_solution, *s_closed))
				{
					solutions.push_back(s_closed);
					break;
				}

				for (auto &t : std::invoke(successors, *s_closed))
					if (!closed.contains(t))
						open.emplace(std::move(t), s_closed);
			}
		}
	}
	else
	{
		open.push(std::move(s0), (heuristic != nullptr ? std::invoke(heuristic, s0) : 0), closed.end());
		while (!open.empty())
		{
			auto [s, score, parent] = open.pop();
			auto s_closed = closed.insert(std::move(s)).first;
			parent_map[s_closed] = parent;

			if (std::invoke(is_solution, *s_closed))
			{
				solutions.push_back(std::make_pair(s_closed, score));
				break;
			}

			if (heuristic != nullptr)
				score -= std::invoke(heuristic, *s_closed); // zostaje samo g(s)

			for (auto& t : std::invoke(successors, *s_closed))
			{
				if (closed.contains(t))
					continue;

				score_t h = (heuristic != nullptr ? std::invoke(heuristic, t) : 0);
				score_t d = (delta != nullptr ? std::invoke(delta, *s_closed, t) : 0);
				open.push(std::move(t), score + d + h, s_closed);
			}
		}
	}

	std::chrono::steady_clock::time_point stop_time = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration elapsed_time = stop_time - start_time;

	running = false;
	thr.join();

	return search_result(std::move(closed), std::move(parent_map), std::move(open), std::move(solutions), elapsed_time);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0,
						  std::vector<state_t>(state_t::*successors)() const,
						  bool (state_t::*is_solution)() const,
						  score_t (state_t::*delta)(const state_t&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	return ::informative_searcher(state_t{s0}, successors, is_solution, delta, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const std::unique_ptr<state_t> &s0,
						  std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const,
						  bool (state_t::*is_solution)() const,
						  score_t (state_t::*delta)(const std::unique_ptr<state_t>&) const,
						  score_t (state_t::*heuristic)() const,
						  bool show_progress=false)
{
	return ::informative_searcher(std::make_unique<state_t>(*s0), successors, is_solution, delta, heuristic, show_progress);
}

/*
 * Breadth First Search
 */
//export
template<typename state_t>
auto informative_searcher(state_t &&s0, std::vector<state_t>(state_t::*successors)() const, bool (state_t::*is_solution)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, void>(std::move(s0), successors, is_solution, nullptr, nullptr, show_progress);
}

//export
template<typename state_t>
auto informative_searcher(const state_t &s0, std::vector<state_t>(state_t::*successors)() const, bool (state_t::*is_solution)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, void>(state_t{s0}, successors, is_solution, nullptr, nullptr, show_progress);
}

//export
template<typename state_t>
auto informative_searcher(std::unique_ptr<state_t> &&s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, void>(std::move(s0), successors, is_solution, nullptr, nullptr, show_progress);
}

//export
template<typename state_t>
auto informative_searcher(const std::unique_ptr<state_t> &s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, void>(std::make_unique<state_t>(*s0), successors, is_solution, nullptr, nullptr, show_progress);
}

/*
 * Dijkstra
 */
//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0, std::vector<state_t>(state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*delta)(const state_t&) const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::move(s0), successors, is_solution, delta, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0, std::vector<state_t>(state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*delta)(const state_t&) const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(state_t{s0}, successors, is_solution, delta, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(std::unique_ptr<state_t> &&s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*delta)(const std::unique_ptr<state_t>&) const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::move(s0), successors, is_solution, delta, nullptr, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const std::unique_ptr<state_t> &s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*delta)(const std::unique_ptr<state_t>&) const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::make_unique<state_t>(*s0), successors, is_solution, delta, nullptr, show_progress);
}

/*
 * Best First Search
 */
//export
template<typename state_t, typename score_t>
auto informative_searcher(state_t &&s0, std::vector<state_t>(state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::move(s0), successors, is_solution, nullptr, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const state_t &s0, std::vector<state_t> (state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(state_t{s0}, successors, is_solution, nullptr, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(std::unique_ptr<state_t> &&s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::move(s0), successors, is_solution, nullptr, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t>
auto informative_searcher(const std::unique_ptr<state_t> &s0, std::vector<std::unique_ptr<state_t>> (state_t::*successors)() const, bool (state_t::*is_solution)() const, score_t (state_t::*heuristic)() const, bool show_progress=false)
{
	return ::informative_searcher<state_t, score_t>(std::make_unique<state_t>(*s0), successors, is_solution, nullptr, heuristic, show_progress);
}

//export
template<typename state_t, typename score_t, template<typename> class Delta, template<typename> class Heuristic>
class [[deprecated("Use informative_searcher instead")]] graph_searcher
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

	[[nodiscard]] std::string get_summary() const
	{
		std::ostringstream str;

		str << "elapsed time: " << parse_time(elapsed_time) << std::endl;
		str << "#visited    : " << closed.size() << std::endl;
		str << "#open       : " << open.size() << std::endl;
		str << "nodes/second: " << (closed.size() + open.size())*1000ull/std::max<size_t>(elapsed_time.count(),1) << std::endl;

		return str.str();
	}

	[[nodiscard]] size_t get_number_of_solutions() const { return solutions.size(); }

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
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - s.start_time)) << " #closed = " << s.closed.size() << ", #open = " << s.open.size() << "				\r";
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

//
// rzeczy dotyczące przeszukiwania drzew gier
//

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
	return (stm == side_to_move::max_player ? side_to_move::min_player : side_to_move::max_player);
}

//export
template<typename state_t>
struct status_test
{
	node_status operator()(const state_t&) const
	{
		return node_status::unresolved;
	}
};

class timeout final : public std::exception
{
public:
	[[nodiscard]] const char *what() const noexcept override { return "Run out of time!"; }
};

//export
template<typename state_t, typename move_t>
class game_state
{
public:
	using move_type = move_t;
	explicit game_state(side_to_move stm) : stm{ stm }
	{
	}

	[[nodiscard]] side_to_move get_stm() const { return stm; }

	move_t get_move() const
	{
//		return static_cast<const state_t*>(this)->get_move();
		return m_move;
	}

	template<typename T>
	T no_heuristic() const { return static_cast<T>(0); }

	void set_move(const move_t &move)
	{
		m_move = move;
	}
protected:
	side_to_move stm;
	move_t m_move;
};

enum class transposition_table_type
{
	no_tt, std_unordered_map, fixed
};

//export
template<typename state_t, typename score_t=int, transposition_table_type tt_type=transposition_table_type::no_tt>
class alpha_beta_searcher
{
	// static_assert(std::derived_from<state_t, game_state<state_t>>);
	// static_assert(std::numeric_limits<score_t>::lowest() < 0);
	static_assert(std::numeric_limits<score_t>::lowest() <= -std::numeric_limits<score_t>::max());
	// static_assert(use_tt == false);
	static constexpr status_test<state_t> is_terminal {};

	score_t (state_t::*heuristic)() const;
	std::vector<state_t>(state_t::*successors)() const;

	std::chrono::steady_clock::time_point start_time;
	std::chrono::nanoseconds search_time_limit;
	std::chrono::steady_clock::duration elapsed_time;

	using move_type = state_t::move_type;

public:
	template<typename duration_type=std::chrono::nanoseconds>
	alpha_beta_searcher(unsigned max_depth, std::vector<state_t> (state_t::*successors)() const, score_t (state_t::*heuristic)() const=&state_t::template no_heuristic<score_t>, duration_type search_time_limit=std::chrono::nanoseconds::max()) : heuristic{heuristic}, successors{successors}, search_time_limit{std::chrono::duration_cast<std::chrono::nanoseconds>(search_time_limit)}, elapsed_time {0}, m_max_depth{ max_depth }, m_visited{ 0 }, m_pv_array(m_max_depth * (m_max_depth + 1) / 2), m_pv_length(m_max_depth + 1, 0), m_tt_hits{0}, m_tt_miss{0}
	{
		if (heuristic == nullptr)
			heuristic = &state_t::template no_heuristic<score_t>;
		if constexpr (tt_type == transposition_table_type::fixed)
			m_tt_fixed.resize(100'000'000);
	}

	void perform_search(const state_t &s, bool use_iterative_deepening=true)
	{
		m_visited = 0;
		m_tt_hits = m_tt_miss = 0;
		m_best_move.second = (s.get_stm() == side_to_move::max_player ? std::numeric_limits<score_t>::lowest() : std::numeric_limits<score_t>::max());
		start_time = std::chrono::steady_clock::now();

		if (use_iterative_deepening)
		{
			std::vector<move_type> prev_pv_stack, old_pv;
			prev_pv_stack.reserve(m_max_depth);
			try
			{
				for (unsigned depth = 1; depth <= m_max_depth; depth++)
				{
					clear_tt();

					auto v = alpha_beta_search(s, std::numeric_limits<score_t>::lowest(), std::numeric_limits<score_t>::max(), depth, m_pv_array.begin(), m_pv_length.begin(), prev_pv_stack);
					m_best_move = {m_pv_array[0], v};

					prev_pv_stack.resize(m_pv_length[0]);
					std::copy_n(m_pv_array.begin(), m_pv_length[0], prev_pv_stack.rbegin());
					old_pv.resize(m_pv_length[0]);
					std::copy_n(m_pv_array.begin(), m_pv_length[0], old_pv.begin());

					if (m_pv_length[0] < depth)
						break;
				}
			}
			catch (timeout&)
			{
				m_pv_length[0] = old_pv.size();
				std::ranges::copy(old_pv, m_pv_array.begin());
			}
		}
		else
			alpha_beta_search(s, std::numeric_limits<score_t>::lowest(), std::numeric_limits<score_t>::max(), m_max_depth);

		elapsed_time = std::chrono::steady_clock::now() - start_time;
	}

	[[nodiscard]] std::string get_summary() const
	{
		double total = std::max<size_t>(m_tt_hits + m_tt_miss, 1);
		return std::format("#Visited     : {}\nElapsed time : {}\nNodes per sec: {:.0f}\nTT hits (%)  : {:5.2f}\nTT miss (%)  : {:5.2f}",
			m_visited,
			parse_time(elapsed_time),
			m_visited*1e6/std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count(),
			m_tt_hits*1e2/total,
			m_tt_miss*1e2/total
		);
	}

	std::vector<move_type> get_pv() const { return std::vector<move_type>(m_pv_array.begin(), m_pv_array.begin() + m_pv_length[0]); }

	void clear_tt()
	{
		if constexpr (tt_type == transposition_table_type::std_unordered_map)
			m_tt.clear();

		if constexpr (tt_type == transposition_table_type::fixed)
			std::memset(m_tt_fixed.data(), sizeof(m_tt_fixed[0]), m_tt_fixed.size());
	}

	void set_tt_size([[maybe_unused]] size_t new_size)
	{
		if constexpr (tt_type == transposition_table_type::fixed)
		{
			if (new_size == 0)
				throw std::invalid_argument("New size must be positive!");

			m_tt_fixed.resize(new_size);
		}
	}

	[[nodiscard]] unsigned get_visited() const { return m_visited; }

	auto get_best_move() const { return m_best_move; }

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
		str << "#visited	: " << closed.size() << std::endl;
		str << "#open	   : " << open.size() << std::endl;

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

	score_t alpha_beta_search(const state_t &s, score_t alpha, score_t beta, unsigned depth)
	{
		++m_visited;

		if (auto entry = retrieve(s); entry != nullptr && depth >= entry->depth)
		{
			switch (entry->s_type)
			{
			case score_type::lower_bound:
				if (entry->score >= beta)
					return entry->score;
				alpha = std::max(alpha, entry->score);
				break;
			case score_type::upper_bound:
				if (alpha >= entry->score)
					return entry->score;
				beta = std::min(beta, entry->score);
				break;
			case score_type::exact:
				if (alpha >= entry->score || entry->score >= beta)
					return entry->score;
				alpha = std::max(alpha, entry->score);
				beta = std::min(beta, entry->score);
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
			case node_status::min_won: g = std::numeric_limits<score_t>::lowest(); break;
			}
		else if (depth == 0)
			g = std::invoke(heuristic, s);
		else if (s.get_stm() == side_to_move::max_player)
		{
			score_t a = alpha;
			g = std::numeric_limits<score_t>::lowest();

			for (auto &t : std::invoke(successors, s))
			{
				auto v = alpha_beta_search(t, a, beta, depth - 1);

				if (depth == m_max_depth && v > m_best_move.second)
					m_best_move = {t.get_move(), v};

				g = std::max(g, v);
				a = std::max(a, g);
				if (g >= beta) break;
			}
		}
		else
		{
			score_t b = beta;
			g = std::numeric_limits<score_t>::max();

			for (auto &t : std::invoke(successors, s))
			{
				auto v = alpha_beta_search(t, alpha, b, depth - 1);

				if (depth == m_max_depth && v < m_best_move.second)
					m_best_move = {t.get_move(), v};

				g = std::min(g, v);
				b = std::min(b, g);
				if (g <= alpha) break;
			}
		}

		if (g <= alpha)
			store(s, {g, score_type::upper_bound, depth});
		else if (beta <= g)
			store(s, {g, score_type::lower_bound, depth});
		else
			store(s, {g, score_type::exact, depth});

		return g;
	}

	score_t alpha_beta_search(const state_t &s, score_t alpha, score_t beta, unsigned depth, std::vector<move_type>::iterator pv, std::vector<size_t>::iterator pv_length, std::vector<move_type> &prev_pv_stack/*, unsigned parent_id*/)
	{
		// static unsigned tab = 0;
		static move_type moves_stack[128], *moves_ptr = moves_stack;
		static uint32_t stack_len = 0;

		struct inc_dec
		{	// wyjścia w różnych punktach, po to opakowanie
			~inc_dec()
			{
				// --cnt;
				--moves_ptr;
			}
			explicit inc_dec(/*unsigned &cnt,*/ move_type *&moves_ptr, move_type m) : /*cnt{cnt},*/ moves_ptr(moves_ptr)
			{
				*moves_ptr++ = m;
				// ++cnt;
			}
			// unsigned &cnt;
			move_type *&moves_ptr;
		} _{/*tab,*/ moves_ptr, s.get_move()};
		stack_len = moves_ptr - moves_stack;

		// std::cout << '\n' << std::string(12*(tab-1), ' ') << std::format("{} (α={}, β={}): ", (s.get_stm() == side_to_move::max_player ? "MAX" : "MIN"), alpha, beta) << s.get_move();

		*pv_length = 0;
		++m_visited;

		if (std::chrono::steady_clock::now() - start_time >= search_time_limit)
			throw timeout();

		if (prev_pv_stack.empty())
			if (auto entry = retrieve(s); entry != nullptr && depth >= entry->depth)
			{
				switch (entry->s_type)
				{
				case score_type::lower_bound:
					if (entry->score >= beta)
						return entry->score;
					alpha = std::max(alpha, entry->score);
					break;
				case score_type::upper_bound:
					if (alpha >= entry->score)
						return entry->score;
					beta = std::min(beta, entry->score);
					break;
				case score_type::exact:
					if (alpha >= entry->score || entry->score >= beta)
						return entry->score;
					alpha = std::max(alpha, entry->score);
					beta = std::min(beta, entry->score);
					break;
				}
			}

		switch (is_terminal(s))
		{
		default: break;
		case node_status::draw: return 0;
		case node_status::max_won: return std::numeric_limits<score_t>::max();
		case node_status::min_won: return std::numeric_limits<score_t>::lowest();
		}

		if (depth == 0)
		{
			auto score = std::invoke(heuristic, s);
			store(s, {score, score_type::exact, depth});
			return score;
		}

		auto children = std::invoke(successors, s);
		if (!prev_pv_stack.empty())
		{
			const auto &move = prev_pv_stack.back();
			auto it = std::find_if(children.begin(), children.end(),[&move](const state_t &t){return t.get_move() == move; });
			if (it == children.end())
				throw std::runtime_error(std::format("Something went wrong: missing move '{}'", move));
			std::swap(*children.begin(), *it);
			prev_pv_stack.pop_back();
		}

		if (s.get_stm() == side_to_move::max_player)
		{
			for (auto &t : children)
			{
				auto v = alpha_beta_search(t, alpha, beta, depth - 1, pv + depth, pv_length + 1, prev_pv_stack/*, node_id*/);

				if (v >= beta)
				{
					if (depth == stack_len && *pv_length == 0)
					{	// max ma zwycięską sekwencję
						std::copy_n(moves_stack + 1, stack_len - 1, pv);
						*pv_length = stack_len - 1;
					}

					// std::cout << std::format(" CUT {}: {} <{}>", (s.get_stm() == side_to_move::max_player ? "MAX" : "MIN"), s.get_move(), v);
					store(s, {v, score_type::lower_bound, depth});
					return v;
				}

				if (v > alpha)
				{
					*pv = t.get_move();
					// std::copy(pv + depth, pv + 2*depth - 1, pv + 1);
					std::copy(pv + depth, pv + depth + pv_length[1], pv + 1);
					*pv_length = pv_length[1] + 1;
					alpha = v;
				}
			}
			// std::cout << std::format(" EXIT {} (α={}, β={}): {} [{}]", (s.get_stm() == side_to_move::max_player ? "MAX" : "MIN"), alpha, beta, s.get_move(), alpha);

			store(s, {alpha, score_type::exact, depth});
			return alpha;
		}
		else
		{
			for (auto &t : children)
			{
				auto v = alpha_beta_search(t, alpha, beta, depth - 1, pv + depth, pv_length + 1, prev_pv_stack/*, node_id*/);

				if (v <= alpha)
				{
					if (depth == stack_len && *pv_length == 0)
					{	// min ma zwycięską sekwencję
						std::copy_n(moves_stack + 1, stack_len - 1, pv);
						*pv_length = stack_len - 1;
					}

					// std::cout << std::format(" CUT {}: {} <{}>", (s.get_stm() == side_to_move::max_player ? "MAX" : "MIN"), s.get_move(), v);
					store(s, {v, score_type::upper_bound, depth});
					return v;
				}

				if (v < beta)
				{
					*pv = t.get_move();
					// std::copy(pv + depth, pv + 2*depth - 1, pv + 1);
					std::copy(pv + depth, pv + depth + pv_length[1], pv + 1);
					*pv_length = pv_length[1] + 1;
					beta = v;
				}
			}
			// (std::cout << std::format(" EXIT {} (α={}, β={}): {} [{}]", (s.get_stm() == side_to_move::max_player ? "MAX" : "MIN"), alpha, beta, s.get_move(), beta)).flush();

			store(s, {beta, score_type::exact, depth});
			return beta;
		}
	}

	/* https://www.chessprogramming.org/Transposition_Table
	 * A cutoff can be performed when the depth of entry is greater (or equal) to the depth of the current node and one of the following criteria is satisfied:
	 * * The entry type is EXACT
	 * * The entry type is LOWERBOUND and greater than or equal to beta
	 * * The entry type is UPPERBOUND and less than alpha
	 */
	/*template<typename Successors>
	score_t alpha_beta_max(Successors generator, const state_t &s, score_t alpha, score_t beta, unsigned depth) const
	{
		m_visited++;

		if constexpr (use_tt)
		{
			auto it = m_tt.find(s);
			if (it != m_tt.end() && it->second.depth >= depth)
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
		case node_status::draw	  : score = 0; break;
		case node_status::max_won   : score = std::numeric_limits<score_t>::max(); break;
		case node_status::min_won   : score = std::numeric_limits<score_t>::lowest(); break;
		}
		if (score)
		{
			if constexpr (use_tt)
				if (auto [it, ins] = m_tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score.value(), score_type::exact, depth}}); !ins)
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

			if (depth == m_max_depth)
			{
				m_scores.emplace_back(std::move(t), v);
				if (m_scores[m_best_idx].second	< v)
					m_best_idx = m_scores.size() - 1;
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
		m_visited++;

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
			auto it = m_tt.find(s);
			if (it != m_tt.end() && it->second.depth <= depth)
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
		case node_status::draw	  : score = 0; break;
		case node_status::max_won   : score = std::numeric_limits<score_t>::max(); break;
		case node_status::min_won   : score = std::numeric_limits<score_t>::lowest(); break;
		}
		if (score)
		{
			if constexpr (use_tt)
				if (auto [it, ins] = m_tt.emplace(std::pair<state_t, tt_entry>{std::move(s), {score.value(), score_type::exact, depth}}); !ins)
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

			if (depth == m_max_depth)
			{
				m_scores.emplace_back(std::move(t), v);
				if (m_scores[m_best_idx].second > v)
					m_best_idx = m_scores.size() - 1;
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
	}*/

	unsigned m_max_depth;
	unsigned m_visited;
	std::pair<move_type, score_t> m_best_move;
	std::vector<typename state_t::move_type> m_pv_array;
	std::vector<size_t> m_pv_length;

	enum class score_type { exact, lower_bound, upper_bound };
	struct tt_entry
	{
		score_t score;
		score_type s_type;
		unsigned depth;
	};

	tt_entry* retrieve(const state_t &s)
	{
		tt_entry *res = nullptr;
		if constexpr (tt_type == transposition_table_type::std_unordered_map)
		{
			auto it = m_tt.find(s);
			if (it != m_tt.end())
				res = &it->second;
		}

		if constexpr (tt_type == transposition_table_type::fixed)
		{
			size_t hashcode = std::hash<state_t>{}(s);
			auto &entry = m_tt_fixed[hashcode % m_tt_fixed.size()];
			if (entry.first == hashcode)
				res = &entry.second;
		}

		++(res ? m_tt_hits : m_tt_miss);
		return nullptr;
	}

	void store(const state_t &s, const tt_entry &entry)
	{
		if constexpr (tt_type == transposition_table_type::std_unordered_map)
			m_tt[s] = entry;

		if constexpr (tt_type == transposition_table_type::fixed)
		{
			size_t hashcode = std::hash<state_t>{}(s);
			auto &item = m_tt_fixed[hashcode % m_tt_fixed.size()];
			item.first = hashcode;
			item.second = entry;
		}
	}
	std::unordered_map<state_t, tt_entry> m_tt;
	std::vector<std::pair<size_t, tt_entry>> m_tt_fixed;
	size_t m_tt_hits, m_tt_miss;
};

template<typename state_t, typename score_t, typename duration_type=std::chrono::nanoseconds>
constexpr auto search_with_tt_fixed(unsigned max_depth, score_t (state_t::*heuristic)() const, std::vector<state_t> (state_t::*successors)() const, duration_type search_time_limit=std::chrono::nanoseconds::max())
{
	return alpha_beta_searcher<state_t, score_t, transposition_table_type::fixed>(max_depth, heuristic, successors, search_time_limit);
}

template<typename state_t, typename score_t, typename duration_type=std::chrono::nanoseconds>
constexpr auto search_with_tt_std(unsigned max_depth, score_t (state_t::*heuristic)() const, std::vector<state_t> (state_t::*successors)() const, duration_type search_time_limit=std::chrono::nanoseconds::max())
{
	return alpha_beta_searcher<state_t, score_t, transposition_table_type::std_unordered_map>(max_depth, heuristic, successors, search_time_limit);
}
#endif
