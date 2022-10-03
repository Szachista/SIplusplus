#ifndef GRAPH_STATE_HPP
#define GRAPH_STATE_HPP

#include <vector>
#include <memory>
#include <string>
#include <iostream>

template<typename score_type>
class graph_state
{
public:
	graph_state()
		: parent(nullptr)
	{
	}

	virtual ~graph_state() = default;

	/**
	 * @brief Method creates copy of the current object.
	 * @return Unique pointer to the copied object.
	 */
	virtual std::unique_ptr<graph_state> clone() const = 0;

	/**
	 * @brief Method calculates hash of the current object.
	 * @return Number representing hash.
	 */
	virtual std::size_t hash_code() const = 0;

	/**
	 * @brief get_successors
	 * @return vector of unique pointers containing children states.
	 */
	virtual std::vector<std::unique_ptr<graph_state>> get_successors() const = 0;

	/**
	 * @brief is_solution
	 * @return true if current state is a terminal node, false otherwise.
	 */
	virtual bool is_solution() const = 0;

	/**
	 * @brief Method creates string representation of the current state.
	 * @return string containing character representation.
	 */
	virtual std::string to_string() const = 0;

	bool operator== (const graph_state &s) const
	{
		return is_equal(s);
	}

	/**
	 * @brief get_f
	 * @return sum of true distance from initial node and estimated distance to
	 * terminal node.
	 */
	virtual score_type get_f() const = 0;

	/**
	 * @brief get_g
	 * @return true distance from initial node.
	 */
	virtual score_type get_g() const = 0;

	/**
	 * @brief get_h
	 * @return estimated distance to terminal node; must be non-negative.
	 */
	virtual score_type get_h() const = 0;

	/**
	 * @brief Method updates f, g and h of the current state.
	 * @param Actual distance from inital node to current state.
	 */
	virtual void update_score(score_type) = 0;

	/**
	 * @brief get_parent
	 * @return Pointer to the parent or nullptr if it is initial node.
	 */
	const graph_state* get_parent() const
	{
		return this->parent;
	}

	/**
	 * @brief set_parent
	 * @param parent Pointer to the parent.
	 */
	void set_parent(const graph_state *parent)
	{
		this->parent = parent;
	}

protected:
	/**
	 * @brief Method computes estimated distance to terminal node. Returned
	 * value must be non-negative.
	 * @return estimated distance to terminal node.
	 */
	virtual score_type get_heuristic_grade() const = 0;

	/**
	 * @brief Method compares state with the other state.
	 * @param s state to compare with.
	 * @return true when current state is the same as state s, false otherwise.
	 */
	virtual bool is_equal(const graph_state &s) const = 0;

private:
	const graph_state *parent;
};

namespace std
{
	template<typename score_type>
	struct hash<graph_state<score_type>*>
	{
		size_t operator()(const graph_state<score_type>* s) const
		{
			return s->hash_code();
		}
	};

	template<typename score_type>
	struct equal_to<graph_state<score_type>*>
	{
		bool operator()(const graph_state<score_type>* a,
						const graph_state<score_type>* b) const
		{
			return *a == *b;
		}
	};

	template<typename score_type>
	struct equal_to<std::unique_ptr<graph_state<score_type>>>
	{
		bool operator()(const std::unique_ptr<graph_state<score_type>> &a,
						const std::unique_ptr<graph_state<score_type>> &b) const
		{
			return a && b && *a == *b;
		}
	};

	template<typename score_type>
	ostream& operator << (ostream& out, const graph_state<score_type> &s);
}

template<typename score_type>
class g_state : public graph_state<score_type>
{
public:
	g_state()
		: gscore{}
	{
	}

	virtual ~g_state() = default;

	score_type get_f() const override { return gscore; }

	score_type get_g() const override { return gscore; }

	score_type get_h() const override { return score_type{}; }

	void update_score(score_type new_g) override { gscore = new_g; }

protected:
	score_type get_heuristic_grade() const override { return score_type{}; }

private:
	score_type gscore;
};

template<typename score_type>
class h_state : public graph_state<score_type>
{
public:
	h_state()
		: hscore{}
	{
	}

	virtual ~h_state() = default;

	score_type get_f() const override { return hscore; }

	score_type get_g() const override { return score_type{}; }

	score_type get_h() const override { return hscore; }

	void update_score(score_type) override { hscore = this->get_heuristic_grade(); }

private:
	score_type hscore;
};

template<typename score_type>
class f_state : public graph_state<score_type>
{
public:
	f_state()
		: gscore{},
		  fscore{}
	{
	}

	virtual ~f_state() = default;

	score_type get_f() const override { return fscore; }

	score_type get_g() const override { return gscore; }

	score_type get_h() const override { return fscore - gscore; }

	void update_score(score_type new_g) override
	{
		gscore = new_g;
		fscore = gscore + this->get_heuristic_grade();
	}

private:
	score_type gscore, fscore;
};

#endif
