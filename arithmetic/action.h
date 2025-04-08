#pragma once

#include "expression.h"

namespace arithmetic
{

struct action
{
	action();
	action(Expression expr);
	action(int variable, Expression expr);
	action(int channel, int variable, Expression expr);
	action(int channel, Expression expr, int variable);
	~action();

	int variable;
	int channel;
	Expression expr;
	int behavior;

	enum
	{
		ASSIGN  = 0,
		SEND    = 1,
		RECEIVE = 2
	};

	bool is_infeasible() const;
	bool is_vacuous() const;
	bool is_passive() const;

	void apply(vector<int> uid_map);
};

struct parallel
{
	parallel();
	parallel(Expression expr);
	parallel(int variable, Expression expr);
	parallel(int channel, int variable, Expression expr);
	parallel(int channel, Expression expr, int variable);
	~parallel();

	vector<action> actions;

	action &operator[](int index);
	const action &operator[](int index) const;

	bool is_infeasible() const;
	bool is_vacuous() const;
	bool is_passive() const;

	state evaluate(const state &curr);
	Expression guard();

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
	Expression guard();

	void apply(vector<int> uid_map);
};

ostream &operator<<(ostream &os, const choice &c);

}

