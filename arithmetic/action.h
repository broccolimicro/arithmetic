/*
 * action.h
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#include "expression.h"

#ifndef arithmetic_action_h
#define arithmetic_action_h

namespace arithmetic
{

struct action
{
	action();
	action(expression expr);
	action(int variable, expression expr);
	action(int channel, int variable, expression expr);
	action(int channel, expression expr, int variable);
	~action();

	int variable;
	int channel;
	expression expr;
	int behavior;

	enum
	{
		assign = 0,
		send = 1,
		receive = 2
	};
};

value local_assign(const vector<value> &s, action a, bool stable);
value remote_assign(const vector<value> &s, action a, bool stable);

struct cube
{
	cube();
	cube(expression expr);
	cube(int variable, expression expr);
	cube(int channel, int variable, expression expr);
	cube(int channel, expression expr, int variable);
	~cube();

	vector<action> actions;

	cube remote(vector<vector<int> > groups);
};

value local_assign(const vector<value> &s, cube c, bool stable);
value remote_assign(const vector<value> &s, cube c, bool stable);

struct cover
{
	cover();
	cover(cube c);
	~cover();

	vector<cube> cubes;

	cover remote(vector<vector<int> > groups);
};

}

#endif
