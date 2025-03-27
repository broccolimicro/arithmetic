#include <gtest/gtest.h>

#include <arithmetic/expression.h>

using namespace arithmetic;
using namespace std;

TEST(expression, operand_bitwise_or)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x||y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(value(xval));
	s.push_back(value(yval));

	value result = e.evaluate(s);

	EXPECT_EQ(result.data, xval | yval);
}

TEST(expression, operand_bitwise_and)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x&&y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(value(xval));
	s.push_back(value(yval));

	value result = e.evaluate(s);

	EXPECT_EQ(result.data, xval & yval);
}

TEST(expression, operand_bitwise_xor)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x^y;

	state s;
	int xval = rand()%100;
	int yval = rand()%100;

	s.push_back(value(xval));
	s.push_back(value(yval));

	value result = e.evaluate(s);

	EXPECT_EQ(result.data, xval ^ yval);
}

TEST(expression, operand_equal_to)
{
	int valid = value::valid;
	int neutral = value::neutral;
	int unstable = value::unstable;

	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x==y;

	state s;
	s.push_back(value(5));
	s.push_back(value(4));
	value result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);

	s.clear();
	s.push_back(value(5));
	s.push_back(value(5));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, valid);

	s.clear();
	s.push_back(value(unstable));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, unstable);

	s.clear();
	s.push_back(value(neutral));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);
}

TEST(expression, operand_not_equal_to)
{
	int valid = value::valid;
	int neutral = value::neutral;
	int unstable = value::unstable;

	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x!=y;

	state s;
	s.push_back(value(5));
	s.push_back(value(4));
	value result = e.evaluate(s);
	EXPECT_EQ(result.data, valid);

	s.clear();	
	s.push_back(value(5));
	s.push_back(value(5));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);

	s.clear();
	s.push_back(value(unstable));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, unstable);

	s.clear();
	s.push_back(value(neutral));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);
}

TEST(expression, operand_less_than)
{
	int valid = value::valid;
	int neutral = value::neutral;
	int unstable = value::unstable;

	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x<y;

	state s;
	s.push_back(value(5));
	s.push_back(value(4));
	value result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);

	s.clear();	
	s.push_back(value(4));
	s.push_back(value(5));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, valid);

	s.clear();
	s.push_back(value(unstable));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, unstable);

	s.clear();
	s.push_back(value(neutral));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);
}

TEST(expression, operand_greater_than)
{
	int valid = value::valid;
	int neutral = value::neutral;
	int unstable = value::unstable;

	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x>y;

	state s;
	s.push_back(value(5));
	s.push_back(value(4));
	value result = e.evaluate(s);
	EXPECT_EQ(result.data, valid);

	s.clear();	
	s.push_back(value(4));
	s.push_back(value(5));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);

	s.clear();
	s.push_back(value(unstable));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, unstable);

	s.clear();
	s.push_back(value(neutral));
	s.push_back(value(4));
	result = e.evaluate(s);
	EXPECT_EQ(result.data, neutral);
}

TEST(expression, compound)
{
	operand a(0, operand::variable);
	operand b(1, operand::variable);
	operand c(2, operand::variable);
	operand d(3, operand::variable);
	operand f(4, operand::variable);
	
	expression e = (a+b)*c-d%f;

	state s;
	s.push_back(value(5));
	s.push_back(value(4));
	s.push_back(value(3));
	s.push_back(value(6));
	s.push_back(value(3));
	value result = e.evaluate(s);
	EXPECT_EQ(result.data, 27);
}

