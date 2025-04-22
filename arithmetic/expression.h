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
	~Operand();

	// Used by "type"
	enum {
		CONST  = 0,
		VAR    = 1,
		EXPR   = 2,
	};

	int type;

	// used for CONST
	Value cnst;

	// used for VAR and EXPR
	size_t index;

	bool isConst() const;
	bool isExpr() const;
	bool isVar() const;

	Value get(State values=State(), vector<Value> expressions=vector<Value>()) const;
	void set(State &values, vector<Value> &expressions, Value v) const;

	// Constants
	static Operand X();
	static Operand U();
	static Operand boolOf(bool bval);
	static Operand intOf(int64_t ival);
	static Operand realOf(double rval);
	static Operand arrOf(vector<Value> arr);
	static Operand structOf(vector<Value> arr);

	static Operand exprOf(size_t index);
	void offsetExpr(int off);

	static Operand varOf(size_t index);
	void apply(vector<int> uidMap);
	void apply(vector<Operand> uidMap);
};

ostream &operator<<(ostream &os, Operand o);

bool areSame(Operand o0, Operand o1);

struct Operator {
	Operator();
	Operator(vector<string> infix, string prefix="", string postfix="", bool commutative=false, bool reflexive=false);
	~Operator();

	string prefix;
	vector<string> infix;
	string postfix;

	bool commutative;
	bool reflexive;
};

struct Operation {
	Operation();
	Operation(int func, vector<Operand> args, bool isCall=false);
	~Operation();

	static vector<Operator> operators;
	static int BITWISE_NOT;
	static int IDENTITY;
	static int NEGATION;
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

	static int push(Operator op);
	static void loadOperators();

	bool isCall;
	int func;
	vector<Operand> operands;

	static pair<Type, double> funcCost(int func, vector<Type> args);

	void set(int func, vector<Operand> args, bool isCall=false);
	bool isCommutative() const;
	bool isReflexive() const;

	static Value evaluate(int func, vector<Value> args);
	Value evaluate(State values, vector<Value> expressions) const;
	void propagate(State &result, const State &global, vector<Value> &expressions, const vector<Value> gexpressions, Value v) const;
	void apply(vector<int> uidMap);
	void apply(vector<Operand> uidMap);
	void offsetExpr(int off);
};

bool areSame(Operation o0, Operation o1);

ostream &operator<<(ostream &os, Operation o);

struct Expression {
	Expression();
	Expression(Operand arg0);
	Expression(int func, Operand arg0);
	Expression(int func, Expression arg0);
	Expression(int func, Operand arg0, Operand arg1);
	Expression(int func, Expression arg0, Operand arg1);
	Expression(int func, Operand arg0, Expression arg1);
	Expression(int func, Expression arg0, Expression arg1);
	Expression(int func, vector<Expression> args);
	~Expression();

	// The Expression consists entirely of a tree of operations stored in an
	// array. operations are stored in the array in order of precedence. So if
	// the Expression is (a+b)*3 + x*y, then they'll be stored in the following order:
	// 0. a+b
	// 1. operations[0]*3
	// 2. x*y
	// 3. operations[1]+operations[2]
	// Therefore the final Operation stored is the Operation that produces the
	// final Value for the Expression.
	vector<Operation> operations;

	int push(Operation arg);

	void set(Operand arg0);
	void set(int func, Operand arg0);
	void set(int func, Expression arg0);
	void set(int func, Operand arg0, Operand arg1);
	void set(int func, Expression arg0, Expression arg1);
	void set(int func, Expression arg0, Operand arg1);
	void set(int func, Operand arg0, Expression arg1);
	void set(int func, vector<Expression> args);
	void call(int func, vector<Expression> args);
	void push(int func);
	void push(int func, Operand arg0);
	void push(int func, Expression arg0);
	Value evaluate(State values) const;
	bool isNull() const;
	bool isConstant() const;
	bool isValid() const;
	bool isNeutral() const;
	bool isWire() const;

	void apply(vector<int> uidMap);
	void apply(vector<Expression> uidMap);

	// operating on the Expression
	void insert(size_t index, size_t num);
	void erase(size_t index);
	void erase(vector<size_t> index, bool doSort=false);

	Expression &eraseDangling();
	Expression &propagateConstants();
	Expression &canonicalize(bool rules=false);

	struct Match {
		// what to replace this match with from the rules
		Operand replace;
		// index into operations
		vector<size_t> expr;
		// index into operands for expr.back()
		vector<size_t> top;

		// map variable index to Operand in this
		map<size_t, vector<Operand> > vars;
	};

	Cost cost(vector<Type> vars) const;
	vector<Match> search(const Expression &rules, size_t count=0, bool fwd=true, bool bwd=true);
	void replace(Operand o0, Operand o1);
	void replace(const Expression &rules, Match token);
	size_t count(Operand start) const;
	Expression &minimize(Expression directed=Expression());

	Expression &operator=(Operand e);

	string to_string();
};

Expression espresso(Expression expr, vector<Type> vars=vector<Type>(), Expression directed=Expression(), Expression undirected=Expression());

bool canMap(vector<Operand> search, Operand rule, const Expression &e0, const Expression &e1, bool init, map<size_t, vector<Operand> > *vars=nullptr);

ostream &operator<<(ostream &os, Expression e);
ostream &operator<<(ostream &os, Expression::Match m);

Expression operator~(Expression e);
Expression operator-(Expression e);
Expression isValid(Expression e);
Expression operator!(Expression e);
Expression inv(Expression e);
Expression operator|(Expression e0, Expression e1);
Expression operator&(Expression e0, Expression e1);
Expression operator^(Expression e0, Expression e1);
Expression bitwiseXor(Expression e0, Expression e1);
Expression operator==(Expression e0, Expression e1);
Expression operator!=(Expression e0, Expression e1);
Expression operator<(Expression e0, Expression e1);
Expression operator>(Expression e0, Expression e1);
Expression operator<=(Expression e0, Expression e1);
Expression operator>=(Expression e0, Expression e1);
Expression operator<<(Expression e0, Expression e1);
Expression operator>>(Expression e0, Expression e1);
Expression operator+(Expression e0, Expression e1);
Expression operator-(Expression e0, Expression e1);
Expression operator*(Expression e0, Expression e1);
Expression operator/(Expression e0, Expression e1);
Expression operator%(Expression e0, Expression e1);
Expression operator&&(Expression e0, Expression e1);
Expression operator||(Expression e0, Expression e1);

Expression operator|(Expression e0, Operand e1);
Expression operator&(Expression e0, Operand e1);
Expression operator^(Expression e0, Operand e1);
Expression bitwiseXor(Expression e0, Operand e1);
Expression operator==(Expression e0, Operand e1);
Expression operator!=(Expression e0, Operand e1);
Expression operator<(Expression e0, Operand e1);
Expression operator>(Expression e0, Operand e1);
Expression operator<=(Expression e0, Operand e1);
Expression operator>=(Expression e0, Operand e1);
Expression operator<<(Expression e0, Operand e1);
Expression operator>>(Expression e0, Operand e1);
Expression operator+(Expression e0, Operand e1);
Expression operator-(Expression e0, Operand e1);
Expression operator*(Expression e0, Operand e1);
Expression operator/(Expression e0, Operand e1);
Expression operator%(Expression e0, Operand e1);
Expression operator&&(Expression e0, Operand e1);
Expression operator||(Expression e0, Operand e1);

Expression operator|(Operand e0, Expression e1);
Expression operator&(Operand e0, Expression e1);
Expression operator^(Operand e0, Expression e1);
Expression bitwiseXor(Operand e0, Expression e1);
Expression operator==(Operand e0, Expression e1);
Expression operator!=(Operand e0, Expression e1);
Expression operator<(Operand e0, Expression e1);
Expression operator>(Operand e0, Expression e1);
Expression operator<=(Operand e0, Expression e1);
Expression operator>=(Operand e0, Expression e1);
Expression operator<<(Operand e0, Expression e1);
Expression operator>>(Operand e0, Expression e1);
Expression operator+(Operand e0, Expression e1);
Expression operator-(Operand e0, Expression e1);
Expression operator*(Operand e0, Expression e1);
Expression operator/(Operand e0, Expression e1);
Expression operator%(Operand e0, Expression e1);
Expression operator&&(Operand e0, Expression e1);
Expression operator||(Operand e0, Expression e1);

Expression operator|(Operand e0, Operand e1);
Expression operator&(Operand e0, Operand e1);
Expression operator^(Operand e0, Operand e1);
Expression bitwiseXor(Operand e0, Operand e1);
Expression operator==(Operand e0, Operand e1);
Expression operator!=(Operand e0, Operand e1);
Expression operator<(Operand e0, Operand e1);
Expression operator>(Operand e0, Operand e1);
Expression operator<=(Operand e0, Operand e1);
Expression operator>=(Operand e0, Operand e1);
Expression operator<<(Operand e0, Operand e1);
Expression operator>>(Operand e0, Operand e1);
Expression operator+(Operand e0, Operand e1);
Expression operator-(Operand e0, Operand e1);
Expression operator*(Operand e0, Operand e1);
Expression operator/(Operand e0, Operand e1);
Expression operator%(Operand e0, Operand e1);
Expression operator&&(Operand e0, Operand e1);
Expression operator||(Operand e0, Operand e1);

Expression array(vector<Expression> e);

// Used to create elastic rewrite rules for commutative operators
Expression booleanOr(Expression e0);
Expression booleanAnd(Expression e0);
Expression booleanXor(Expression e0);
Expression bitwiseOr(Expression e0);
Expression bitwiseAnd(Expression e0);
Expression bitwiseXor(Expression e0);
Expression add(Expression e0);
Expression mult(Expression e0);

Expression booleanOr(vector<Expression> e0);
Expression booleanAnd(vector<Expression> e0);
Expression booleanXor(vector<Expression> e0);
Expression bitwiseOr(vector<Expression> e0);
Expression bitwiseAnd(vector<Expression> e0);
Expression bitwiseXor(vector<Expression> e0);
Expression add(vector<Expression> e0);
Expression mult(vector<Expression> e0);

Expression call(int func, vector<Expression> args);

int passesGuard(const State &encoding, const State &global, const Expression &guard, State *total);
Expression weakestGuard(const Expression &guard, const Expression &exclude);

}
