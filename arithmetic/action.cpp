/*
 * action.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#include "action.h"

namespace arithmetic
{

action::action()
{
	behavior = -1;
	variable = -1;
	channel = -1;
}

action::action(expression expr)
{
	this->variable = -1;
	this->channel = -1;
	this->expr = expr;
	this->behavior = assign;
}

action::action(int variable, expression expr)
{
	this->channel = -1;
	this->variable = variable;
	this->behavior = assign;
	this->expr = expr;
}

action::action(int channel, int variable, expression expr)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = receive;
}

action::action(int channel, expression expr, int variable)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = send;
}

action::~action()
{

}

value local_assign(const vector<value> &s, action a, bool stable)
{
	if (a.behavior == action::assign && a.variable >= 0)
	{
		value v = a.expr.evaluate(s);

		if (stable || v.data == s[a.variable].data)
			return v;
	}

	return value(value::unstable);
}

value remote_assign(const vector<value> &s, action a, bool stable)
{
	if (a.behavior == action::assign && a.variable >= 0)
	{
		value v = a.expr.evaluate(s);

		if (v.data == s[a.variable].data)
			return v;
		else if (stable)
			return value(value::unknown);
	}

	return value(value::unstable);
}

cube::cube()
{

}

cube::cube(expression expr)
{
	actions.push_back(action(expr));
}

cube::cube(int variable, expression expr)
{
	actions.push_back(action(variable, expr));
}

cube::cube(int channel, int variable, expression expr)
{
	actions.push_back(action(channel, variable, expr));
}

cube::cube(int channel, expression expr, int variable)
{
	actions.push_back(action(channel, expr, variable));
}

cube::~cube()
{

}

cube cube::remote(vector<vector<int> > groups)
{
	cube result = *this;
	for (int i = (int)result.actions.size()-1; i >= 0; i--)
	{
		if (result.actions[i].variable >= 0)
		{
			for (int j = 0; j < (int)groups.size(); j++)
			{
				if (find(groups[j].begin(), groups[j].end(), result.actions[i].variable) != groups[j].end())
				{
					for (int k = 0; k < (int)groups[j].size(); k++)
						if (groups[j][k] != result.actions[i].variable)
						{
							result.actions.push_back(result.actions[i]);
							result.actions.back().variable = groups[j][k];
						}
				}
			}
		}
	}

	return result;
}

void local_assign(vector<value> &s, cube c, bool stable)
{
	map<int, value> sent;
	map<int, value> recv;
	// Determine the value for the data being sent in either the request or the acknowledge
	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::send)
		{
			value v = c.actions[i].expr.evaluate(s);
			map<int, value>::iterator loc = sent.find(c.actions[i].channel);
			if (loc == sent.end())
				sent.insert(pair<int, value>(c.actions[i].channel, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
		else if (c.actions[i].behavior == action::receive)
		{
			value v = c.actions[i].expr.evaluate(s);
			map<int, value>::iterator loc = recv.find(c.actions[i].channel);
			if (loc == recv.end())
				recv.insert(pair<int, value>(c.actions[i].channel, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
	}

	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::send && c.actions[i].variable >= 0)
		{
			c.actions[i].behavior = action::assign;
			map<int, value>::iterator loc = recv.find(c.actions[i].channel);
			if (loc == recv.end())
				c.actions[i].expr = operand(value(value::invalid));
			else
				c.actions[i].expr = operand(loc->second);
		}
		else if (c.actions[i].behavior == action::receive && c.actions[i].variable >= 0)
		{
			c.actions[i].behavior = action::assign;
			map<int, value>::iterator loc = sent.find(c.actions[i].channel);
			if (loc == sent.end())
				c.actions[i].expr = operand(value(value::invalid));
			else
				c.actions[i].expr = operand(loc->second);
		}
	}

	map<int, value> assigned;
	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::assign && c.actions[i].variable >= 0)
		{
			value v = local_assign(s, c.actions[i], stable);
			map<int, value>::iterator loc = assigned.find(c.actions[i].variable);
			if (loc == assigned.end())
				assigned.insert(pair<int, value>(c.actions[i].variable, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
	}

	for (map<int, value>::iterator i = assigned.begin(); i != assigned.end(); i++)
		s[i->first] = i->second;
}

void remote_assign(vector<value> &s, cube c, bool stable)
{
	map<int, value> sent;
	map<int, value> recv;
	// Determine the value for the data being sent in either the request or the acknowledge
	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::send)
		{
			value v = c.actions[i].expr.evaluate(s);
			map<int, value>::iterator loc = sent.find(c.actions[i].channel);
			if (loc == sent.end())
				sent.insert(pair<int, value>(c.actions[i].channel, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
		else if (c.actions[i].behavior == action::receive)
		{
			value v = c.actions[i].expr.evaluate(s);
			map<int, value>::iterator loc = recv.find(c.actions[i].channel);
			if (loc == recv.end())
				recv.insert(pair<int, value>(c.actions[i].channel, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
	}

	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::send && c.actions[i].variable >= 0)
		{
			c.actions[i].behavior = action::assign;
			map<int, value>::iterator loc = recv.find(c.actions[i].channel);
			if (loc == recv.end())
				c.actions[i].expr = operand(value(value::invalid));
			else
				c.actions[i].expr = operand(loc->second);
		}
		else if (c.actions[i].behavior == action::receive && c.actions[i].variable >= 0)
		{
			c.actions[i].behavior = action::assign;
			map<int, value>::iterator loc = sent.find(c.actions[i].channel);
			if (loc == sent.end())
				c.actions[i].expr = operand(value(value::invalid));
			else
				c.actions[i].expr = operand(loc->second);
		}
	}

	map<int, value> assigned;
	for (int i = 0; i < (int)c.actions.size(); i++)
	{
		if (c.actions[i].behavior == action::assign && c.actions[i].variable >= 0)
		{
			value v = remote_assign(s, c.actions[i], stable);
			map<int, value>::iterator loc = assigned.find(c.actions[i].variable);
			if (loc == assigned.end())
				assigned.insert(pair<int, value>(c.actions[i].variable, v));
			else if (loc->second.data != v.data)
				loc->second.data = value::unstable;
		}
	}

	for (map<int, value>::iterator i = assigned.begin(); i != assigned.end(); i++)
		s[i->first] = i->second;
}

cover::cover()
{

}

cover::cover(cube c)
{
	cubes.push_back(c);
}

cover::~cover()
{

}

cover cover::remote(vector<vector<int> > groups)
{
	cover result;
	for (int i = 0; i < (int)cubes.size(); i++)
		result.cubes.push_back(cubes[i].remote(groups));
	return result;
}

}
