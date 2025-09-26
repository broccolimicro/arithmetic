#include <gtest/gtest.h>

#include <arithmetic/algorithm.h>
#include <arithmetic/expression.h>
#include <arithmetic/rewrite.h>
#include <common/mapping.h>
#include <common/text.h>

using namespace arithmetic;
using namespace std;

void verifyRule(const RuleSet &rules, int idx) {
	Rule rule = rules.rules[idx];
	cout << "evaluating rule " << to_string(rules.sub, rule.left, false) << "->" << to_string(rules.sub, rule.right, false) << endl;
	State s;
	for (ConstDownIterator i(rules.sub, {rule.left, rule.right}); not i.done(); ++i) {
		auto j = i.get();
		for (auto k = j.operands.begin(); k != j.operands.end(); k++) {
			if (k->isVar() and (k->index >= s.values.size() or s.values[k->index].isUndef())) {
				s.set(k->index, Value::intOf(rand()%100+1), true);
			}
		}
	}

	cout << "on state " << s << endl;
	try {
		Value left = evaluate(rules.sub, rule.left, s).val;
		Value right = evaluate(rules.sub, rule.right, s).val;

		EXPECT_TRUE(areSame(left, right)) << "rule " << idx << ": " << left.typeName() << ":" << left << " != " << right.typeName() << ":" << right << endl << subExpr(rules.sub, rule.left) << "vs" << endl << subExpr(rules.sub, rule.right);
	} catch (const std::exception& e) {
		FAIL() << e.what() << " rule " << idx << ": " << subExpr(rules.sub, rule.left) << "vs" << endl << subExpr(rules.sub, rule.right);
	}
	cout << endl;
}

void verifyRewrite(const RuleSet &rules) {
	for (int i = 0; i < (int)rules.rules.size(); i++) {
		verifyRule(rules, i);
	}
}	

TEST(Rewrite, Basic) {
	verifyRewrite(rewriteSimple());
}

TEST(Rewrite, Human) {
	verifyRewrite(rewriteHuman());
}

TEST(Rewrite, Undirected) {
	verifyRewrite(rewriteUndirected());
}

TEST(Rewrite, Commutative) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);

	auto rules = RuleSet({
		(a & wireOr(b)) > (wireOr(a&b)),
	});

	Expression dut = (a | b) & (c | d);
	//Expression exp = ((a & c) | (a & d) | (b & c) | (b & d));
	//exp.minimize();
	cout << dut << endl;
	dut.minimize(rules);
	cout << dut << endl;
	//dut.minimize();
	//EXPECT_TRUE(areSame(dut, exp)) << dut << " != " << exp << endl;
}


