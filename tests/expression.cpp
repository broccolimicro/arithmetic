#include <gtest/gtest.h>

#include <arithmetic/expression.h>
#include <arithmetic/algorithm.h>
#include <common/text.h>

using namespace arithmetic;
using namespace std;

TEST(Expression, OperandBitwiseOr) {
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
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
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	Operand f = Operand::varOf(4);
	
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
	Operand a = Operand::intOf(4);
	Operand b = Operand::intOf(8);
	Operand c = Operand::intOf(2);
	Operand d = Operand::intOf(12);
	
	Expression e = (a+b)*c-d;
	cout << e << endl;
	e.top.replace(tidy(e, {e.top.index}));
	cout << e << endl;
	ASSERT_EQ(e.operations.size(), 0u);
	EXPECT_EQ(e.top.type, Operand::CONST);
	EXPECT_EQ(e.top.cnst.type, Value::INT);
	EXPECT_EQ(e.top.cnst.ival, 12);
}

TEST(Expression, TidyCommutative) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	
	Expression e = (a+b)+(c+d);
	ASSERT_EQ(e.operations.size(), 3u);
	cout << e << endl;
	e.top.replace(tidy(e, {e.top.index}));
	cout << e << endl;
	ASSERT_EQ(e.operations.size(), 1u);
	EXPECT_EQ(e.top.type, Operand::CONST);
	EXPECT_EQ(e.top.cnst.type, Value::INT);
	EXPECT_EQ(e.top.cnst.ival, 12);
}

TEST(Expression, Simplify) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);

	vector<Type> vars({Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0)});
	
	Expression dut = (a*c+b*c-c*a)/c;
	Cost cost = arithmetic::cost(dut, dut.top, vars);
	cout << "Before: cost=" << cost.complexity << " del=" << cost.critical << endl << dut << endl;
	dut.minimize();
	EXPECT_TRUE(areSame(dut, Expression(b)));

	Cost cost2 = arithmetic::cost(dut, dut.top, vars);
	cout << "After: cost=" << cost2.complexity << " del=" << cost2.critical << endl << dut << endl;
	EXPECT_LE(cost2.complexity, cost.complexity);
	EXPECT_LE(cost2.critical, cost.critical);
}

TEST(Expression, ChainOfAdds) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	Operand e = Operand::varOf(4);
	Operand f = Operand::varOf(5);
	Operand g = Operand::varOf(6);
	
	Expression dut = a+b+c+d+e+f+g;
	dut.minimize();
}

TEST(Expression, ElasticRewrite) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	Operand e = Operand::varOf(4);
	
	Expression rules = arithmetic::array({
		a*add(b) > add(a*b),
	});

	Expression dut = e*add({a,b,c,d});
	auto m = arithmetic::search(dut, rules, rules.top);
	cout << ::to_string(m) << endl;
}

TEST(Expression, Boolean) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);

	//Expression dut = ((true&~a)&~b)|((false|c)|d);
	//Expression dut = ((false|c)|d);
	Expression dut = (false|c)|d;
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}

TEST(Expression, Identity) {
	Operand a = Operand::varOf(0);

	Expression dut = a;
	dut.push(Operation::BOOLEAN_NOT, {dut.top});
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}

TEST(Expression, Function) {
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	Operand e = Operand::varOf(4);
	Operand f = Operand::varOf(5);

	Expression dut = call(0, {a+b, c, d+e+f});
	cout << dut << endl;
	dut.minimize();
	cout << dut << endl;
}
