#ifndef GRAPH_SEARCH_HPP
#define GRAPH_SEARCH_HPP

#include "graph_state.hpp"
#include "queue.hpp"
#include <vector>
#include <memory>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <cstdio>

std::string parse_time(int64_t ms)
{
	if (ms < 0)
		throw std::domain_error("Negative measure!");
	char format[32] = {0};
	std::snprintf(format,
			sizeof(format),
			"%02d:%02d:%02d.%03d",
			static_cast<int>(ms / 3600000),
			static_cast<int>((ms / 60000) % 60),
			static_cast<int>((ms / 1000) % 60),
			static_cast<int>(ms % 1000));
	return format;
}

template<typename score_type>
class informative_searcher
{
public:
	/**
	 * @brief Constructs object and performs actual search.
	 * @param initial State from which searching is performed.
	 * @param comp Comparator used by priority queue.
	 * @param max_solutions Maximum number of solutions to find.
	 */
	informative_searcher(const graph_state<score_type> &initial,
						 std::function<bool(const graph_state<score_type>&, const graph_state<score_type>&)> comp,
						 size_t max_solutions = 1)
		: open(comp)
	{
		start_time = std::chrono::steady_clock::now();
		if (max_solutions == 0)
		{
			stop_time = start_time;
			return;
		}
		auto initial_clone = initial.clone();
		initial_clone->set_parent(nullptr);
		initial_clone->update_score(0);
		open.push(std::unique_ptr<graph_state<score_type>>(std::move(initial_clone)));
		while (!open.empty())
		{
			auto s = std::move(open.poll());

			if (s->is_solution())
			{
				solutions.push_back(std::move(s));
				if (solutions.size() >= max_solutions)
					break;
				else
					continue;
			}

			for (auto &&t : s->get_successors())
				if (closed.find(t) == closed.end())
				{
					t->set_parent(s.get());
					open.push(std::move(t));
				}

			closed.insert(std::move(s));
		}
		stop_time  = std::chrono::steady_clock::now();
	}

	/**
	 * @brief get_number_of_solutions
	 * @return number of found solutions
	 */
	size_t get_number_of_solutions() const
	{
		return solutions.size();
	}

	/**
	 * @brief get_solution_path
	 * @param solution_num Number of solution (0 - first, 1 - second and so on).
	 * Throws std::out_of_range if the number is invalid.
	 * @return path from selected solution to initial node.
	 */
	std::vector<const graph_state<score_type>*> get_solution_path(size_t solution_num) const
	{
		std::vector<const graph_state<score_type>*> path;
		auto state = get_solution(solution_num);
		while (state != nullptr)
		{
			path.push_back(state);
			state = state->get_parent();
		}
		return path;
	}

	/**
	 * @brief get_solution
	 * @param solution_num Number of solution (0 - first, 1 - second and so on).
	 * Throws std::out_of_range if the number is invalid.
	 * @return Pointer to selected solution.
	 */
	const graph_state<score_type>* get_solution(size_t solution_num) const
	{
		return solutions.at(solution_num).get();
	}

	/**
	 * @brief get_closed
	 * @return Reference to "Closed" set.
	 */
	const std::unordered_set<std::unique_ptr<graph_state<score_type>>>& get_closed() const
	{
		return closed;
	}

	/**
	 * @brief get_open
	 * @return Reference to "Open" set.
	 */
	const updatable_priority_queue<graph_state<score_type>, std::function<bool(const graph_state<score_type>&, const graph_state<score_type>&)>>& get_open() const
	{
		return open;
	}

	template<typename T=std::chrono::milliseconds>
	int64_t get_elapsed_time() const
	{
		return std::chrono::duration_cast<T>(stop_time - start_time).count();
	}

	/**
	 * @brief get_stats
	 * @return String containing some pieces of information about performed
	 * search.
	 */
	std::string get_stats() const
	{
		std::ostringstream out;
		out << "elapsed time: " << parse_time(get_elapsed_time<std::chrono::milliseconds>()) << std::endl;
		out << "number of solutions: " << solutions.size() << std::endl;
		out << "#closed: " << closed.size() << ", #open: " << open.size() << std::endl;
		return out.str();
	}

private:
	std::unordered_set<std::unique_ptr<graph_state<score_type>>> closed;
	updatable_priority_queue<graph_state<score_type>, std::function<bool(const graph_state<score_type>&, const graph_state<score_type>&)>> open;
	std::vector<std::unique_ptr<graph_state<score_type>>> solutions;
	std::chrono::steady_clock::time_point start_time, stop_time;
};

template<typename score_type>
bool default_f_compare(const graph_state<score_type> &a, const graph_state<score_type> &b)
{
	return a.get_f() < b.get_f();
}

template<typename score_type>
bool default_g_compare(const graph_state<score_type> &a, const graph_state<score_type> &b)
{
	return a.get_g() < b.get_g();
}

template<typename score_type>
bool default_h_compare(const graph_state<score_type> &a, const graph_state<score_type> &b)
{
	return a.get_h() < b.get_h();
}

#endif
