#include <gtest/gtest.h>

#include <arithmetic/expression.h>

using namespace arithmetic;
using namespace std;

TEST(expression, operand_bitwise_or)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x|y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval | yval);
}

TEST(expression, operand_bitwise_and)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x&y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval & yval);
}

TEST(expression, operand_bitwise_xor)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x^y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval ^ yval);
}

TEST(expression, operand_equal_to)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x==y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval == yval);
}

TEST(expression, operand_not_equal_to)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x!=y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval != yval);
}

TEST(expression, operand_less_than)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x<y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval < yval);
}

TEST(expression, operand_greater_than)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	
	expression e = x>y;

	vector<value> state;
	int xval = rand()%100;
	int yval = rand()%100;

	state.push_back(value(xval));
	state.push_back(value(yval));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, xval > yval);
}

