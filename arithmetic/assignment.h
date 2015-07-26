/*
 * assignment.h
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#include "expression.h"

#ifndef arithmetic_assignment_h
#define arithmetic_assignment_h

namespace arithmetic
{

struct assignment
{
	assignment();
	assignment(expression expr);
	assignment(int variable, expression expr);
	assignment(int channel, int variable, expression expr);
	assignment(int channel, expression expr, int variable);
	~assignment();

	int variable;
	int channel;
	expression expr;
	int behavior;

	enum
	{
		guard = 0,
		assign = 1,
		send = 2,
		receive = 3
	};
};

}

#endif
