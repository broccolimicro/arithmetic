#pragma once

#include "operation_set.h"

namespace arithmetic {

struct Expression;

struct Rule {
	Rule();
	Rule(Operand left, Operand right, bool directed=true);
	~Rule();

	Operand left;
	Operand right;
	bool directed;
};

struct RuleSet {
	RuleSet();
	RuleSet(std::initializer_list<Expression> lst);
	~RuleSet();

	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
	SimpleOperationSet sub;
	vector<Rule> rules;

	bool empty() const;
	vector<Operand> top() const;

	RuleSet &operator+=(const RuleSet &r1);
};

bool verifyRuleFormat(ConstOperationSet ops, Operand i, bool msg);

RuleSet operator+(RuleSet r0, const RuleSet &r1);

ostream &operator<<(ostream &os, const RuleSet &r);

// load basic simplifcation rules into rewrite vector
RuleSet rewriteCanonical();
RuleSet rewriteSimple();
RuleSet rewriteHuman();
RuleSet rewriteUndirected();

}


