#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

namespace arithmetic {

struct Mapping;

struct Operand {
	// Used by "type"
	enum Type {
		UNDEF  = -1,
		CONST  = 0,
		VAR    = 1,
		EXPR   = 2,
		TYPE   = 3,
	};

	Operand(Value v = Value());
	Operand(bool bval);
	Operand(int64_t ival);
	Operand(int ival);
	Operand(double rval);
	Operand(string sval);
	~Operand();

	Type type;

	// used for CONST
	Value cnst;

	// used for VAR and EXPR
	size_t index;

	bool isUndef() const;
	bool isConst() const;
	bool isExpr() const;
	bool isVar() const;
	bool isType() const;

	Value get(State values=State(), vector<Value> expressions=vector<Value>()) const;
	void set(State &values, vector<Value> &expressions, Value v) const;

	// Undefined
	static Operand undef();

	// Constants
	static Operand X();
	static Operand U();
	static Operand boolOf(bool bval);
	static Operand intOf(int64_t ival);
	static Operand realOf(double rval);
	static Operand arrOf(vector<Value> arr);
	static Operand structOf(string name, vector<Value> arr);
	static Operand stringOf(string sval);

	static Operand exprOf(size_t index);
	Operand &offsetExpr(int off);

	static Operand varOf(size_t index);
	Operand &apply(vector<int> varMap);
	
	static Operand typeOf(Type type);
};

ostream &operator<<(ostream &os, Operand o);

bool operator==(Operand o0, Operand o1);
bool operator!=(Operand o0, Operand o1);
bool operator<(Operand o0, Operand o1); // does not differentiate constants

struct Mapping {
	std::map<Operand, Operand> m;

	bool set(Operand o0, Operand o1);

	bool has(Operand o0) const;
	Operand map(Operand o0) const;
	vector<Operand> map(vector<Operand> from) const;

	bool isIdentity() const;

	Mapping &apply(Mapping m0);
};

struct Operator {
	Operator();
	Operator(string prefix, string trigger, string infix, string postfix, bool commutative=false, bool reflexive=false);
	~Operator();

	string prefix;
	string trigger;
	string infix;
	string postfix;

	bool commutative;
	bool reflexive;
};

struct Operation {
	enum OpType : int {
		UNDEF = -1,
		WIRE_NOT = 0,
		IDENTITY = 1,
		NEGATION = 2,
		NEGATIVE = 3,
		VALIDITY = 4,
		BOOLEAN_NOT = 5,
		INVERSE = 6,
		WIRE_OR = 7,
		WIRE_AND = 8,
		WIRE_XOR = 9,
		EQUAL = 10,
		NOT_EQUAL = 11,
		LESS = 12,
		GREATER = 13,
		LESS_EQUAL = 14,
		GREATER_EQUAL = 15,
		SHIFT_LEFT = 16,
		SHIFT_RIGHT = 17,
		ADD = 18,
		SUBTRACT = 19,
		MULTIPLY = 20,
		DIVIDE = 21,
		MOD = 22,
		TERNARY = 23,
		BOOLEAN_OR = 24,
		BOOLEAN_AND = 25,
		BOOLEAN_XOR = 26,
		ARRAY = 27,
		INDEX = 28,
		CALL = 29,
		MEMBER = 30,
		STRUCT = 31
	};

	Operation();
	Operation(int func, vector<Operand> args, size_t exprIndex=std::numeric_limits<size_t>::max());
	~Operation();

	static Operation undef(size_t exprIndex=std::numeric_limits<size_t>::max());

	static index_vector<Operator> operators;
	static void set(OpType, Operator op);
	static void loadOperators();

	OpType func;
	vector<Operand> operands;

	// The expression index to map this operation to
	size_t exprIndex;

	static pair<Type, double> funcCost(int func, vector<Type> args);

	void set(int func, vector<Operand> args);
	bool isCommutative() const;
	bool isReflexive() const;
	bool isUndef() const;

	static Value evaluate(int func, vector<Value> args);
	static Value evaluate(int func, vector<Value> args, TypeSet types);
	Value evaluate(State values, vector<Value> expressions) const;
	Value evaluate(State values, vector<Value> expressions, TypeSet types) const;
	void propagate(State &result, const State &global, vector<Value> &expressions, const vector<Value> gexpressions, Value v) const;
	Operation &apply(vector<int> varMap);
	Operation &apply(const Mapping &m);
	Operation extract(vector<size_t> idx, size_t exprIndex=0);
	Operation &offsetExpr(int off);

	Operand op() const;

	void tidy();
};

bool operator==(Operation o0, Operation o1);
bool operator!=(Operation o0, Operation o1);

ostream &operator<<(ostream &os, Operation o);

}
