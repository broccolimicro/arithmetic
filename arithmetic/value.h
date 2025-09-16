#pragma once

#include <common/standard.h>
#include <common/interface.h>
#include "type.h"

namespace arithmetic
{

struct LValue {
	LValue(size_t index=std::numeric_limits<size_t>::max(), std::initializer_list<size_t> mods={});
	~LValue();

	size_t index;

	// The stack of modifiers to apply.
	// The last modifer in the stack
	// should be applied first and so
	// on.
	vector<size_t> mods;

	bool isUndef();
};

// This structure represents a delay insensitive encoded integer
// value with a single neutral state. This is purposefully limited
// for now to keep the CHP language and simulator simple to
// implement.
struct Value {
	enum ValType : int32_t {
		UNDEF = -7,
		WIRE = -6,
		// can only be used to dereference a member of a structure
		STRING = -5,
		BOOL = -4,
		// INT and REAL are both considered "VALID"
		INT = -3,
		REAL = -2,
		// ARRAY and STRUCT have separate validities for each value
		ARRAY = -1,
		// By default, all operators on STRUCTs behave like operators on ARRAYs
		// arr stores all of the members
		// sval stores the name of the structure type for lookup
		STRUCT = 0
	};

	enum StateType : int8_t {
		// At least one signal in this value is unstable or the encoding is in an
		// illegal state.
		UNSTABLE = 0,
		// neutral state in a delay insensitive encoding.
		NEUTRAL  = 1,
		// dataless valid (vdd)
		VALID    = 2,
		// The value is not currently known.
		UNKNOWN  = 3
	};

	Value();
	Value(bool bval);
	Value(int64_t ival);
	Value(int ival);
	Value(double rval);
	Value(string sval);
	~Value();

	ValType type;
	StateType state;
	union {
		bool bval;
		int64_t ival;
		double rval;
	};
	string sval;

	vector<Value> arr;

	bool isValid() const;
	bool isNeutral() const;
	bool isUnstable() const;
	bool isUnknown() const;
	bool isTrue() const;
	const char *ctypeName() const;
	string typeName() const;

	static Value X(ValType type=WIRE);
	static Value U(ValType type=WIRE);
	static Value gnd(ValType type=WIRE);
	static Value vdd();
	static Value stringOf(string sval);
	static Value boolOf(bool valid);
	static Value intOf(int64_t ival);
	static Value realOf(double rval);
	static Value arrOf(vector<Value> arr);
	static Value structOf(string name, vector<Value> arr);	

	bool isSubsetOf(Value v) const;

	operator bool() const;

	Type typeOf() const;

	Value *at(vector<size_t> mods, bool init=false);
	const Value *at(vector<size_t> mods) const;
};

bool areSame(Value v0, Value v1);
int order(Value v0, Value v1);

ostream &operator<<(ostream &os, Value v);

Value isTrue(Value v); // return a wire which is "valid" when the value is "true" and "neutral" otherwise
Value isValid(Value v);
	

// wire NOT using neutral as false and any valid value as true
Value operator~(Value v);
// boolean NOT
Value operator!(Value v);
Value operator-(Value v);
Value inv(Value v);
// boolean AND, OR, and XOR
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

// wire AND and OR using neutral as false and any valid value as true.
Value operator&(Value v0, Value v1);
Value operator|(Value v0, Value v1);

// typecast operators
Value stringOf(Value v);
Value wireOf(Value v); // compute validity
Value boolOf(Value v); // compute truthiness
Value realOf(Value v);
Value cast(Value::ValType type, Value v);
Value intOf(Value v);
Value index(Value v, Value i);
Value index(Value v, Value f, Value t);

_CONST_INTERFACE_ARG(TypeSet,
	(int, memberIndex, (string type, string name) const, (type, name)));

Value member(Value v0, Value v1, TypeSet types);

// set operators of the lattice documented in Value::isSubsetOf()
Value intersect(Value v0, Value v1);
Value unionOf(Value v0, Value v1);

}
