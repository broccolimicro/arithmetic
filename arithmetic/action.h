#pragma once

#include "expression.h"

namespace arithmetic
{

struct action
{
	action();
	action(expression expr);
	action(int variable, expression expr);
	action(int channel, int variable, expression expr);
	action(int channel, expression expr, int variable);
	~action();

	int variable;
	int channel;
	expression expr;
	int behavior;

	enum
	{
		assign = 0,
		send = 1,
		receive = 2
	};

	bool is_infeasible() const;
	bool is_vacuous() const;
	bool is_passive() const;

	void apply(vector<int> uid_map);
};

struct parallel
{
	parallel();
	parallel(expression expr);
	parallel(int variable, expression expr);
	parallel(int channel, int variable, expression expr);
	parallel(int channel, expression expr, int variable);
	~parallel();

	vector<action> actions;

	action &operator[](int index);
	const action &operator[](int index) const;

	bool is_infeasible() const;
	bool is_vacuous() const;
	bool is_passive() const;

	state evaluate(const state &curr);
	expression guard();

	void apply(vector<int> uid_map);
};

ostream &operator<<(ostream &os, const parallel &p);

struct choice
{
	choice();
	choice(parallel c);
	~choice();

	vector<parallel> terms;

	parallel &operator[](int index);
	const parallel &operator[](int index) const;

	bool is_infeasible() const;
	bool is_vacuous() const;
	bool is_passive() const;

	region evaluate(const state &curr);
	expression guard();

	void apply(vector<int> uid_map);
};

ostream &operator<<(ostream &os, const choice &c);

}

