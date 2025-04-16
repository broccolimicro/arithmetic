#pragma once

#include <common/standard.h>
#include "type.h"

namespace arithmetic
{

// This structure represents a delay insensitive encoded integer
// value with a single neutral state. This is purposefully limited
// for now to keep the CHP language and simulator simple to
// implement.
struct value
{
	value();
	value(bool bval);
	value(int64_t ival);
	value(int ival);
	value(double rval);
	~value();

	// used by "type"
	enum {
		BOOL = -4,
		// INT and REAL are both considered "VALID"
		INT = -3,
		REAL = -2,
		// ARRAY and STRUCT have separate validities for each value
		ARRAY = -1,
		// By default, all operators on STRUCTs behave like operators on ARRAYs
		STRUCT = 0
	};

	int type;

	enum {
		// used by "bval"
		// At least one signal in this value is unstable or the encoding is in an
		// illegal state.
		UNSTABLE = -1,
		// neutral state in a delay insensitive encoding. This is used to represent
		// "false" in guard expressions.
		NEUTRAL  = 0,
		// The value encodes a valid integer value if it is greater than
		// or value::valid or Vdd if it is equal to value::valid. This is used to
		// represent "true" in guard expressions.	
		VALID    = 1,
		// The value is not currently known.
		UNKNOWN  = 2
	};

	union {
		int16_t bval;
		int64_t ival;
		double rval;
	};

	vector<value> arr;

	bool isValid() const;
	bool isNeutral() const;
	bool isUnstable() const;
	bool isUnknown() const;
	const char *ctypeName() const;
	string typeName() const;

	static value X();
	static value U();
	static value boolOf(bool valid);
	static value intOf(int64_t ival);
	static value realOf(double rval);
	static value arrOf(vector<value> arr);
	static value structOf(vector<value> arr);	

	bool isSubsetOf(value v) const;

	operator bool() const;

	Type typeOf() const;
};

bool areSame(value v0, value v1);
int order(value v0, value v1);

ostream &operator<<(ostream &os, value v);

// boolean NOT using neutral as false and any valid value as true
value operator~(value v);
value operator-(value v);
value valid(value v);
// bitwise NOT
value operator!(value v);
value inv(value v);
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

value boolOf(value v);
value realOf(value v);
value intOf(value v);
value index(value v, value i);

// set operators of the lattice documented in value::isSubsetOf()
value intersect(value v0, value v1);
value union_of(value v0, value v1);

}
