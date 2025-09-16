#pragma once

#include "expression.h"

namespace arithmetic
{

struct Action {
	Action();
	Action(Expression rvalue);
	Action(Expression lvalue, Expression rvalue);
	~Action();

	Expression lvalue;
	Expression rvalue;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	void applyVars(const Mapping<size_t> &m);
	void applyVars(const Mapping<int> &m);
	
	void evaluate(State &next, const State &curr, TypeSet types=TypeSet());
};

bool areSame(Action a0, Action a1);

ostream &operator<<(ostream &os, const Action &a);

struct Parallel {
	Parallel();
	Parallel(Expression expr);
	Parallel(Expression lvalue, Expression rvalue);
	Parallel(std::initializer_list<Action> exprs);
	~Parallel();

	vector<Action> actions;

	vector<Action>::iterator begin();
	vector<Action>::iterator end();
	vector<Action>::const_iterator begin() const;
	vector<Action>::const_iterator end() const;

	Action &operator[](int index);
	const Action &operator[](int index) const;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	State evaluate(const State &curr, TypeSet types=TypeSet());
	Expression guard();

	void applyVars(const Mapping<size_t> &m);
	void applyVars(const Mapping<int> &m);

	Parallel &operator&=(const Action &c0);
	Parallel &operator&=(const Parallel &c0);
};

bool areSame(Parallel p0, Parallel p1);

ostream &operator<<(ostream &os, const Parallel &p);

struct Choice {
	Choice();
	Choice(bool skip);
	Choice(std::initializer_list<Parallel> exprs);
	~Choice();

	vector<Parallel> terms;

	vector<Parallel>::iterator begin();
	vector<Parallel>::iterator end();
	vector<Parallel>::const_iterator begin() const;
	vector<Parallel>::const_iterator end() const;

	Parallel &operator[](int index);
	const Parallel &operator[](int index) const;

	bool isInfeasible() const;
	bool isVacuous() const;
	bool isPassive() const;

	Region evaluate(const State &curr, TypeSet types=TypeSet());
	Expression guard();

	void applyVars(const Mapping<size_t> &m);
	void applyVars(const Mapping<int> &m);

	Choice &operator&=(const Action &c0);
	Choice &operator&=(const Parallel &c0);
	Choice &operator&=(const Choice &c0);
	Choice &operator|=(const Action &c0);
	Choice &operator|=(const Parallel &c0);
	Choice &operator|=(const Choice &c0);
};

bool areSame(Choice c0, Choice c1);

ostream &operator<<(ostream &os, const Choice &c);

Parallel operator&(const Action &c0, const Action &c1);
Parallel operator&(const Action &c0, const Parallel &c1);
Parallel operator&(Parallel c0, const Action &c1);
Parallel operator&(Parallel c0, const Parallel &c1);

Choice operator|(const Action &c0, const Action &c1);
Choice operator|(const Action &c0, const Parallel &c1);
Choice operator|(const Parallel &c0, const Action &c1);
Choice operator|(const Parallel &c0, const Parallel &c1);

Choice operator&(const Action &c0, const Choice &c1);
Choice operator&(Choice c0, const Action &c1);
Choice operator&(const Parallel &c0, const Choice &c1);
Choice operator&(Choice c0, const Parallel &c1);
Choice operator&(const Choice &c0, const Choice &c1);

Choice operator|(const Action &c0, const Choice &c1);
Choice operator|(Choice c0, const Action &c1);
Choice operator|(const Parallel &c0, const Choice &c1);
Choice operator|(Choice c0, const Parallel &c1);
Choice operator|(Choice c0, const Choice &c1);

}

