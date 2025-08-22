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
#include <future>

//export module state;
//import std;

static std::string parse_time(std::chrono::milliseconds t)
{
	std::ostringstream str;
	str.fill('0');

	const int ms = abs(t.count() % 1000), s = abs(t.count()/1000) % 60,
			m = abs(t.count()/60000) % 60, h = t.count() / 3600000;

	str << std::setw(2) << h << ':' << std::setw(2) << m << ':'
		<< std::setw(2) << s << '.' << std::setw(3) << ms;

	return str.str();
}

//export
template<typename state_t, typename score_t>
class delta_zero
{
public:
	constexpr score_t operator()(const state_t&, const state_t&) const
	{
		return 0;
	}
};

//export
template<typename state_t, typename score_t>
class delta_unit
{
public:
	constexpr score_t operator()(const state_t&, const state_t&) const
	{
		return 1;
	}
};

//export
template<typename state_t, typename score_t>
class no_heuristic
{
public:
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
				up(old_prop.score);
				down(old_prop.score);
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
		score_t score = q[0]->second.score;
		parent_t parent = q[0]->second.parent;
		state_t state = std::move(obj2prop.extract(q[0]).key());

		if (q.size() > 0)
			std::swap(q.front(), q.back());

		q.pop_back();

		q[0]->second.idx = 0;
		down(0);

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

			if (q[p]->second.score < q[idx]->second.score)
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
template<typename state_t, typename score_t, template<typename, typename> class Delta, template<typename, typename> class Heuristic>
class searcher
{
	static constexpr Delta<state_t, score_t> delta {};
	static constexpr Heuristic<state_t, score_t> heuristic {};

public:
	template<typename Successors, typename SolutionPredicate>
	searcher(state_t&& s0, Successors generator, SolutionPredicate is_solution)
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

private:
	static void progress(const searcher *s, std::ostream &out)
	{
		while (s->running)
		{
			out << parse_time(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - s->start_time)) << " #closed = " << s->closed.size() << ", #open = " << s->open.size() << "                \r";
			out.flush();
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}
		out << std::endl;
	}

	std::unordered_set<state_t> closed;
	priority_queue<state_t, score_t, typename std::unordered_set<state_t>::const_iterator> open;
	std::unordered_map<typename std::unordered_set<state_t>::const_iterator, typename std::unordered_set<state_t>::const_iterator, hash_iterator<state_t>, equal_to_iterator<state_t>> parent_map;
	std::vector<std::pair<typename std::unordered_set<state_t>::const_iterator, score_t>> solutions;
	bool running;
	std::chrono::milliseconds elapsed_time;
	std::chrono::steady_clock::time_point start_time, stop_time;
};

#endif
