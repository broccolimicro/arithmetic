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
	static Operand structOf(vector<Value> arr);
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
		TYPE_UNDEF = -1,
		TYPE_BITWISE_NOT = 0,
		TYPE_IDENTITY = 1,
		TYPE_NEGATION = 2,
		TYPE_NEGATIVE = 3,
		TYPE_VALIDITY = 4,
		TYPE_BOOLEAN_NOT = 5,
		TYPE_INVERSE = 6,
		TYPE_BITWISE_OR = 7,
		TYPE_BITWISE_AND = 8,
		TYPE_BITWISE_XOR = 9,
		TYPE_EQUAL = 10,
		TYPE_NOT_EQUAL = 11,
		TYPE_LESS = 12,
		TYPE_GREATER = 13,
		TYPE_LESS_EQUAL = 14,
		TYPE_GREATER_EQUAL = 15,
		TYPE_SHIFT_LEFT = 16,
		TYPE_SHIFT_RIGHT = 17,
		TYPE_ADD = 18,
		TYPE_SUBTRACT = 19,
		TYPE_MULTIPLY = 20,
		TYPE_DIVIDE = 21,
		TYPE_MOD = 22,
		TYPE_TERNARY = 23,
		TYPE_BOOLEAN_OR = 24,
		TYPE_BOOLEAN_AND = 25,
		TYPE_BOOLEAN_XOR = 26,
		TYPE_ARRAY = 27,
		TYPE_INDEX = 28,
		TYPE_CALL = 29,
		TYPE_MEMBER = 30
	};

	Operation();
	Operation(int func, vector<Operand> args, size_t exprIndex=std::numeric_limits<size_t>::max());
	~Operation();

	static Operation undef(size_t exprIndex=std::numeric_limits<size_t>::max());

	static vector<Operator> operators;
	static const int UNDEF = -1;
	static int BITWISE_NOT;
	static int IDENTITY;
	static int NEGATION;
	static int NEGATIVE;
	static int VALIDITY;
	static int BOOLEAN_NOT;
	static int INVERSE;
	static int BITWISE_OR;
	static int BITWISE_AND;
	static int BITWISE_XOR;
	static int EQUAL;
	static int NOT_EQUAL;
	static int LESS;
	static int GREATER;
	static int LESS_EQUAL;
	static int GREATER_EQUAL;
	static int SHIFT_LEFT;
	static int SHIFT_RIGHT;
	static int ADD;
	static int SUBTRACT;
	static int MULTIPLY;
	static int DIVIDE;
	static int MOD;
	static int TERNARY;
	static int BOOLEAN_OR;
	static int BOOLEAN_AND;
	static int BOOLEAN_XOR;
	static int ARRAY;
	static int INDEX;
	static int CALL;
	static int MEMBER;

	static int push(Operator op);
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
