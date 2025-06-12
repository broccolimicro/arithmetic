#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

#include "operation.h"

namespace arithmetic {

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

	Operation *operationAt(size_t index);
	const Operation *operationAt(size_t index) const;

	Operand popReflexive(size_t index=std::numeric_limits<size_t>::max());
	Operand push(Operation arg);
	Operand push(Expression arg);

	Operand set(Operand arg0);
	Operand set(int func, Operand arg0);
	Operand set(int func, Expression arg0);
	Operand set(int func, Operand arg0, Operand arg1);
	Operand set(int func, Expression arg0, Expression arg1);
	Operand set(int func, Expression arg0, Operand arg1);
	Operand set(int func, Operand arg0, Expression arg1);
	Operand set(int func, vector<Expression> args);
	Operand push(int func);
	Operand push(int func, Operand arg0);
	Operand push(int func, Expression arg0);
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

bool areSame(Expression e0, Expression e1);

Expression espresso(Expression expr, vector<Type> vars=vector<Type>(), Expression directed=Expression(), Expression undirected=Expression());

bool canMap(vector<Operand> search, Operand rule, ConstOperationSet e0, ConstOperationSet e1, bool init, map<size_t, vector<Operand> > *vars=nullptr);

ostream &operator<<(ostream &os, Expression e);
ostream &operator<<(ostream &os, Expression::Match m);

Expression operator~(Expression e);
Expression operator-(Expression e);
Expression ident(Expression e);
Expression isValid(Expression e);
Expression isNegative(Expression e);
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
