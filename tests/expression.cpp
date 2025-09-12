#include <gtest/gtest.h>

#include <arithmetic/algorithm.h>
#include <arithmetic/expression.h>
#include <common/mapping.h>
#include <common/text.h>

using namespace arithmetic;
using namespace std;

TEST(Expression, OperandWireOr) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x|y;
	cout << e << endl;

	State s;
	s.push_back(10);
	s.push_back(Value::gnd());

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::WIRE);
	EXPECT_TRUE(result.isValid());
	EXPECT_FALSE(result.isNeutral());
}

TEST(Expression, OperandWireAnd) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x&y;
	cout << e << endl;

	State s;
	s.push_back(10);
	s.push_back(Value::gnd());

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::WIRE);
	EXPECT_FALSE(result.isValid());
	EXPECT_TRUE(result.isNeutral());
}

TEST(Expression, OperandWireXor) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x^y;
	cout << e << endl;

	State s;
	s.push_back(10);
	s.push_back(Value::gnd());

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::WIRE);
	EXPECT_TRUE(result.isValid());
	EXPECT_FALSE(result.isNeutral());
}

TEST(Expression, OperandEqualTo) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x==y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isValid());
	EXPECT_FALSE(result.bval);

	s.clear();
	s.push_back(5);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isValid());
	EXPECT_TRUE(result.bval);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());
}

TEST(Expression, OperandNotEqualTo) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x!=y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.bval);

	s.clear();	
	s.push_back(5);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_FALSE(result.bval);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());
}

TEST(Expression, OperandLessThan) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x<y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_FALSE(result.bval);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.bval);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());
}

TEST(Expression, OperandGreaterThan) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x>y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.bval);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_FALSE(result.bval);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_TRUE(result.isUnstable());
}

TEST(Expression, Compound) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression f = Expression::varOf(4);
	
	Expression e = (a+b)*c-d%f;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	s.push_back(3);
	s.push_back(6);
	s.push_back(3);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::INT);
	EXPECT_EQ(result.ival, 27);
}

TEST(Expression, TidyConstants) {
	Expression a = Expression::intOf(4);
	Expression b = Expression::intOf(8);
	Expression c = Expression::intOf(2);
	Expression d = Expression::intOf(12);
	
	Expression e = (a+b)*c-d;
	cout << e << endl;
	e.top = tidy(e, {e.top}).map(e.top);
	cout << e << endl;
	ASSERT_EQ(e.size(), 0u);
	EXPECT_TRUE(e.top.isConst());
	EXPECT_EQ(e.top.cnst.type, Value::INT);
	EXPECT_EQ(e.top.cnst.ival, 12);
}

TEST(Expression, TidyCommutative) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	
	Expression e = (a+b)+(c+d);
	ASSERT_EQ(e.size(), 3u);
	cout << e << endl;
	e.top = tidy(e, {e.top}).map(e.top);
	cout << e << endl;
	ASSERT_EQ(e.size(), 1u);
	EXPECT_TRUE(e.top.isExpr());
	EXPECT_EQ(e.getExpr(e.top.index)->operands.size(), 4u);
}

TEST(Expression, Simplify) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	vector<Type> vars({Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0)});
	
	Expression dut = (a*c+b*c-c*a)/c;
	Cost cost = arithmetic::cost(dut, dut.top, vars);
	cout << "Before: dut=" << dut;
	cout << "\tcost=" << cost.complexity << " del=" << cost.critical << endl;
	dut.minimize();
	cout << "After: dut=" << dut;
	EXPECT_TRUE(areSame(dut, b));

	Cost cost2 = arithmetic::cost(dut, dut.top, vars);
	cout << "\tcost=" << cost2.complexity << " del=" << cost2.critical << endl;
	EXPECT_LE(cost2.complexity, cost.complexity);
	EXPECT_LE(cost2.critical, cost.critical);
}

TEST(Expression, ChainOfAdds) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression e = Expression::varOf(4);
	Expression f = Expression::varOf(5);
	Expression g = Expression::varOf(6);
	
	Expression dut = a+b+c+d+e+f+g;
	dut.minimize();
}

TEST(Expression, ElasticRewrite) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression e = Expression::varOf(4);
	
	RuleSet rules({
		a*add(b) > add(a*b),
	});

	Expression dut = e*add({a,b,c,d});
	auto m = arithmetic::search(dut, {dut.top}, rules);
	cout << ::to_string(m) << endl;
}

TEST(Expression, Boolean) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);

	//Expression dut = ((true and not a) and not b) or ((false or c) or d);
	//Expression dut = ((false or c) or d);
	Expression dut = (false or c) or d;
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}

TEST(Expression, Identity) {
	Expression a = Expression::varOf(0);

	Expression dut = a;
	dut.push(Operation::OpType::BOOLEAN_NOT, {dut.top});
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}

TEST(Expression, Function) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression e = Expression::varOf(4);
	Expression f = Expression::varOf(5);

	Expression dut = call("ident", {a+b, c, d+e+f});
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}

TEST(Expression, Apply) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression e = Expression::varOf(4);
	Expression f = Expression::varOf(5);

	std::vector<int> nets1 = {2, 0, 1};
	mapping m1(nets1);

	std::vector<int> nets2 = {5, 4, 3};
	mapping m2(nets2);

	Expression expected = f+e+d;
	Expression reality = a+b+c;
	reality.apply(nets2);
	EXPECT_TRUE(areSame(reality, expected));
	cout << reality << "... vs ..." << endl << expected << endl;

	// Edge case: a lone .top var w/ an empty .sub body should still map
	expected = f;
	reality = a;
	reality.apply(nets2);
	EXPECT_TRUE(areSame(reality, expected));
	cout << reality << "... vs ..." << endl << expected << endl;
}

TEST(Expression, DanglingExpression) {
	// top: e8
	// e8 = e0&e7  (25)
	// e7 = e6&v2  (25)
	// e6 = e5&e5  (25)
	// e5 = true&e4  (25)
	// e4 = e3|v7  (24)
	// e3 = ~v6  (5)
	// e2 = e1|v13  (24)
	// e1 = ~v12  (5)
	// e0 = +true  (1)

	Expression after;
	after.push(Operation::IDENTITY, {Operand::boolOf(true)});
	after.push(Operation::BOOLEAN_NOT, {Operand::varOf(12)});
	after.push(Operation::BOOLEAN_OR, {Operand::exprOf(1), Operand::varOf(13)});
	after.push(Operation::BOOLEAN_NOT, {Operand::varOf(6)});
	after.push(Operation::BOOLEAN_OR, {Operand::exprOf(3), Operand::varOf(7)});
	after.push(Operation::BOOLEAN_AND, {Operand::boolOf(true), Operand::exprOf(4)});
	after.push(Operation::BOOLEAN_AND, {Operand::exprOf(5), Operand::exprOf(5)});
	after.push(Operation::BOOLEAN_AND, {Operand::exprOf(6), Operand::varOf(2)});
	after.push(Operation::BOOLEAN_AND, {Operand::exprOf(0), Operand::exprOf(7)});
	after.top = Operand::exprOf(8);

	Expression before(after);
	cout << before << endl;

	after.minimize();
	cout << after << endl;
	EXPECT_TRUE(!areSame(before, after)) << before << endl << after << endl;
	//TODO: document Expected result to test
}

TEST(Expression, Iteration) {
	Operand oA = Operand::intOf(0);
	Operand oB = Operand::intOf(1);
	Operand oC = Operand::intOf(2);
	Operand oD = Operand::intOf(3);

	Expression eA = Expression(oA);
	Expression eB = Expression(oB);
	Expression eC = Expression(oC);
	Expression eD = Expression(oD);
	Expression eE = eA + eB;
	Expression eF = eC + eD;
	Expression eG = eF - eE;
	cout << eG << endl;

	// Operation indexing is based on order of operands referenced in target expr, eG
	vector<Operation> expected_operations = {
		Operation(Operation::ADD, {oC, oD}, 1),  // eF
		Operation(Operation::ADD, {oA, oB}, 0),  // eE
		Operation(Operation::SUBTRACT, {Operand::exprOf(0), Operand::exprOf(1)}, 2),
	};

	EXPECT_EQ(eG.size(), expected_operations.size());
	for (size_t operation_idx = 0; operation_idx < eG.size(); operation_idx++) {
		const Operation &real_operation = *eG.getExpr(operation_idx);
		EXPECT_EQ(real_operation, expected_operations[operation_idx]);
	}

	size_t operation_idx = 0;
	for (const Operand &operand : eG.exprIndex()) {
		const Operation &real_operation = *eG.getExpr(operand.index);
		EXPECT_EQ(real_operation, expected_operations[operation_idx]);
		operation_idx++;
	}

	Expression expected;
	for (const Operation &operation : expected_operations) {
		expected.pushExpr(operation);
	}
	expected.top = Operand::exprOf(2);
	EXPECT_TRUE(areSame(eG, expected)) << expected << endl;
}

TEST(Expression, BooleanSimplification) {
	Expression x = Expression::varOf(0);
	Expression eTrue = Expression::boolOf(true);
	Expression eFalse = Expression::boolOf(false);

	Expression a = eTrue && x && eTrue;
	a.minimize();
	EXPECT_TRUE(areSame(a, x)) << x << endl << x << endl;

	Expression b = eFalse || x || eFalse;
	b.minimize();
	EXPECT_TRUE(areSame(b, x)) << b << endl << x << endl;

	Expression e = x && x && x && x;
	e.minimize();
	EXPECT_TRUE(areSame(e, x)) << e << endl << x << endl;

	Expression f = x || x || x || x;
	f.minimize();
	EXPECT_TRUE(areSame(f, x)) << f << endl << x << endl;

	Expression g = x && x || x && x || x && x;
	g.minimize();
	EXPECT_TRUE(areSame(g, x)) << g << endl << x << endl;
}

TEST(Expression, ConstantFolding) {
	Expression zero = Expression::intOf(0);
	Expression one = Expression::intOf(1);
	Expression x = Expression::varOf(0);

	Expression a = Expression::intOf(1) || x;
	a.minimize();
	EXPECT_TRUE(areSame(a, one)) << a << endl << one << endl;

	Expression b = Expression::intOf(0) && x;
	b.minimize();
	EXPECT_TRUE(areSame(b, zero)) << b << endl << one << endl;
}

