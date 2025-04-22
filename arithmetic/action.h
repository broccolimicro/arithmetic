#pragma once

#include "expression.h"

namespace arithmetic
{

struct Action {
	Action();
	Action(Expression expr);
	Action(int variable, Expression expr);
	~Action();

	int variable;
	Expression expr;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	void apply(vector<int> uidMap);
};

struct Parallel {
	Parallel();
	Parallel(Expression expr);
	Parallel(int variable, Expression expr);
	~Parallel();

	vector<Action> actions;

	Action &operator[](int index);
	const Action &operator[](int index) const;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	State evaluate(const State &curr);
	Expression guard();

	void apply(vector<int> uidMap);
};

ostream &operator<<(ostream &os, const Parallel &p);

struct Choice {
	Choice();
	Choice(Parallel c);
	~Choice();

	vector<Parallel> terms;

	Parallel &operator[](int index);
	const Parallel &operator[](int index) const;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	Region evaluate(const State &curr);
	Expression guard();

	void apply(vector<int> uidMap);
};

ostream &operator<<(ostream &os, const Choice &c);

}

