#pragma once

#include <common/standard.h>

#include "state.h"
#include "type.h"

#include "operation.h"

namespace arithmetic {

struct Expression {
	struct iterator {
		iterator(Expression *root = nullptr);
		~iterator();

		Expression *root;
		// prefer multiple vector<bool> instead of vector<pair<bool, bool>
		// > because vector<bool> is a bitset in a c++
		vector<bool> expand;
		vector<bool> seen;
		vector<size_t> stack;

		Operation &get();
		vector<Operation>::iterator at();
		Operation &operator*();
		vector<Operation>::iterator operator->();
		iterator &operator++();
	};

	struct const_iterator {
		const_iterator(const Expression *root = nullptr);
		~const_iterator();

		const Expression *root;
		// prefer multiple vector<bool> instead of vector<pair<bool, bool>
		// > because vector<bool> is a bitset in a c++
		vector<bool> expand;
		vector<bool> seen;
		vector<size_t> stack;

		const Operation &get();
		vector<Operation>::const_iterator at();
		const Operation &operator*();
		vector<Operation>::const_iterator operator->();
		const_iterator &operator++();
	};

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
	Operand top;

	// next available index to be assigned as an exprIndex for an expression
	size_t nextExprIndex;

	// DESIGN(edward.bingham) this is not stricly necessary, but this is a nice
	// optimization for mapping exprIndex to the index into the operations
	// vector.

	// exprIndex -> index into operations or -1 if not found
	mutable vector<int> exprMap;
	mutable bool exprMapIsDirty;

	iterator at(Operand op);
	iterator begin();
	iterator end();

	const_iterator at(Operand op) const;
	const_iterator begin() const;
	const_iterator end() const;

	void breakMap() const;
	void fixMap() const;
	void setExpr(size_t exprIndex, size_t index) const;
	int lookup(size_t exprIndex) const;

	Expression(Operand arg0 = Operand::undef());
	Expression(int func, vector<Operand> args);
	~Expression();

	vector<Operation>::iterator at(size_t index);
	vector<Operation>::const_iterator at(size_t index) const;

	void clear();

	Operand append(Expression arg);
	vector<Operand> append(vector<Expression> arg);
	Expression &push(int func, vector<Operand> args);

	Value evaluate(State values) const;
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

	struct Match {
		// what to replace this match with from the rules
		Operand replace;
		// exprIndex of matched operations. These are things we can replace.
		vector<size_t> expr;
		// index into operands for expr.back(). Some operations are commutative and
		// commutative operations are rolled up into a single operation with more
		// than two operands to help deal with the search space. top is the index
		// of the operands that were matched. In most cases it will be every
		// operand, but in the commutative cases, it'll only be specific operands.
		vector<size_t> top;

		// map variable index to Operand in this
		map<size_t, vector<Operand> > vars;
	};

	Cost cost(vector<Type> vars) const;
	/*vector<Match> search(const Expression &rules, size_t count=0, bool fwd=true, bool bwd=true);
	void replace(Operand o0, Operand o1);
	void replace(const Expression &rules, Match token);
	size_t count(Operand start) const;
	Expression &minimize(Expression directed=Expression());*/

	Expression &operator=(Operand e);

	string to_string() const;
};

bool operator==(const Expression::iterator &i0, const Expression::iterator &i1);
bool operator!=(const Expression::iterator &i0, const Expression::iterator &i1);
bool operator==(const Expression::const_iterator &i0, const Expression::const_iterator &i1);
bool operator!=(const Expression::const_iterator &i0, const Expression::const_iterator &i1);

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
