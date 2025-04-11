#pragma once

#include <common/standard.h>

#include "state.h"

namespace arithmetic {

struct Operand {
	Operand(value v = value());
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
	value cnst;

	// used for VAR and EXPR
	size_t index;

	bool isConst() const;
	bool isExpr() const;
	bool isVar() const;

	value get(state values=state(), vector<value> expressions=vector<value>()) const;
	void set(state &values, vector<value> &expressions, value v) const;

	// Constants
	static Operand X();
	static Operand U();
	static Operand boolOf(bool bval);
	static Operand intOf(int64_t ival);
	static Operand realOf(double rval);
	static Operand arrOf(vector<value> arr);
	static Operand structOf(vector<value> arr);

	static Operand exprOf(size_t index);
	void offsetExpr(int off);

	static Operand varOf(size_t index);
	void apply(vector<int> uid_map);
	void apply(vector<Operand> uid_map);
};

ostream &operator<<(ostream &os, Operand o);

bool areSame(Operand o0, Operand o1);

struct Operation
{
	Operation();
	Operation(string func, vector<Operand> args);
	~Operation();

	int func;
	vector<Operand> operands;

	static string funcString(int func);
	static int funcIndex(string func, int args=2);

	void set(string func, vector<Operand> args);
	string get() const;
	bool is_commutative() const;

	static value evaluate(int func, vector<value> args);
	value evaluate(state values, vector<value> expressions) const;
	void propagate(state &result, const state &global, vector<value> &expressions, const vector<value> gexpressions, value v) const;
	void apply(vector<int> uid_map);
	void apply(vector<Operand> uid_map);
	void offsetExpr(int off);
};

bool areSame(Operation o0, Operation o1);

ostream &operator<<(ostream &os, Operation o);

struct Expression {
	Expression();
	Expression(Operand arg0);
	Expression(string func, Operand arg0);
	Expression(string func, Expression arg0);
	Expression(string func, Operand arg0, Operand arg1);
	Expression(string func, Expression arg0, Operand arg1);
	Expression(string func, Operand arg0, Expression arg1);
	Expression(string func, Expression arg0, Expression arg1);
	Expression(string func, vector<Expression> args);
	~Expression();

	// The Expression consists entirely of a tree of operations stored in an
	// array. operations are stored in the array in order of precedence. So if
	// the Expression is (a+b)*3 + x*y, then they'll be stored in the following order:
	// 0. a+b
	// 1. operations[0]*3
	// 2. x*y
	// 3. operations[1]+operations[2]
	// Therefore the final Operation stored is the Operation that produces the
	// final value for the Expression.
	vector<Operation> operations;

	int find(Operation arg);
	int push(Operation arg);

	void set(Operand arg0);
	void set(string func, Operand arg0);
	void set(string func, Expression arg0);
	void set(string func, Operand arg0, Operand arg1);
	void set(string func, Expression arg0, Expression arg1);
	void set(string func, Expression arg0, Operand arg1);
	void set(string func, Operand arg0, Expression arg1);
	void set(string func, vector<Expression> args);
	void push(string func);
	void push(string func, Operand arg0);
	void push(string func, Expression arg0);
	value evaluate(state values) const;
	bool is_null() const;
	bool is_constant() const;
	bool is_valid() const;
	bool is_neutral() const;
	bool is_wire() const;

	void apply(vector<int> uid_map);
	void apply(vector<Expression> uid_map);

	// operating on the Expression
	void erase(size_t index);
	Expression &erase_dangling();
	void replace(Operand o0, Operand o1);
	Expression &propagate_constants();
	vector<pair<Operand, Operand> > match(Operand lhs, const Expression &rule, Operand rhs, map<int, Operand> *mapping=nullptr);
	Expression &minimize(Expression rewrite = Expression());

	Expression &operator=(Operand e);

	string to_string();
};

bool canMap(Operand search, Operand rule, const Expression &e0, const Expression &e1, bool init, map<int, Operand> *mapping=nullptr);

ostream &operator<<(ostream &os, Expression e);

Expression operator~(Expression e);
Expression operator-(Expression e);
Expression is_valid(Expression e);
Expression operator!(Expression e);
Expression operator|(Expression e0, Expression e1);
Expression operator&(Expression e0, Expression e1);
Expression operator^(Expression e0, Expression e1);
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

int passes_guard(const state &encoding, const state &global, const Expression &guard, state *total);
Expression weakest_guard(const Expression &guard, const Expression &exclude);

}
