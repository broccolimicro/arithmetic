#pragma once

#include <common/standard.h>

#include "value.h"

namespace arithmetic
{

// States operate outside of the delay-insensitive value space. They
// are used by the simulator to keep track of the current value of
// each variable.
struct state
{
	state();
	state(int uid, value v);
	~state();

	// Each value at index i represents the current value of the
	// variable at index i in a ucs::variable_set structure.
	vector<value> values;

	size_t size() const;
	void extendX(int num);
	void extendN(int num);
	void extendU(int num);
	void trunk(int size);
	void clear();
	void push_back(value v);

	value get(int uid) const;
	void set(int uid, value v);
	void sv_intersect(int uid, value v);

	bool isSubsetOf(const state &s) const;

	state &operator&=(state s);
	state &operator|=(state s);
	state &operator=(state s);

	value &operator[](int uid);
	value operator[](int uid) const;

	state remote(vector<vector<int> > groups);

	bool is_tautology() const;

	state mask() const;
	state mask(const state &m) const;
	state combine_mask(const state &m) const;

	void apply(vector<int> uid_map);
};

ostream &operator<<(ostream &os, const state &s);

bool operator==(state s0, state s1);
bool operator!=(state s0, state s1);
bool operator<(state s0, state s1);
bool operator>(state s0, state s1);
bool operator<=(state s0, state s1);
bool operator>=(state s0, state s1);

// Set operators intersect and union
state operator&(state s0, state s1);
state operator|(state s0, state s1);

state local_assign(state s0, state s1, bool stable);
state remote_assign(state s0, state s1, bool stable);
bool vacuous_assign(const state &encoding, const state &assignment, bool stable);

bool are_interfering(const state &s0, const state &s1);
state interfere(state s0, const state &s1);

struct region {
	region();
	~region();

	vector<state> states;

	region remote(vector<vector<int> > groups);
	
	bool is_tautology() const;

	state &operator[](int idx);
	state operator[](int idx) const;

	void apply(vector<int> uid_map);
};

bool vacuous_assign(const state &encoding, const region &assignment, bool stable);

}
