#pragma once

#include <common/standard.h>
#include <common/mapping.h>

#include "state.h"
#include "type.h"

namespace arithmetic {

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
	static Operand X(Value::ValType type=Value::WIRE);
	static Operand U(Value::ValType type=Value::WIRE);
	static Operand gnd(Value::ValType type=Value::WIRE);
	static Operand vdd();
	static Operand boolOf(bool bval);
	static Operand intOf(int64_t ival);
	static Operand realOf(double rval);
	static Operand arrOf(vector<Value> arr);
	static Operand structOf(string name, vector<Value> arr);
	static Operand stringOf(string sval);

	static Operand exprOf(size_t index);
	Operand &offsetExpr(int off);

	static Operand varOf(size_t index);
	Operand &applyVars(const Mapping<size_t> &m);
	Operand &applyVars(const Mapping<int> &m);
	Operand &applyExprs(const Mapping<size_t> &m);
	Operand &applyExprs(const Mapping<int> &m);
	
	static Operand typeOf(Type type);
};

ostream &operator<<(ostream &os, Operand o);

bool operator==(Operand o0, Operand o1);
bool operator!=(Operand o0, Operand o1);
bool operator<(Operand o0, Operand o1); // does not differentiate constants

struct Operator {
	Operator();
	Operator(string prefix, string trigger, string infix, string postfix, uint8_t flags=0);
	~Operator();

	enum Flags {
		COMMUTATIVE = 1,
		REFLEXIVE = 2,
	};

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
		VALIDITY,
		WIRE_NOT,
		WIRE_OR,
		WIRE_AND,
		WIRE_XOR,
		
		TRUTHINESS,
		BOOLEAN_NOT,
		BOOLEAN_OR,
		BOOLEAN_AND,
		BOOLEAN_XOR,

		EQUAL,
		NOT_EQUAL,
		LESS,
		GREATER,
		LESS_EQUAL,
		GREATER_EQUAL,
		NEGATIVE,
		TERNARY,

		IDENTITY,
		NEGATION,
		INVERSE,

		SHIFT_LEFT,
		SHIFT_RIGHT,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		MOD,

		CALL,

		ARRAY,
		INDEX,

		STRUCT,
		MEMBER,
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

	static Value evaluate(int func, vector<Value> args, TypeSet types=TypeSet());
	Value evaluate(State values, vector<Value> expressions, TypeSet types=TypeSet()) const;
	void propagate(State &result, const State &global, vector<Value> &expressions, const vector<Value> gexpressions, Value v) const;
	Operation &applyVars(const Mapping<size_t> &m);
	Operation &applyVars(const Mapping<int> &m);
	Operation &applyExprs(const Mapping<size_t> &m);
	Operation &applyExprs(const Mapping<int> &m);
	Operation &apply(const Mapping<Operand> &m);
	Operation extract(vector<size_t> idx, size_t exprIndex=0);
	Operation &offsetExpr(int off);

	Operand op() const;

	void tidy();
};

bool operator==(Operation o0, Operation o1);
bool operator!=(Operation o0, Operation o1);

ostream &operator<<(ostream &os, Operation o);

}
