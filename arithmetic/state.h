#pragma once

#include <common/standard.h>

#include "value.h"

namespace arithmetic
{

// States operate outside of the delay-insensitive value space. They
// are used by the simulator to keep track of the current value of
// each variable.
struct State
{
	State();
	State(int uid, Value v);
	~State();

	// Each value at index i represents the current value of the
	// variable at index i in a ucs::variable_set structure.
	vector<Value> values;

	size_t size() const;
	void extendX(int num);
	void extendN(int num);
	void extendU(int num);
	void trunk(int size);
	void clear();
	void push_back(Value v);

	Value get(int uid) const;
	void set(int uid, Value v);
	void svIntersect(int uid, Value v);

	bool isSubsetOf(const State &s) const;

	State &operator&=(State s);
	State &operator|=(State s);
	State &operator=(State s);

	Value &operator[](int uid);
	Value operator[](int uid) const;

	State remote(vector<vector<int> > groups);

	bool isTautology() const;

	State mask() const;
	State mask(const State &m) const;
	State combineMask(const State &m) const;

	void apply(vector<int> uidMap);
};

ostream &operator<<(ostream &os, const State &s);

bool operator==(State s0, State s1);
bool operator!=(State s0, State s1);
bool operator<(State s0, State s1);
bool operator>(State s0, State s1);
bool operator<=(State s0, State s1);
bool operator>=(State s0, State s1);

// Set operators intersect and union
State operator&(State s0, State s1);
State operator|(State s0, State s1);

State localAssign(State s0, State s1, bool stable);
State remoteAssign(State s0, State s1, bool stable);
bool vacuousAssign(const State &encoding, const State &assignment, bool stable);

bool areInterfering(const State &s0, const State &s1);
State interfere(State s0, const State &s1);

struct Region {
	Region();
	~Region();

	vector<State> states;

	Region remote(vector<vector<int> > groups);
	
	bool isTautology() const;

	State &operator[](int idx);
	State operator[](int idx) const;

	void apply(vector<int> uidMap);
};

bool vacuousAssign(const State &encoding, const Region &assignment, bool stable);

}
