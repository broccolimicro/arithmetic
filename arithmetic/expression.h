/*
 * expression.h
 *
 *  Created on: Jun 25, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#ifndef chp_expression_h
#define chp_expression_h

namespace arithmetic
{

struct value
{
	value();
	value(int data);
	~value();

	int data;

	const static int invalid  = 0x80000000;
	const static int unstable = 0x80000001;
	const static int unknown  = 0x80000002;
	const static int valid	  = 0x80000003;
};

value operator~(value v);
value operator-(value v);
value operator!(value v);
value operator|(value v0, value v1);
value operator&(value v0, value v1);
value operator^(value v0, value v1);
value operator<<(value v0, value v1);
value operator>>(value v0, value v1);
value operator+(value v0, value v1);
value operator-(value v0, value v1);
value operator*(value v0, value v1);
value operator/(value v0, value v1);
value operator%(value v0, value v1);

value operator==(value v0, value v1);
value operator!=(value v0, value v1);
value operator<(value v0, value v1);
value operator>(value v0, value v1);
value operator<=(value v0, value v1);
value operator>=(value v0, value v1);
value operator&&(value v0, value v1);
value operator||(value v0, value v1);

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
};

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

#endif
