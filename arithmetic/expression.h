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
		neutral = 0,
		unstable = 1,
		unknown = 2,
		valid = 3,
		constant = 4,
		expression = 5,
		variable = 6
	};

	value get(state values, vector<value> expressions) const;
	void set(state &values, vector<value> &expressions, value v) const;
};

ostream &operator<<(ostream &os, operand o);

bool are_same(operand o0, operand o1);

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
	value evaluate(state values, vector<value> expressions) const;
	void propagate(state &result, const state &global, vector<value> &expressions, const vector<value> gexpressions, value v) const;
};

struct expression
{
	expression();
	expression(bool v);
	expression(value v);
	expression(operand arg0);
	expression(string func, operand arg0);
	expression(string func, expression arg0);
	expression(string func, operand arg0, operand arg1);
	expression(string func, expression arg0, expression arg1);
	expression(string func, expression arg0, operand arg1);
	expression(string func, operand arg0, expression arg1);
	expression(string func, vector<expression> args);
	~expression();

	static expression rewrite;
	static vector<int> rewrite_top;

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

	static void init_rewrite();

	int find(operation arg);
	int push(operation arg);

	void set(operand arg0);
	void set(string func, operand arg0);
	void set(string func, expression arg0);
	void set(string func, operand arg0, operand arg1);
	void set(string func, expression arg0, expression arg1);
	void set(string func, expression arg0, operand arg1);
	void set(string func, operand arg0, expression arg1);
	void set(string func, vector<expression> args);
	void push(string func);
	void push(string func, operand arg0);
	void push(string func, expression arg0);
	value evaluate(state values) const;
	bool is_null() const;
	bool is_constant() const;
	bool is_valid() const;
	bool is_neutral() const;
	bool is_wire() const;

	expression &operator=(operand e);

	string to_string(int index = -1);
};

ostream &operator<<(ostream &os, expression e);

void minimize(expression &F);

expression operator~(expression e);
expression operator-(expression e);
expression is_valid(expression e);
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

int passes_guard(const state &encoding, const state &global, const expression &guard, state *total);
expression weakest_guard(const expression &guard, const expression &exclude);

}
