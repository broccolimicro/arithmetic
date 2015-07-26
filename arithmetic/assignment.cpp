/*
 * assignment.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#include "assignment.h"

namespace arithmetic
{

assignment::assignment()
{
	behavior = -1;
	variable = -1;
	channel = -1;
}

assignment::assignment(expression expr)
{
	this->variable = -1;
	this->channel = -1;
	this->expr = expr;
	this->behavior = guard;
}

assignment::assignment(int variable, expression expr)
{
	this->channel = -1;
	this->variable = variable;
	this->behavior = assign;
	this->expr = expr;
}

assignment::assignment(int channel, int variable, expression expr)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = receive;
}

assignment::assignment(int channel, expression expr, int variable)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = send;
}

assignment::~assignment()
{

}

}
