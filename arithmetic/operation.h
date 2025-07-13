#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

namespace arithmetic {

struct Operand {
	Operand(Value v = Value());
	Operand(bool bval);
	Operand(int64_t ival);
	Operand(int ival);
	Operand(double rval);
	Operand(string sval);
	~Operand();

	// Used by "type"
	enum {
		UNDEF  = -1,
		CONST  = 0,
		VAR    = 1,
		EXPR   = 2,
		TYPE   = 3,
	};

	int type;

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
	void apply(vector<int> varMap);
	void apply(vector<Operand> varMap);
	void replace(vector<Operand> exprMap);
	
	static Operand typeOf(int type);
};

ostream &operator<<(ostream &os, Operand o);

bool areSame(Operand o0, Operand o1);

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
	Operation();
	Operation(int func, vector<Operand> args, size_t exprIndex=std::numeric_limits<size_t>::max());
	~Operation();

	static vector<Operator> operators;
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

	int func;
	vector<Operand> operands;

	// The expression index to map this operation to
	size_t exprIndex;

	static pair<Type, double> funcCost(int func, vector<Type> args);

	void set(int func, vector<Operand> args);
	bool isCommutative() const;
	bool isReflexive() const;

	static Value evaluate(int func, vector<Value> args);
	static Value evaluate(int func, vector<Value> args, TypeSet types);
	Value evaluate(State values, vector<Value> expressions) const;
	Value evaluate(State values, vector<Value> expressions, TypeSet types) const;
	void propagate(State &result, const State &global, vector<Value> &expressions, const vector<Value> gexpressions, Value v) const;
	void apply(vector<int> varMap);
	void apply(vector<Operand> varMap);
	void replace(vector<Operand> exprMap);
	Operation extract(vector<size_t> idx, size_t exprIndex=0);
	Operation &offsetExpr(int off);

	void tidy();
};

bool areSame(Operation o0, Operation o1);

ostream &operator<<(ostream &os, Operation o);

}
