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
	State s;
	for (ConstDownIterator i(rules.sub, {rule.left, rule.right}); not i.done(); ++i) {
		auto j = i.get();
		for (auto k = j.operands.begin(); k != j.operands.end(); k++) {
			if (k->isVar() and s.get(k->index).isUnknown()) {
				s.set(k->index, Value::intOf(rand()%100+1));
			}
		}
	}

	try {
		Value left = evaluate(rules.sub, rule.left, s);
		Value right = evaluate(rules.sub, rule.right, s);

		EXPECT_TRUE(areSame(left, right)) << "rule " << idx << ": " << left.typeName() << ":" << left << " != " << right.typeName() << ":" << right << endl << subExpr(rules.sub, rule.left) << "vs" << endl << subExpr(rules.sub, rule.right);
	} catch (const std::exception& e) {
		FAIL() << e.what() << " rule " << idx << ": " << subExpr(rules.sub, rule.left) << "vs" << endl << subExpr(rules.sub, rule.right);
	}
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


