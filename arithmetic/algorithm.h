#pragma once

#include "state.h"
#include "iterator.h"
#include "operation.h"
#include "type.h"
#include "expression.h"
#include <ostream>

namespace arithmetic {

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

ostream &operator<<(ostream &os, Match m);

Value evaluate(ConstOperationSet expr, Operand top, State values);
Cost cost(ConstOperationSet ops, Operand top, vector<Type> vars);

bool verifyRuleFormat(ConstOperationSet ops, Operand i, bool msg=true);
bool verifyRulesFormat(ConstOperationSet ops, Operand top, bool msg=true);

bool canMap(vector<Operand> o0, Operand o1, ConstOperationSet e0, ConstOperationSet e1, bool init, map<size_t, vector<Operand> > *vars=nullptr);
Operand extract(OperationSet expr, size_t from, vector<size_t> operands);
Expression subExpr(ConstOperationSet e0, Operand top);

Mapping tidy(OperationSet expr, vector<Operand> top, bool rules=false);

vector<Match> search(ConstOperationSet ops, vector<Operand> pin, const Expression &rules, size_t count=0, bool fwd=true, bool bwd=true);
Mapping replace(OperationSet expr, const Expression &rules, Match token);
Mapping minimize(OperationSet expr, vector<Operand> top, Expression rules=Expression());

//Expression espresso(Expression expr, vector<Type> vars=vector<Type>(), Expression directed=Expression(), Expression undirected=Expression());

}

