#pragma once

#include "state.h"
#include "type.h"
#include "expression.h"
#include "rewrite.h"
#include <ostream>

namespace arithmetic {

struct UpIterator {
	UpIterator(OperationSet root, vector<Operand> start=vector<Operand>());
	~UpIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	void setSeen(size_t index);
	bool getSeen(size_t index) const;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	UpIterator &operator++();
	bool done() const;
};

bool operator==(const UpIterator &i0, const UpIterator &i1);
bool operator!=(const UpIterator &i0, const UpIterator &i1);

struct ConstUpIterator {
	ConstUpIterator(ConstOperationSet root, vector<Operand> start=vector<Operand>());
	~ConstUpIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	void setSeen(size_t index);
	bool getSeen(size_t index) const;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstUpIterator &operator++();
	bool done() const;
};

bool operator==(const ConstUpIterator &i0, const ConstUpIterator &i1);
bool operator!=(const ConstUpIterator &i0, const ConstUpIterator &i1);

struct DownIterator {
	DownIterator(OperationSet root, vector<Operand> start=vector<Operand>());
	~DownIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	void setSeen(size_t index);
	bool getSeen(size_t index) const;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	DownIterator &operator++();
	bool done() const;
};

bool operator==(const DownIterator &i0, const DownIterator &i1);
bool operator!=(const DownIterator &i0, const DownIterator &i1);

struct ConstDownIterator {
	ConstDownIterator(ConstOperationSet root, vector<Operand> start=vector<Operand>());
	~ConstDownIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	void setSeen(size_t index);
	bool getSeen(size_t index) const;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstDownIterator &operator++();
	bool done() const;
};

bool operator==(const ConstDownIterator &i0, const ConstDownIterator &i1);
bool operator!=(const ConstDownIterator &i0, const ConstDownIterator &i1);

struct PostOrderDFSIterator {
    PostOrderDFSIterator(ConstOperationSet root, vector<Operand> start=vector<Operand>());
    ~PostOrderDFSIterator();

    ConstOperationSet root;
    vector<bool> visited;
    vector<size_t> stack;
    size_t current;

    const Operation &get();
    const Operation &operator*();
    const Operation *operator->();
    PostOrderDFSIterator &operator++();
    bool done() const;
};

bool operator==(const PostOrderDFSIterator &i0, const PostOrderDFSIterator &i1);
bool operator!=(const PostOrderDFSIterator &i0, const PostOrderDFSIterator &i1);

struct Match {
	// what to replace this match with from the rules
	Operand replace;
	// exprIndex of the top matched operation.
	size_t expr;
	// index into operands for expr. Some operations are commutative and
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

vector<Match> search(ConstOperationSet ops, vector<Operand> pin, const RuleSet &rules, size_t count=0, bool fwd=true, bool bwd=true);
Mapping replace(OperationSet expr, const RuleSet &rules, Match token);
Mapping minimize(OperationSet expr, vector<Operand> top, RuleSet rules=RuleSet());

//Expression espresso(Expression expr, vector<Type> vars=vector<Type>(), Expression directed=Expression(), Expression undirected=Expression());

}

