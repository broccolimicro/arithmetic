#pragma once

#include <common/standard.h>
#include <common/interface.h>
#include "type.h"

namespace arithmetic
{

// This structure represents a delay insensitive encoded integer
// value with a single neutral state. This is purposefully limited
// for now to keep the CHP language and simulator simple to
// implement.
struct Value
{
	enum ValType : int {
		// can only be used to dereference a member of a structure
		STRING = -5,
		BOOL = -4,
		// INT and REAL are both considered "VALID"
		INT = -3,
		REAL = -2,
		// ARRAY and STRUCT have separate validities for each value
		ARRAY = -1,
		// By default, all operators on STRUCTs behave like operators on ARRAYs
		// anything with a type >= 0 is a struct and the type stores the ID of
		// the struct type in the type system
		STRUCT = 0
	};

	Value();
	Value(bool bval);
	Value(int64_t ival);
	Value(int ival);
	Value(double rval);
	Value(string sval);
	~Value();

	ValType type;

	enum Bval : int16_t {
		// used by "bval"
		// At least one signal in this value is unstable or the encoding is in an
		// illegal state.
		UNSTABLE = -1,
		// neutral state in a delay insensitive encoding. This is used to represent
		// "false" in guard expressions.
		NEUTRAL  = 0,
		// The value encodes a valid integer value if it is greater than
		// or Value::valid or Vdd if it is equal to Value::valid. This is used to
		// represent "true" in guard expressions.	
		VALID    = 1,
		// The value is not currently known.
		UNKNOWN  = 2
	};

	union {
		Bval bval;
		int64_t ival;
		double rval;
	};
	string sval;

	vector<Value> arr;

	bool isValid() const;
	bool isNeutral() const;
	bool isUnstable() const;
	bool isUnknown() const;
	const char *ctypeName() const;
	string typeName() const;

	static Value X();
	static Value U();
	static Value stringOf(string sval);
	static Value boolOf(bool valid);
	static Value intOf(int64_t ival);
	static Value realOf(double rval);
	static Value arrOf(vector<Value> arr);
	static Value structOf(vector<Value> arr);	

	bool isSubsetOf(Value v) const;

	operator bool() const;

	Type typeOf() const;
};

bool areSame(Value v0, Value v1);
int order(Value v0, Value v1);

ostream &operator<<(ostream &os, Value v);

// boolean NOT using neutral as false and any valid value as true
Value operator~(Value v);
Value operator-(Value v);
Value valid(Value v);
// bitwise NOT
Value operator!(Value v);
Value inv(Value v);
// bitwise AND, OR, and XOR
Value operator||(Value v0, Value v1);
Value operator&&(Value v0, Value v1);
Value operator^(Value v0, Value v1);
Value operator<<(Value v0, Value v1);
Value operator>>(Value v0, Value v1);
Value operator+(Value v0, Value v1);
Value operator-(Value v0, Value v1);
Value operator*(Value v0, Value v1);
Value operator/(Value v0, Value v1);
Value operator%(Value v0, Value v1);

// boolean equality operators return neutral as false and a 0-bit
// valid (representing the Value 0) as true.
Value operator==(Value v0, Value v1);
Value operator!=(Value v0, Value v1);
Value operator<(Value v0, Value v1);
Value operator>(Value v0, Value v1);
Value operator<=(Value v0, Value v1);
Value operator>=(Value v0, Value v1);

// boolean AND and OR using neutral as false and any valid value as true.
Value operator&(Value v0, Value v1);
Value operator|(Value v0, Value v1);

Value stringOf(Value v);
Value boolOf(Value v);
Value realOf(Value v);
Value intOf(Value v);
Value index(Value v, Value i);

_CONST_INTERFACE_ARG(TypeSet,
	(int, memberIndex, (Value::ValType type, string name) const, (type, name)));

Value member(Value v0, Value v1, TypeSet types);

// set operators of the lattice documented in Value::isSubsetOf()
Value intersect(Value v0, Value v1);
Value unionOf(Value v0, Value v1);

}
