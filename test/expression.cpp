#include <gtest/gtest.h>

#include <arithmetic/expression.h>

using namespace arithmetic;
using namespace std;

TEST(expression, test_name)
{
	operand x(0, operand::variable);
	operand y(1, operand::variable);
	operand z(2, operand::variable);
	
	expression e = x+y*z;

	vector<value> state;
	state.push_back(value(3));
	state.push_back(value(4));
	state.push_back(value(5));

	value result = e.evaluate(state);

	EXPECT_EQ(result.data, 23);
}

