#include <gtest/gtest.h>

#include <arithmetic/expression.h>

using namespace arithmetic;
using namespace std;

TEST(Expression, Operand_bitwise_or)
{
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

TEST(Expression, Operand_bitwise_and)
{
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

TEST(Expression, Operand_bitwise_xor)
{
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

TEST(Expression, Operand_equal_to)
{
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

TEST(Expression, Operand_not_equal_to)
{
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

TEST(Expression, OperandLessThan)
{
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

TEST(Expression, OperandGreaterThan)
{
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

TEST(Expression, Compound)
{
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

TEST(Expression, PropagateConstants)
{
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

