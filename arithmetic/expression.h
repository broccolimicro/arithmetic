#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

#include "operation_set.h"
#include "rewrite.h"

namespace arithmetic {

// The Expression is a tree of operations stored in an array.
// 0. a+b
// 1. operations[0]*3
// 2. x*y
// 3. operations[1]+operations[2]
struct Expression {
	Expression(Operand top = Operand::undef());
	Expression(int func, vector<Operand> args);
	Expression(int func, vector<Expression> args);
	~Expression();

	SimpleOperationSet sub;
	Operand top;

	static Expression undef();
	static Expression X(Value::ValType type=Value::WIRE);
	static Expression U(Value::ValType type=Value::WIRE);
	static Expression gnd(Value::ValType type=Value::WIRE);
	static Expression vdd();
	static Expression boolOf(bool bval);
	static Expression intOf(int64_t ival);
	static Expression realOf(double rval);
	static Expression arrOf(vector<Value> arr);
	static Expression structOf(string name, vector<Value> arr);
	static Expression stringOf(string sval);
	static Expression varOf(size_t index);
	static Expression typeOf(Operand::Type type);

	vector<Operand> exprIndex() const;
	const Operation *getExpr(size_t index) const;
	bool setExpr(Operation o);
	Operand pushExpr(Operation o);
	bool eraseExpr(size_t index);

	void clear();
	void tidy();
	void minimize(RuleSet rules=RuleSet());
	Expression minimized(RuleSet rules=RuleSet());
	size_t size() const;

	Operand append(Expression arg);
	vector<Operand> append(vector<Expression> arg);
	Expression &push(int func, vector<Operand> args);

	bool isUndef() const;
	bool isNull() const;
	bool isConstant() const;
	bool isValid() const;
	bool isNeutral() const;
	bool isWire() const;

	Expression &applyVars(const Mapping<size_t> &m);
	Expression &applyVars(const Mapping<int> &m);
	Expression &apply(const Mapping<Operand> &m);

	string to_string(bool debug=false) const;

	Expression operator()(Expression idx) const;
	Expression operator()(Expression from, Expression to) const;
	Expression operator()(Operand idx) const;
	Expression operator()(Operand from, Operand to) const;
	Expression operator()(Operand from, Expression to) const;
	Expression operator()(Expression from, Operand to) const;
};

bool areSame(Expression e0, Expression e1);

ostream &operator<<(ostream &os, Expression e);

Expression operator~(Expression e);
Expression operator-(Expression e);
Expression ident(Expression e);
Expression isValid(Expression e);
Expression isTrue(Expression e);
Expression isNegative(Expression e);
Expression operator!(Expression e);
Expression inv(Expression e);
Expression operator|(Expression e0, Expression e1);
Expression operator&(Expression e0, Expression e1);
Expression operator^(Expression e0, Expression e1);
Expression booleanXor(Expression e0, Expression e1);
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
Expression booleanXor(Expression e0, Operand e1);
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
Expression booleanXor(Operand e0, Expression e1);
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

// Used to create elastic rewrite rules for commutative operators
Expression booleanOr(Expression e0);
Expression booleanAnd(Expression e0);
Expression booleanXor(Expression e0);
Expression wireOr(Expression e0);
Expression wireAnd(Expression e0);
Expression wireXor(Expression e0);
Expression add(Expression e0);
Expression mult(Expression e0);

Expression array(vector<Expression> e);
Expression booleanOr(vector<Expression> e0);
Expression booleanAnd(vector<Expression> e0);
Expression booleanXor(vector<Expression> e0);
Expression wireOr(vector<Expression> e0);
Expression wireAnd(vector<Expression> e0);
Expression wireXor(vector<Expression> e0);
Expression add(vector<Expression> e0);
Expression mult(vector<Expression> e0);

Expression call(string func_name, vector<Expression> args);

int passesGuard(const State &encoding, const State &global, const Expression &guard, State *total);
Expression weakestGuard(const Expression &guard, const Expression &exclude);

}
