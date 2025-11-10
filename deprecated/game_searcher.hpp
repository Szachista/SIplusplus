#ifndef GAME_SEARCHER_HPP
#define GAME_SEARCHER_HPP

#include "game_state.hpp"
#include <unordered_set>
#include <iostream>
#include <chrono>

enum class tt_type : uint8_t
{
	unknown, exact_value, lower_bound, upper_bound
};

template<typename Move>
struct tt_entry
{
	std::unique_ptr<game_state<Move>> state;
	double score;
	uint8_t depth;
	tt_type type;

	tt_entry(tt_entry &&entry)
		: state(std::move(entry.state)),
		  score(entry.score),
		  depth(entry.depth),
		  type(entry.type)
	{
	}

	tt_entry()
		: score(0),
		  depth(0),
		  type(tt_type::unknown)
	{
	}

	tt_entry(std::unique_ptr<game_state<Move>> &&state,
			 double score,
			 uint8_t depth,
			 tt_type type)
		: state(std::move(state)),
		  score(score),
		  depth(depth),
		  type(type)
	{
	}
};

namespace std {
	template<typename Move>
	struct hash<tt_entry<Move>>
	{
		size_t operator()(const tt_entry<Move> &t) const
		{
			return hash<std::unique_ptr<game_state<Move>>>{}(t.state);
		}
	};

	template<typename Move>
	struct equal_to<tt_entry<Move>>
	{
		bool operator()(const tt_entry<Move> &s, const tt_entry<Move> &t) const
		{
			return equal_to<std::unique_ptr<game_state<Move>>>{}(s.state, t.state);
		}
	};
}

template<typename Move, bool use_tt = true>
class alpha_beta_searcher
{
public:
	alpha_beta_searcher(unsigned search_depth, bool maximize, unsigned max_depth = 32)
		: search_depth(search_depth),
		  maximize(maximize),
		  max_depth(max_depth)
	{
		if (search_depth > max_depth)
			throw std::logic_error("search_depth > max_depth");
	}

	void do_search(const game_state<Move> &state)
	{
		scores.clear();
		transposition_table.clear();
		num_visited = 0;
		hash_hits = 0;
		if (maximize)
			alpha_beta_max(state.clone());
		else
			alpha_beta_min(state.clone());
	}

	const std::vector<std::pair<Move, double>>& get_scores() const
	{
		return scores;
	}

	const std::unordered_set<tt_entry<Move>>& get_transposition_table() const
	{
		return transposition_table;
	}

	std::size_t get_number_of_visited_states() const
	{
		return num_visited;
	}

	std::size_t get_number_of_hash_hits() const
	{
		return hash_hits;
	}

private:
	const unsigned search_depth;
	const bool maximize;
	const unsigned max_depth;
	std::vector<std::pair<Move, double>> scores;
	std::unordered_set<tt_entry<Move>> transposition_table;
	tt_entry<Move> dummy_entry;
	std::size_t num_visited, hash_hits;

	double alpha_beta_max(std::unique_ptr<game_state<Move>> &&state,
						  unsigned d = 0,
						  double alpha = -std::numeric_limits<double>::infinity(),
						  double beta = std::numeric_limits<double>::infinity())
	{
		num_visited++;
		if constexpr (use_tt)
		{
			dummy_entry.state = std::move(state);
			auto hash_hits = transposition_table.find(alpha_beta_searcher::dummy_entry);
			state = std::move(dummy_entry.state);
			if (hash_hits != transposition_table.end() && hash_hits->depth >= d)
			{
				this->hash_hits++;
				switch (hash_hits->type)
				{
				case tt_type::exact_value:
					return hash_hits->score;
				case tt_type::lower_bound:
					if (hash_hits->score >= beta)
						return hash_hits->score;
					break;
				case tt_type::upper_bound:
					if (alpha >= hash_hits->score)
						return hash_hits->score;
					break;
				case tt_type::unknown:
					throw std::runtime_error("Invalid tt_type");
				}
			}
		}

		auto val = state->is_terminal();
		if (val)
		{
			if constexpr (use_tt)
				transposition_table.emplace(tt_entry<Move>(std::move(state),
														   val.value(),
														   d,
														   tt_type::exact_value));
			return val.value();
		}

		if (d == search_depth)
		{
//			double score = state->get_h();
			double score = quiescence_max(std::move(state), d, alpha, beta);
			if constexpr (use_tt)
				transposition_table.emplace(tt_entry<Move>(std::move(state),
														   score,
														   d,
														   tt_type::exact_value));
			return score;
		}

		for (const auto &m : state->generate_moves())
		{
			auto s = state->make_move(m);
			if (!s)
				continue;

			double score;
			if (d == 0)
			{	// to acquire exact score of the move
				score = alpha_beta_min(std::move(s), d + 1);
				scores.emplace_back(std::make_pair(m, score));
			}
			else
				score = alpha_beta_min(std::move(s), d + 1, alpha, beta);

			alpha = std::max(alpha, score);
			if (alpha >= beta)
			{
				if constexpr (use_tt)
					transposition_table.emplace(tt_entry<Move>(std::move(state),
															   alpha,
															   d,
															   tt_type::lower_bound));
				return alpha;
			}
		}

		if constexpr (use_tt)
			transposition_table.emplace(tt_entry<Move>(std::move(state),
													   alpha,
													   d,
													   tt_type::exact_value));

		return alpha;
	}

	double alpha_beta_min(std::unique_ptr<game_state<Move>> &&state,
						  unsigned d = 0,
						  double alpha = -std::numeric_limits<double>::infinity(),
						  double beta = std::numeric_limits<double>::infinity())
	{
		num_visited++;
		if constexpr (use_tt)
		{
			dummy_entry.state = std::move(state);
			auto hash_hits = transposition_table.find(alpha_beta_searcher::dummy_entry);
			state = std::move(dummy_entry.state);
			if (hash_hits != transposition_table.end() && hash_hits->depth >= d)
			{
				this->hash_hits++;
				switch (hash_hits->type)
				{
				case tt_type::exact_value:
					return hash_hits->score;
				case tt_type::lower_bound:
					if (hash_hits->score >= beta)
						return hash_hits->score;
					break;
				case tt_type::upper_bound:
					if (alpha >= hash_hits->score)
						return hash_hits->score;
					break;
				case tt_type::unknown:
					throw std::runtime_error("Invalid tt_type");
				}
			}
		}

		auto val = state->is_terminal();
		if (val)
		{
			if constexpr (use_tt)
				transposition_table.emplace(tt_entry<Move>(std::move(state),
														   val.value(),
														   d,
														   tt_type::exact_value));
			return val.value();
		}

		if (d == search_depth)
		{
//			double score = state->get_h();
			double score = quiescence_min(std::move(state), d, alpha, beta);
			if constexpr (use_tt)
				transposition_table.emplace(tt_entry<Move>(std::move(state),
														   score,
														   d,
														   tt_type::exact_value));
			return score;
		}

		for (const auto &m : state->generate_moves())
		{
			auto s = state->make_move(m);
			if (!s)
				continue;

			double score;
			if (d == 0)
			{	// to acquire exact score of the move
				score = alpha_beta_max(std::move(s), d + 1);
				scores.emplace_back(std::make_pair(m, score));
			}
			else
				score = alpha_beta_max(std::move(s), d + 1, alpha, beta);

			beta = std::min(beta, score);
			if (alpha >= beta)
			{
				if constexpr (use_tt)
					transposition_table.emplace(tt_entry<Move>(std::move(state),
															   beta,
															   d,
															   tt_type::upper_bound));
				return beta;
			}
		}

		if constexpr (use_tt)
			transposition_table.emplace(tt_entry<Move>(std::move(state),
													   beta,
													   d,
													   tt_type::exact_value));

		return beta;
	}

	double quiescence_max(std::unique_ptr<game_state<Move>> &&state,
						  unsigned d = 0,
						  double alpha = -std::numeric_limits<double>::infinity(),
						  double beta = std::numeric_limits<double>::infinity())
	{
//		num_visited++;
//		if constexpr (use_tt)
//		{
//			dummy_entry.state = std::move(state);
//			auto hash_hits = transposition_table.find(alpha_beta_searcher::dummy_entry);
//			state = std::move(dummy_entry.state);
//			if (hash_hits != transposition_table.end() && hash_hits->depth >= d)
//			{
//				this->hash_hits++;
//				switch (hash_hits->type)
//				{
//				case tt_type::exact_value:
//					return hash_hits->score;
//				case tt_type::lower_bound:
//					if (hash_hits->score >= beta)
//						return hash_hits->score;
//					break;
//				case tt_type::upper_bound:
//					if (alpha >= hash_hits->score)
//						return hash_hits->score;
//					break;
//				case tt_type::unknown:
//					throw std::runtime_error("Invalid tt_type");
//				}
//			}
//		}

		auto val = state->is_terminal();
		if (val)
		{
//			if constexpr (use_tt)
//				transposition_table.emplace(tt_entry<Move>(std::move(state),
//														   val.value(),
//														   d,
//														   tt_type::exact_value));
			return val.value();
		}

		auto moves_list = state->generate_non_quiet_moves();
		if (moves_list.empty() /*|| d == max_depth*/)
		{
			double score = state->get_h();
//			if constexpr (use_tt)
//				transposition_table.emplace(tt_entry<Move>(std::move(state),
//														   score,
//														   d,
//														   tt_type::exact_value));
			return score;
		}

		for (const auto &m : moves_list)
		{
			auto s = state->make_move(m);
			if (!s)
				continue;

			double score = quiescence_min(std::move(s), d + 1, alpha, beta);

			alpha = std::max(alpha, score);
			if (alpha >= beta)
			{
//				if constexpr (use_tt)
//					transposition_table.emplace(tt_entry<Move>(std::move(state),
//															   alpha,
//															   d,
//															   tt_type::lower_bound));
				return alpha;
			}
		}

//		if constexpr (use_tt)
//			transposition_table.emplace(tt_entry<Move>(std::move(state),
//													   alpha,
//													   d,
//													   tt_type::exact_value));

		return alpha;
	}

	double quiescence_min(std::unique_ptr<game_state<Move>> &&state,
						  unsigned d = 0,
						  double alpha = -std::numeric_limits<double>::infinity(),
						  double beta = std::numeric_limits<double>::infinity())
	{
//		num_visited++;
//		if constexpr (use_tt)
//		{
//			dummy_entry.state = std::move(state);
//			auto hash_hits = transposition_table.find(alpha_beta_searcher::dummy_entry);
//			state = std::move(dummy_entry.state);
//			if (hash_hits != transposition_table.end() && hash_hits->depth >= d)
//			{
//				this->hash_hits++;
//				switch (hash_hits->type)
//				{
//				case tt_type::exact_value:
//					return hash_hits->score;
//				case tt_type::lower_bound:
//					if (hash_hits->score >= beta)
//						return hash_hits->score;
//					break;
//				case tt_type::upper_bound:
//					if (alpha >= hash_hits->score)
//						return hash_hits->score;
//					break;
//				case tt_type::unknown:
//					throw std::runtime_error("Invalid tt_type");
//				}
//			}
//		}

		auto val = state->is_terminal();
		if (val)
		{
//			if constexpr (use_tt)
//				transposition_table.emplace(tt_entry<Move>(std::move(state),
//														   val.value(),
//														   d,
//														   tt_type::exact_value));
			return val.value();
		}

		auto moves_list = state->generate_non_quiet_moves();
		if (moves_list.empty() /*|| d == max_depth*/)
		{
			double score = state->get_h();
//			if constexpr (use_tt)
//				transposition_table.emplace(tt_entry<Move>(std::move(state),
//														   score,
//														   d,
//														   tt_type::exact_value));
			return score;
		}

		for (const auto &m : moves_list)
		{
			auto s = state->make_move(m);
			if (!s)
				continue;

			double score = quiescence_max(std::move(s), d + 1, alpha, beta);

			beta = std::min(beta, score);
			if (alpha >= beta)
			{
//				if constexpr (use_tt)
//					transposition_table.emplace(tt_entry<Move>(std::move(state),
//															   beta,
//															   d,
//															   tt_type::upper_bound));
				return beta;
			}
		}

//		if constexpr (use_tt)
//			transposition_table.emplace(tt_entry<Move>(std::move(state),
//													   beta,
//													   d,
//													   tt_type::exact_value));

		return beta;
	}
};

#endif
