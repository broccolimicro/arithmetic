#pragma once

#include <common/standard.h>

namespace arithmetic
{

// TODO(edward.bingham) Built-in Data Types
// wire, int, struct
// in a struct, each member has it's own validity information.

// This structure represents a delay insensitive encoded integer value with a
// single neutral state. This is purposefully limited for now to keep the CHP
// language and simulator simple to implement.
struct value
{
	value();
	value(int data);
	~value();

	// At least one signal in this value is unstable or the encoding is in an
	// illegal state.
	const static int unstable = 0x80000000;
	// The value is not currently known.
	const static int unknown  = 0x80000001;
	// neutral state in a delay insensitive encoding. This is used to represent
	// "false" in guard expressions.
	const static int neutral  = 0x80000002;
	// The value encodes a valid integer value if it is greater than
	// or value::valid or Vdd if it is equal to value::valid. This is used to
	// represent "true" in guard expressions.
	const static int valid	  = 0x80000003;

	int data;

	bool is_subset_of(value v) const;

	operator bool() const;
};

ostream &operator<<(ostream &os, value v);

// boolean NOT using neutral as false and any valid value as true
value operator~(value v);
value operator-(value v);
value is_valid(value v);
// bitwise NOT
value operator!(value v);
// bitwise AND, OR, and XOR
value operator||(value v0, value v1);
value operator&&(value v0, value v1);
value operator^(value v0, value v1);
value operator<<(value v0, value v1);
value operator>>(value v0, value v1);
value operator+(value v0, value v1);
value operator-(value v0, value v1);
value operator*(value v0, value v1);
value operator/(value v0, value v1);
value operator%(value v0, value v1);

// boolean equality operators return neutral as false and a 0-bit
// valid (representing the value 0) as true.
value operator==(value v0, value v1);
value operator!=(value v0, value v1);
value operator<(value v0, value v1);
value operator>(value v0, value v1);
value operator<=(value v0, value v1);
value operator>=(value v0, value v1);

// boolean AND and OR using neutral as false and any valid value as true.
value operator&(value v0, value v1);
value operator|(value v0, value v1);

// set operators of the lattice documented in value::is_subset_of()
value intersect(value v0, value v1);
value union_of(value v0, value v1);

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

	int size() const;
	void extendX(int num);
	void extendN(int num);
	void extendU(int num);
	void trunk(int size);
	void clear();
	void push_back(value v);

	value get(int uid) const;
	void set(int uid, value v);
	void sv_intersect(int uid, value v);

	bool is_subset_of(const state &s) const;

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
