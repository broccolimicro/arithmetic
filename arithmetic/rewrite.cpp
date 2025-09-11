#include "rewrite.h"
#include "algorithm.h"
#include "expression.h"
#include <common/text.h>

namespace arithmetic {

Rule::Rule() {
	directed = true;
}

Rule::Rule(Operand left, Operand right, bool directed) {
	this->left = left;
	this->right = right;
	this->directed = directed;
}

Rule::~Rule() {
}

RuleSet::RuleSet() {
}

RuleSet::RuleSet(std::initializer_list<Expression> lst) {
	for (auto e = lst.begin(); e != lst.end(); e++) {
		if (not verifyRuleFormat(*e, e->top, true)) {
			continue;
		}

		Operation rule = *sub.getExpr(sub.append(e->sub, {e->top}).map(e->top).index);
		if (rule.func == Operation::EQUAL) {
			rules.push_back(Rule(rule.operands[0], rule.operands[1], false));
		} else if (rule.func == Operation::GREATER) {
			rules.push_back(Rule(rule.operands[0], rule.operands[1], true));
		} else if (rule.func == Operation::LESS) {
			rules.push_back(Rule(rule.operands[1], rule.operands[0], true));
		}
	}

	Mapping m = arithmetic::tidy(sub, top(), true);
	for (auto i = rules.begin(); i != rules.end(); i++) {
		i->left = m.map(i->left);
		i->right = m.map(i->right);
	}
}

RuleSet::~RuleSet() {
}

bool RuleSet::empty() const {
	return rules.empty();
}

vector<Operand> RuleSet::top() const {
	vector<Operand> result;
	for (auto i = rules.begin(); i != rules.end(); i++) {
		if (i->left.isExpr()) {
			result.push_back(i->left);
		}
		if (i->right.isExpr()) {
			result.push_back(i->right);
		}
	}

	sort(result.begin(), result.end());
	result.erase(unique(result.begin(), result.end()), result.end());
	return result;
}

RuleSet &RuleSet::operator+=(const RuleSet &r1) {
	Mapping m = sub.append(r1.sub, r1.top());
	for (int i = 0; i < (int)r1.rules.size(); i++) {
		Rule r = r1.rules[i];
		r.left = m.map(r.left);
		r.right = m.map(r.right);
		rules.push_back(r);
	}
	return *this;
}


bool verifyRuleFormat(ConstOperationSet ops, Operand i, bool msg) {
	// ==, <, >
	if (not i.isExpr()) {
		if (msg) printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		return false;
	}
	auto j = ops.getExpr(i.index);
	if (j == nullptr
		or j->operands.size() != 2u
		or (j->func != Operation::EQUAL
		and j->func != Operation::LESS
		and j->func != Operation::GREATER)) {
		if (msg) printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		return false;
	}
	return true;
}

RuleSet operator+(RuleSet r0, const RuleSet &r1) {
	return (r0+=r1);
}

ostream &operator<<(ostream &os, const RuleSet &r) {
	os << r.sub;
	
	for (int i = 0; i < (int)r.rules.size(); i++) {
		os << i << ": " << r.rules[i].left << (r.rules[i].directed ? ">" : "=") << r.rules[i].right << endl;
	}
	return os;
}

RuleSet rewriteCanonical() {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
 
	return RuleSet({
		(a-b) > (a+(-b)),
		(a/b) > (a*inv(b)),
		(a > b) > isNegative(b+(-a)),
		(a >= b) > not isNegative(a+(-b)),
		(a < b) > isNegative(a+(-b)),
		(a <= b) > not isNegative(b+(-a)),
	});
}

RuleSet rewriteSimple() {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();
	Expression gnd = Expression::gnd();
	Expression vdd = Expression::vdd();

	return RuleSet({
		(a-b) > (a+(-b)),
		(a/b) > (a*inv(b)),
		(a > b) > isNegative(b+(-a)),
		(a >= b) > not isNegative(a+(-b)),
		(a < b) > isNegative(a+(-b)),
		(a <= b) > not isNegative(b+(-a)),

		// Wire identity rules
		(~(~a)) > (isValid(a)),
		(isValid(~a)) > (~a),
		(~isValid(a)) > (~a),
		(isValid(isValid(a))) > (isValid(a)),

		(isValid(a) & b) > (a & b),
		(isValid(a & b)) > (a & b),

		(isValid(a) | b) > (a | b),
		(isValid(a | b)) > (a | b),
		
		(a & a) > (isValid(a)),
		(a & ~a) > (gnd),
		
		(a | a) > (isValid(a)),
		(a | ~a) > (vdd),

		// Wire constant propagation
		(vdd & a) > (isValid(a)),
		(gnd & a) > (gnd),
		
		(vdd | a) > (vdd),
		(gnd | a) > (isValid(a)),

		// Unwrap validity of arithmetic operations
		(a & (-b)) > (a & b),
		(a | (-b)) > (a | b),
		(isValid(-a)) > (isValid(a)),
		(~(-b)) > (~b),

		(a & inv(b)) > (a & b),
		(a | inv(b)) > (a | b),
		(isValid(inv(a))) > (isValid(a)),
		(~inv(a)) > (~a),

		(a & (b+c)) > (a & b & c),
		(a | (b+c)) > (a | (b & c)),
		(isValid(a+b)) > (a & b),
		(~(a+b)) > (~a | ~b),

		(a & (b*c)) > (a & b & c),
		(a | (b*c)) > (a | (b & c)),
		(isValid(a*b)) > (a & b),
		(~(a*b)) > (~a | ~b),

		// Simplify arithmetic expressions	
		(a+a) > (2*a),
		(a+(-a)) > (0),
		(X+a) > (X),
		(0+a) > (a),

		(a*inv(a)) > (1),
		(inv(a*b)) > (inv(a)*inv(b)),
		(inv(inv(a))) > a,
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),


		//(a & (!b)) > (a & b),
		//(a | (!b)) > (a | b),
		//(~(!b)) > (~b),
		//(a and (b & c)) > (a and b and c),
		//(a or (b & c)) > (a or (b and c)),
		//(not (b & c)) > (not b or not c),
		//(a and (b | c)) > (a and b and c),
		//(a or (b | c)) > (a or (b and c)),
		//(not (b | c)) > (not b or not c),
	
		/*(a ^^ a) > (0),
		(a && a) > (a),
		(0 && a) > (0),
		(0 || a) > (a),
		(-1 && a) > (a),
		(-1 || a) > (-1),
		(a || a) > (a),
		(!!a) > (a)*/
	});
}

RuleSet rewriteHuman() {
	// matches all variables and Expressions
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();
	Expression gnd = Expression::gnd();
	Expression vdd = Expression::vdd();

	// true matches valid
	// false matches neutral
	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
 
	return RuleSet({
		(a + (-b)) > (a - b),
		(isNegative(a)) > (a < 0),
		(not (a < b)) > (a >= b),
		(a-b < c) > (a < b+c),
		(inv(a)) > (1/a),
	});
}

RuleSet rewriteUndirected() {
	// matches all variables and Expressions
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();
	Expression gnd = Expression::gnd();
	Expression vdd = Expression::vdd();

	return RuleSet({
		(not booleanAnd(a)) == booleanOr(not a),
		(not booleanOr(a)) == booleanAnd(not a),
		(a and booleanOr(b)) == booleanOr(a and b),
		(~wireAnd(a)) == wireOr(~a),
		(~wireOr(a)) == wireAnd(~a),
		(a & wireOr(b)) == wireOr(a & b),
		(-add(a)) == add(-a),
		(-(a * b)) == ((-a)*b),
		(a * add(b)) == add(a * b),
		booleanXor(a, b) == ((a and (not b)) or ((not a) and b)),
		(a ^ b) == ((a & (~b)) | ((~a) & b)),
	});
}

/*void valrdyRewrite(vector<Expression> and rewrite) {

}*/

}
