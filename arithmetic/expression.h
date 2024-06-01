/*
 * expression.h
 *
 *  Created on: Jun 25, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include "state.h"

namespace arithmetic
{

struct operand
{
	operand();
	operand(int index, int type);
	operand(value v);
	~operand();

	int index;
	int type;

	enum
	{
		invalid = 0,
		unstable = 1,
		unknown = 2,
		constant = 3,
		expression = 4,
		variable = 5
	};

	value get(vector<value> values, vector<value> expressions) const;
};

ostream &operator<<(ostream &os, operand o);

struct operation
{
	operation();
	operation(string func, operand arg0);
	operation(string func, operand arg0, operand arg1);
	operation(string func, operand arg0, operand arg1, operand arg2);
	operation(string func, vector<operand> args);
	~operation();

	vector<operand> operands;
	int func;

	void set(string func, operand arg0);
	void set(string func, operand arg0, operand arg1);
	void set(string func, operand arg0, operand arg1, operand arg2);
	void set(string func, vector<operand> args);
	string get() const;
	value evaluate(vector<value> values, vector<value> expressions) const;
};

struct expression
{
	expression();
	expression(operand arg0);
	expression(string func, operand arg0);
	expression(string func, expression arg0);
	expression(string func, operand arg0, operand arg1);
	expression(string func, expression arg0, expression arg1);
	expression(string func, expression arg0, operand arg1);
	expression(string func, operand arg0, expression arg1);
	expression(string func, vector<expression> args);
	~expression();

	// The expression consists entirely of a tree of operations stored in an
	// array. Operations are stored in the array in order of precedence. So if
	// the expression is (a+b)*3 + x*y, then they'll be stored in the following order:
	// 0. a+b
	// 1. operations[0]*3
	// 2. x*y
	// 3. operations[1]+operations[2]
	// Therefore the final operation stored is the operation that produces the
	// final value for the expression.
	vector<operation> operations;

	void set(operand arg0);
	void set(string func, operand arg0);
	void set(string func, expression arg0);
	void set(string func, operand arg0, operand arg1);
	void set(string func, expression arg0, expression arg1);
	void set(string func, expression arg0, operand arg1);
	void set(string func, operand arg0, expression arg1);
	void set(string func, vector<expression> args);
	value evaluate(vector<value> values) const;
	bool is_constant() const;

	expression &operator=(operand e);

	string to_string(int index = -1);
};

ostream &operator<<(ostream &os, expression e);

expression operator~(expression e);
expression operator-(expression e);
expression operator!(expression e);
expression operator|(expression e0, expression e1);
expression operator&(expression e0, expression e1);
expression operator^(expression e0, expression e1);
expression operator==(expression e0, expression e1);
expression operator!=(expression e0, expression e1);
expression operator<(expression e0, expression e1);
expression operator>(expression e0, expression e1);
expression operator<=(expression e0, expression e1);
expression operator>=(expression e0, expression e1);
expression operator<<(expression e0, expression e1);
expression operator>>(expression e0, expression e1);
expression operator+(expression e0, expression e1);
expression operator-(expression e0, expression e1);
expression operator*(expression e0, expression e1);
expression operator/(expression e0, expression e1);
expression operator%(expression e0, expression e1);
expression operator&&(expression e0, expression e1);
expression operator||(expression e0, expression e1);

expression operator|(expression e0, operand e1);
expression operator&(expression e0, operand e1);
expression operator^(expression e0, operand e1);
expression operator==(expression e0, operand e1);
expression operator!=(expression e0, operand e1);
expression operator<(expression e0, operand e1);
expression operator>(expression e0, operand e1);
expression operator<=(expression e0, operand e1);
expression operator>=(expression e0, operand e1);
expression operator<<(expression e0, operand e1);
expression operator>>(expression e0, operand e1);
expression operator+(expression e0, operand e1);
expression operator-(expression e0, operand e1);
expression operator*(expression e0, operand e1);
expression operator/(expression e0, operand e1);
expression operator%(expression e0, operand e1);
expression operator&&(expression e0, operand e1);
expression operator||(expression e0, operand e1);

expression operator|(operand e0, expression e1);
expression operator&(operand e0, expression e1);
expression operator^(operand e0, expression e1);
expression operator==(operand e0, expression e1);
expression operator!=(operand e0, expression e1);
expression operator<(operand e0, expression e1);
expression operator>(operand e0, expression e1);
expression operator<=(operand e0, expression e1);
expression operator>=(operand e0, expression e1);
expression operator<<(operand e0, expression e1);
expression operator>>(operand e0, expression e1);
expression operator+(operand e0, expression e1);
expression operator-(operand e0, expression e1);
expression operator*(operand e0, expression e1);
expression operator/(operand e0, expression e1);
expression operator%(operand e0, expression e1);
expression operator&&(operand e0, expression e1);
expression operator||(operand e0, expression e1);

expression operator|(operand e0, operand e1);
expression operator&(operand e0, operand e1);
expression operator^(operand e0, operand e1);
expression operator==(operand e0, operand e1);
expression operator!=(operand e0, operand e1);
expression operator<(operand e0, operand e1);
expression operator>(operand e0, operand e1);
expression operator<=(operand e0, operand e1);
expression operator>=(operand e0, operand e1);
expression operator<<(operand e0, operand e1);
expression operator>>(operand e0, operand e1);
expression operator+(operand e0, operand e1);
expression operator-(operand e0, operand e1);
expression operator*(operand e0, operand e1);
expression operator/(operand e0, operand e1);
expression operator%(operand e0, operand e1);
expression operator&&(operand e0, operand e1);
expression operator||(operand e0, operand e1);

}
