/*
 * action.h
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#pragma once

#include "expression.h"

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

struct parallel
{
	parallel();
	parallel(expression expr);
	parallel(int variable, expression expr);
	parallel(int channel, int variable, expression expr);
	parallel(int channel, expression expr, int variable);
	~parallel();

	vector<action> actions;

	parallel remote(vector<vector<int> > groups);

	action &operator[](int index);
	const action &operator[](int index) const;
};

value local_assign(const vector<value> &s, parallel c, bool stable);
value remote_assign(const vector<value> &s, parallel c, bool stable);

struct choice
{
	choice();
	choice(parallel c);
	~choice();

	vector<parallel> terms;

	choice remote(vector<vector<int> > groups);

	parallel &operator[](int index);
	const parallel &operator[](int index) const;
};

}

