#include <gtest/gtest.h>

#include <arithmetic/algorithm.h>
#include <arithmetic/expression.h>
#include <common/mapping.h>
#include <common/text.h>

using namespace arithmetic;
using namespace std;

TEST(Expression, OperandBitwiseOr) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x||y;
	cout << e << endl;

	State s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::INT);
	EXPECT_EQ(result.ival, xval | yval);
}

TEST(Expression, OperandBitwiseAnd) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x&&y;
	cout << e << endl;

	State s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::INT);
	EXPECT_EQ(result.ival, xval & yval);
}

TEST(Expression, OperandBitwiseXor) {
	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x^y;
	cout << e << endl;

	State s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	Value result = evaluate(e, e.top, s);

	EXPECT_EQ(result.type, Value::INT);
	EXPECT_EQ(result.ival, xval ^ yval);
}

TEST(Expression, OperandEqualTo) {
	int valid = Value::VALID;
	int neutral = Value::NEUTRAL;
	int unstable = Value::UNSTABLE;

	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x==y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(5);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandNotEqualTo) {
	int valid = Value::VALID;
	int neutral = Value::NEUTRAL;
	int unstable = Value::UNSTABLE;

	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x!=y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();	
	s.push_back(5);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandLessThan) {
	int valid = Value::VALID;
	int neutral = Value::NEUTRAL;
	int unstable = Value::UNSTABLE;

	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x<y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandGreaterThan) {
	int valid = Value::VALID;
	int neutral = Value::NEUTRAL;
	int unstable = Value::UNSTABLE;

	Expression x = Expression::varOf(0);
	Expression y = Expression::varOf(1);
	
	Expression e = x>y;
	cout << e << endl;

	State s;
	s.push_back(5);
	s.push_back(4);
	Value result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(Value::X());
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = evaluate(e, e.top, s);
	EXPECT_EQ(result.type, Value::BOOL);
	EXPECT_EQ(result.bval, neutral);
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
	cout << "Before: cost=" << cost.complexity << " del=" << cost.critical << endl << dut << endl;
	dut.top = minimize(dut, {dut.top}).map(dut.top);
	EXPECT_TRUE(areSame(dut, b));

	Cost cost2 = arithmetic::cost(dut, dut.top, vars);
	cout << "After: cost=" << cost2.complexity << " del=" << cost2.critical << endl << dut << endl;
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
	dut.top = minimize(dut, {dut.top}).map(dut.top);
}

TEST(Expression, ElasticRewrite) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);
	Expression e = Expression::varOf(4);
	
	Expression rules = arithmetic::array({
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

	//Expression dut = ((true&~a)&~b)|((false|c)|d);
	//Expression dut = ((false|c)|d);
	Expression dut = (false|c)|d;
	cout << dut << endl;
	dut.top = minimize(dut, {dut.top}).map(dut.top);
	cout << dut << endl;
}

TEST(Expression, Identity) {
	Expression a = Expression::varOf(0);

	Expression dut = a;
	dut.push(Operation::OpType::TYPE_BOOLEAN_NOT, {dut.top});
	cout << dut << endl;
	dut.top = minimize(dut, {dut.top}).map(dut.top);
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
	dut.top = minimize(dut, {dut.top}).map(dut.top);
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

	Expression dut;
	dut.push(Operation::IDENTITY, {Operand::boolOf(true)});
	dut.push(Operation::BOOLEAN_NOT, {Operand::varOf(12)});
	dut.push(Operation::BOOLEAN_OR, {Operand::exprOf(1), Operand::varOf(13)});
	dut.push(Operation::BOOLEAN_NOT, {Operand::varOf(6)});
	dut.push(Operation::BOOLEAN_OR, {Operand::exprOf(3), Operand::varOf(7)});
	dut.push(Operation::BOOLEAN_AND, {Operand::boolOf(true), Operand::exprOf(4)});
	dut.push(Operation::BOOLEAN_AND, {Operand::exprOf(5), Operand::exprOf(5)});
	dut.push(Operation::BOOLEAN_AND, {Operand::exprOf(6), Operand::varOf(2)});
	dut.push(Operation::BOOLEAN_AND, {Operand::exprOf(0), Operand::exprOf(7)});
	dut.top = Operand::exprOf(8);

	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}
