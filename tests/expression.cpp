#include <gtest/gtest.h>

#include <arithmetic/expression.h>

using namespace arithmetic;
using namespace std;

TEST(Expression, OperandBitwiseOr) {
	Operation::loadOperators();
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x||y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	value result = e.evaluate(s);

	EXPECT_EQ(result.type, value::INT);
	EXPECT_EQ(result.ival, xval | yval);
}

TEST(Expression, OperandBitwiseAnd) {
	Operation::loadOperators();
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x&&y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	value result = e.evaluate(s);

	EXPECT_EQ(result.type, value::INT);
	EXPECT_EQ(result.ival, xval & yval);
}

TEST(Expression, OperandBitwiseXor) {
	Operation::loadOperators();
	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x^y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(xval);
	s.push_back(yval);

	value result = e.evaluate(s);

	EXPECT_EQ(result.type, value::INT);
	EXPECT_EQ(result.ival, xval ^ yval);
}

TEST(Expression, OperandEqualTo) {
	Operation::loadOperators();
	int valid = value::VALID;
	int neutral = value::NEUTRAL;
	int unstable = value::UNSTABLE;

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x==y;

	state s;
	s.push_back(5);
	s.push_back(4);
	value result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(5);
	s.push_back(5);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();
	s.push_back(value::X());
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandNotEqualTo) {
	Operation::loadOperators();
	int valid = value::VALID;
	int neutral = value::NEUTRAL;
	int unstable = value::UNSTABLE;

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x!=y;

	state s;
	s.push_back(5);
	s.push_back(4);
	value result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();	
	s.push_back(5);
	s.push_back(5);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(value::X());
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandLessThan) {
	Operation::loadOperators();
	int valid = value::VALID;
	int neutral = value::NEUTRAL;
	int unstable = value::UNSTABLE;

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x<y;

	state s;
	s.push_back(5);
	s.push_back(4);
	value result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();
	s.push_back(value::X());
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, OperandGreaterThan) {
	Operation::loadOperators();
	int valid = value::VALID;
	int neutral = value::NEUTRAL;
	int unstable = value::UNSTABLE;

	Operand x = Operand::varOf(0);
	Operand y = Operand::varOf(1);
	
	Expression e = x>y;

	state s;
	s.push_back(5);
	s.push_back(4);
	value result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, valid);

	s.clear();	
	s.push_back(4);
	s.push_back(5);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);

	s.clear();
	s.push_back(value::X());
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, unstable);

	s.clear();
	s.push_back(false);
	s.push_back(4);
	result = e.evaluate(s);
	EXPECT_EQ(result.type, value::BOOL);
	EXPECT_EQ(result.bval, neutral);
}

TEST(Expression, Compound) {
	Operation::loadOperators();
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);
	Operand d = Operand::varOf(3);
	Operand f = Operand::varOf(4);
	
	Expression e = (a+b)*c-d%f;

	state s;
	s.push_back(5);
	s.push_back(4);
	s.push_back(3);
	s.push_back(6);
	s.push_back(3);
	value result = e.evaluate(s);
	EXPECT_EQ(result.type, value::INT);
	EXPECT_EQ(result.ival, 27);
}

TEST(Expression, PropagateConstants) {
	Operation::loadOperators();
	Operand a = Operand::intOf(4);
	Operand b = Operand::intOf(8);
	Operand c = Operand::intOf(2);
	Operand d = Operand::intOf(12);
	
	Expression e = (a+b)*c-d;
	e.propagate_constants();
	ASSERT_EQ(e.operations.size(), 1u);
	ASSERT_EQ(e.operations[0].operands.size(), 1u);
	EXPECT_EQ(e.operations[0].operands[0].type, Operand::CONST);
	EXPECT_EQ(e.operations[0].operands[0].cnst.type, value::INT);
	EXPECT_EQ(e.operations[0].operands[0].cnst.ival, 12);
}

TEST(Expression, Simplify) {
	Operation::loadOperators();
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);

	vector<Type> vars({Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0), Type(1.0, 16.0, 0.0)});
	
	Expression dut = a*c+b*c-c*a;
	Cost cost = dut.cost(vars);
	cout << "Before: cost=" << cost.complexity << " del=" << cost.critical << endl << dut << endl;
	dut.minimize();
	Cost cost2 = dut.cost(vars);
	cout << "After: cost=" << cost2.complexity << " del=" << cost2.critical << endl << dut << endl;
	EXPECT_LE(cost2.complexity, cost.complexity);
	EXPECT_LE(cost2.critical, cost.critical);
}

TEST(Expression, ChainOfAdds) {
	Operation::loadOperators();
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


