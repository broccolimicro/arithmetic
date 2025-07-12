#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

#include "operation.h"
#include "iterator.h"
#include "algorithm.h"

namespace arithmetic {

// The Expression is a tree of operations stored in an array.
// 0. a+b
// 1. operations[0]*3
// 2. x*y
// 3. operations[1]+operations[2]
struct Expression {
	vector<Operation> operations;
	Operand top;

	// next available index to be assigned as an exprIndex for an expression
	size_t nextExprIndex;

	// DESIGN(edward.bingham) this is not stricly necessary, but this is a nice
	// optimization for mapping exprIndex to the index into the operations
	// vector.

	// exprIndex -> index into operations or -1 if not found
	mutable vector<int> exprMap;
	mutable bool exprMapIsDirty;

	void breakMap() const;
	void fixMap() const;
	void setExpr(size_t exprIndex, size_t index) const;
	int lookup(size_t exprIndex) const;

	Expression(Operand arg0 = Operand::undef());
	Expression(int func, vector<Operand> args);
	~Expression();

	Operation *exprAt(size_t index);
	const Operation *exprAt(size_t index) const;
	vector<Operand> exprIndex() const;

	UpIterator exprUp(size_t exprIndex = std::numeric_limits<size_t>::max());
	ConstUpIterator exprUp(size_t exprIndex = std::numeric_limits<size_t>::max()) const;
	DownIterator exprDown(size_t exprIndex = std::numeric_limits<size_t>::max());
	ConstDownIterator exprDown(size_t exprIndex = std::numeric_limits<size_t>::max()) const;

	void clear();

	Operand append(Expression arg);
	vector<Operand> append(vector<Expression> arg);
	Expression &push(int func, vector<Operand> args);

	bool isNull() const;
	bool isConstant() const;
	bool isValid() const;
	bool isNeutral() const;
	bool isWire() const;

	void apply(vector<int> uidMap);
	void apply(vector<Expression> uidMap);

	// TODO(edward.bingham) All of these functions need to be moved out of
	// Expression and operate on an interface that represents any
	// random-access container of Operations
	Expression &tidy(bool rules=false);
	Operand extract(size_t exprIndex, vector<size_t> operands);
	void replace(Operand o0, Operand o1);
	void replace(const Expression &rules, Match token);
	Expression &minimize(Expression directed=Expression());

	Expression &operator=(Operand e);

	string to_string() const;
};

bool areSame(Expression e0, Expression e1);

Expression espresso(Expression expr, vector<Type> vars=vector<Type>(), Expression directed=Expression(), Expression undirected=Expression());

ostream &operator<<(ostream &os, Expression e);

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

// Used to create elastic rewrite rules for commutative operators
Expression booleanOr(Expression e0);
Expression booleanAnd(Expression e0);
Expression booleanXor(Expression e0);
Expression bitwiseOr(Expression e0);
Expression bitwiseAnd(Expression e0);
Expression bitwiseXor(Expression e0);
Expression add(Expression e0);
Expression mult(Expression e0);

Expression array(vector<Expression> e);
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
