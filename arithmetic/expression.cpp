/*
 * expression.cpp
 *
 *  Created on: Jul 1, 2015
 *      Author: nbingham
 */

#include "expression.h"

#include <sstream>

namespace arithmetic
{

operand::operand()
{
	index = 0;
	type = neutral;
}

operand::operand(int index, int type)
{
	this->index = index;
	this->type = type;
}

operand::operand(value v)
{
	if (v.data == value::neutral)
		type = neutral;
	else if (v.data == value::unstable)
		type = unstable;
	else if (v.data == value::unknown)
		type = unknown;
	else if (v.data >= value::valid)
	{
		index = v.data;
		type = constant;
	}
}

operand::~operand()
{
}

ostream &operator<<(ostream &os, operand o)
{
	if (o.type == operand::neutral) {
		os << "N";
	} else if (o.type == operand::unstable) {
		os << "X";
	} else if (o.type == operand::unknown) {
		os << "-";
	} else if (o.type == operand::constant) {
		os << o.index;
	} else if (o.type == operand::expression) {
		os << "e" << o.index;
	} else if (o.type == operand::variable) {
		os << "v" << o.index;
	}
	return os;
}

value operand::get(state values, vector<value> expressions) const
{
	switch (type)
	{
	case neutral:		return value(value::neutral);
	case unstable:		return value(value::unstable);
	case unknown:		return value(value::unknown);
	case constant:		return value(index);
	case expression:
		if (index >= 0 && index < (int)expressions.size())
			return expressions[index];
		else
			return value(value::neutral);
	case variable:
		if (index >= 0 && index < (int)values.size())
			return values[index];
		else
			return value(value::neutral);
	default:			return value(value::unstable);
	}
}

void operand::set(state &values, vector<value> &expressions, value v) const {
	if (type == expression and index >= 0 and index < (int)expressions.size()) {
		expressions[index] = v;
	} else if (type == variable and index >= 0 and index < (int)values.size()) {
		values[index] = v;
	}
}

operation::operation()
{
	func = -1;
}

operation::operation(string func, operand arg0)
{
	set(func, arg0);
}

operation::operation(string func, operand arg0, operand arg1)
{
	set(func, arg0, arg1);
}

operation::operation(string func, operand arg0, operand arg1, operand arg2)
{
	set(func, arg0, arg1, arg2);
}

operation::operation(string func, vector<operand> args)
{
	set(func, args);
}

operation::~operation()
{

}

void operation::set(string func, operand arg0)
{
	operands.push_back(arg0);
	this->func = -1;

	if (func == "~")
		this->func = 0;
	else if (func == "+")
		this->func = 1;
	else if (func == "-")
		this->func = 2;
	else if (func == "!")
		this->func = 3;
}

void operation::set(string func, operand arg0, operand arg1)
{
	operands.push_back(arg0);
	operands.push_back(arg1);
	this->func = -1;

	if (func == "|")
		this->func = 4;
	else if (func == "&")
		this->func = 5;
	else if (func == "^")
		this->func = 6;
	else if (func == "==")
		this->func = 7;
	else if (func == "~=")
		this->func = 8;
	else if (func == "<")
		this->func = 9;
	else if (func == ">")
		this->func = 10;
	else if (func == "<=")
		this->func = 11;
	else if (func == ">=")
		this->func = 12;
	else if (func == "<<")
		this->func = 13;
	else if (func == ">>")
		this->func = 14;
	else if (func == "+")
		this->func = 15;
	else if (func == "-")
		this->func = 16;
	else if (func == "*")
		this->func = 17;
	else if (func == "/")
		this->func = 18;
	else if (func == "%")
		this->func = 19;
	else if (func == "?")
		this->func = 20;
	else if (func == "&&")
		this->func = 21;
	else if (func == "||")
		this->func = 22;

}

void operation::set(string func, operand arg0, operand arg1, operand arg2)
{
	operands.push_back(arg0);
	operands.push_back(arg1);
	operands.push_back(arg2);
	this->func = -1;
}

void operation::set(string func, vector<operand> args)
{
	if (args.size() == 1)
		set(func, args[0]);
	else if (args.size() == 2)
		set(func, args[0], args[1]);
	else if (args.size() == 3)
		set(func, args[0], args[1], args[2]);
}

string operation::get() const
{
	switch (func)
	{
	case 0: return "~";
	case 1: return "+";
	case 2: return "-";
	case 3: return "!";
	case 4: return "|";
	case 5: return "&";
	case 6: return "^";
	case 7: return "==";
	case 8: return "!=";
	case 9: return "<";
	case 10: return ">";
	case 11: return "<=";
	case 12: return ">=";
	case 13: return "<<";
	case 14: return ">>";
	case 15: return "+";
	case 16: return "-";
	case 17: return "*";
	case 18: return "/";
	case 19: return "%";
	case 20: return "&&";
	case 21: return "||";
	default: return "";
	}
}

value operation::evaluate(state values, vector<value> expressions) const
{
	vector<value> args;
	args.reserve(operands.size());
	for (int i = 0; i < (int)operands.size(); i++)
		args.push_back(operands[i].get(values, expressions));

	switch (func)
	{
	case 0: //cout << "~" << args[0] << " = " << ~args[0] << endl;
		return ~args[0];
	case 1: //cout << "+" << args[0] << " = " << +args[0] << endl;
		return args[0];
	case 2: //cout << "-" << args[0] << " = " << -args[0] << endl;
		return -args[0];
	case 3: //cout << "!" << args[0] << " = " << !args[0] << endl;
		return !args[0];
	case 4: //cout << args[0] << "|" << args[1] << " = " << (args[0] |  args[1]) << endl;
		return (args[0] |  args[1]);
	case 5: //cout << args[0] << "&" << args[1] << " = " << (args[0] &  args[1]) << endl;
		return (args[0] &  args[1]);
	case 6: //cout << args[0] << "^" << args[1] << " = " << (args[0] ^  args[1]) << endl;
		return (args[0] ^  args[1]);
	case 7: //cout << args[0] << "==" << args[1] << " = " << (args[0] ==  args[1]) << endl;
		return (args[0] == args[1]);
	case 8: //cout << args[0] << "!=" << args[1] << " = " << (args[0] !=  args[1]) << endl;
		return (args[0] != args[1]);
	case 9: //cout << args[0] << "<" << args[1] << " = " << (args[0] <  args[1]) << endl;
		return (args[0] <  args[1]);
	case 10: //cout << args[0] << ">" << args[1] << " = " << (args[0] >  args[1]) << endl;
		return (args[0] >  args[1]);
	case 11: //cout << args[0] << "<=" << args[1] << " = " << (args[0] <=  args[1]) << endl;
		return (args[0] <= args[1]);
	case 12: //cout << args[0] << ">=" << args[1] << " = " << (args[0] >=  args[1]) << endl;
		return (args[0] >= args[1]);
	case 13: //cout << args[0] << "<<" << args[1] << " = " << (args[0] <<  args[1]) << endl;
		return (args[0] << args[1]);
	case 14: //cout << args[0] << ">>" << args[1] << " = " << (args[0] >>  args[1]) << endl;
		return (args[0] >> args[1]);
	case 15: //cout << args[0] << "+" << args[1] << " = " << (args[0] +  args[1]) << endl;
		return (args[0] +  args[1]);
	case 16: //cout << args[0] << "-" << args[1] << " = " << (args[0] -  args[1]) << endl;
		return (args[0] -  args[1]);
	case 17: //cout << args[0] << "*" << args[1] << " = " << (args[0] *  args[1]) << endl;
		return (args[0] *  args[1]);
	case 18: //cout << args[0] << "/" << args[1] << " = " << (args[0] /  args[1]) << endl;
		return (args[0] /  args[1]);
	case 19: //cout << args[0] << "%" << args[1] << " = " << (args[0] %  args[1]) << endl;
		return (args[0] %  args[1]);
	case 20: //cout << args[0] << "&&" << args[1] << " = " << (args[0] &&  args[1]) << endl;
		return (args[0] &&  args[1]);
	case 21: //cout << args[0] << "||" << args[1] << " = " << (args[0] ||  args[1]) << endl;
		return (args[0] ||  args[1]);
	default: return  args[0];
	}
}

void operation::propagate(state &result, const state &global, vector<value> &expressions, const vector<value> gexpressions, value v) const
{
	if (v.data == value::valid) {
		if (func >= 0 and func <= 2) {
			// ~a, a, -a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func == 3) {
			// !a
			operands[0].set(result, expressions, value(value::neutral));
		} else if (func >= 4 and func <= 20) {
			// a|b, a&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&&b
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
			operands[1].set(result, expressions, operands[1].get(global, gexpressions));
		} else if (func == 21) {
			// a||b
			value v0 = operands[0].get(global, gexpressions);
			if (v0.data >= value::valid) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, value(value::unknown));
			}
			value v1 = operands[1].get(global, expressions);
			if (v1.data >= value::valid) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, value(value::unknown));
			}
		} else {
			// a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		}
	} else if (v.data == value::neutral) {
		if (func >= 0 and func <= 2) {
			// ~a, a, -a
			operands[0].set(result, expressions, value(value::neutral));
		} else if (func == 3) {
			// !a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func >= 4 and func <= 20) {
			// a|b, a&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&&b
			value v0 = operands[0].get(global, gexpressions);
			if (v0.data == value::neutral) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, value(value::unknown));
			}
			value v1 = operands[1].get(global, expressions);
			if (v1.data == value::neutral) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, value(value::unknown));
			}
		} else if (func == 21) {
			// a||b
			operands[0].set(result, expressions, value(value::neutral));
			operands[1].set(result, expressions, value(value::neutral));
		} else {
			// a
			operands[0].set(result, expressions, value(value::neutral));
		}
	} else {
		if (func >= 0 and func <= 3) {
			// ~a, a, -a, !a
			operands[0].set(result, expressions, value(value::unknown));
		} else if (func >= 4 and func <= 21) {
			// a|b, a&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&&b, a||b
			operands[0].set(result, expressions, value(value::unknown));
			operands[1].set(result, expressions, value(value::unknown));
		} else {
			// a
			operands[0].set(result, expressions, value(value::unknown));
		}
	} 
}

expression::expression()
{
	set(operand(value(value::neutral)));
}

expression::expression(bool v)
{
	set(operand(value(v ? 0 : value::neutral)));
}

expression::expression(value v)
{
	set(operand(v));
}

expression::expression(operand arg0)
{
	set(arg0);
}

expression::expression(string func, operand arg0)
{
	set(func, arg0);
}

expression::expression(string func, expression arg0)
{
	set(func, arg0);
}

expression::expression(string func, operand arg0, operand arg1)
{
	set(func, arg0, arg1);
}

expression::expression(string func, expression arg0, expression arg1)
{
	set(func, arg0, arg1);
}

expression::expression(string func, expression arg0, operand arg1)
{
	set(func, arg0, arg1);
}

expression::expression(string func, operand arg0, expression arg1)
{
	set(func, arg0, arg1);
}

expression::expression(string func, vector<expression> args)
{
	set(func, args);
}

expression::~expression()
{

}

string expression::to_string(int index)
{
	if (index < 0) {
		index = (int)operations.size()-1;
	}

	stringstream os;
	os << "(";
	for (auto v = operations[index].operands.begin(); v != operations[index].operands.end(); v++) {
		if (v != operations[index].operands.begin()) {
			os << operations[index].get();
		}
		if (v->type != operand::expression) {
			os << *v;
		} else {
			os << to_string(v->index);
		}
	}
	os << ")";
	return os.str();
}

ostream &operator<<(ostream &os, expression e)
{
	os << e.to_string();
	return os;
}

void expression::set(operand arg0)
{
	operations.clear();
	operations.push_back(operation("", arg0));
}

void expression::set(string func, operand arg0)
{
	operations.clear();
	operations.push_back(operation(func, arg0));
}

void expression::set(string func, expression arg0)
{
	operations = arg0.operations;
	operations.push_back(operation(func, operand(operations.size()-1, operand::expression)));
}

void expression::set(string func, operand arg0, operand arg1)
{
	operations.clear();
	operations.push_back(operation(func, arg0, arg1));
}

void expression::set(string func, expression arg0, expression arg1)
{
	operations = arg0.operations;
	operand op0((int)operations.size()-1, operand::expression);

	int offset = operations.size();
	for (int j = 0; j < (int)arg1.operations.size(); j++)
	{
		operation tmp = arg1.operations[j];
		for (int k = 0; k < (int)tmp.operands.size(); k++)
			if (tmp.operands[k].type == operand::expression)
				tmp.operands[k].index += offset;

		operations.push_back(tmp);
	}

	operand op1((int)operations.size()-1, operand::expression);

	operations.push_back(operation(func, op0, op1));
}

void expression::set(string func, expression arg0, operand arg1)
{
	operations = arg0.operations;
	operand op0((int)operations.size()-1, operand::expression);


	operations.push_back(operation(func, op0, arg1));
}

void expression::set(string func, operand arg0, expression arg1)
{
	operations = arg1.operations;
	operand op1((int)operations.size()-1, operand::expression);

	operations.push_back(operation(func, arg0, op1));
}

void expression::set(string func, vector<expression> args)
{
	operations.clear();
	vector<operand> operands;

	for (int i = 0; i < (int)args.size(); i++)
	{
		int offset = operations.size();
		for (int j = 0; j < (int)args[i].operations.size(); j++)
		{
			operation tmp = args[i].operations[j];
			for (int k = 0; k < (int)tmp.operands.size(); k++)
				if (tmp.operands[k].type == operand::expression)
					tmp.operands[k].index += offset;

			operations.push_back(tmp);
		}
		operands.push_back(operand((int)operations.size()-1, false));
	}

	operations.push_back(operation(func, operands));
}

value expression::evaluate(state values) const
{
	vector<value> expressions;

	for (int i = 0; i < (int)operations.size(); i++)
		expressions.push_back(operations[i].evaluate(values, expressions));

	if (expressions.size() > 0)
		return expressions.back();
	else
		return value();
}

bool expression::is_constant() const
{
	for (int i = 0; i < (int)operations.size(); i++)
		for (int j = 0; j < (int)operations[i].operands.size(); j++)
			if (operations[i].operands[j].type != operand::constant && operations[i].operands[j].type != operand::expression && operations[i].operands[j].type != operand::neutral)
				return false;

	return true;
}

expression &expression::operator=(operand e)
{
	set("", e);
	return *this;
}

expression operator~(expression e)
{
	expression result;
	result.set("~", e);
	return result;
}

expression operator-(expression e)
{
	expression result;
	result.set("-", e);
	return result;
}

expression operator!(expression e)
{
	expression result;
	result.set("!", e);
	return result;
}

expression operator|(expression e0, expression e1)
{
	expression result;
	result.set("|", e0, e1);
	return result;
}

expression operator&(expression e0, expression e1)
{
	expression result;
	result.set("&", e0, e1);
	return result;
}

expression operator^(expression e0, expression e1)
{
	expression result;
	result.set("^", e0, e1);
	return result;
}

expression operator==(expression e0, expression e1)
{
	expression result;
	result.set("==", e0, e1);
	return result;
}

expression operator!=(expression e0, expression e1)
{
	expression result;
	result.set("~=", e0, e1);
	return result;
}

expression operator<(expression e0, expression e1)
{
	expression result;
	result.set("<", e0, e1);
	return result;
}

expression operator>(expression e0, expression e1)
{
	expression result;
	result.set(">", e0, e1);
	return result;
}

expression operator<=(expression e0, expression e1)
{
	expression result;
	result.set("<=", e0, e1);
	return result;
}

expression operator>=(expression e0, expression e1)
{
	expression result;
	result.set(">=", e0, e1);
	return result;
}

expression operator<<(expression e0, expression e1)
{
	expression result;
	result.set("<<", e0, e1);
	return result;
}

expression operator>>(expression e0, expression e1)
{
	expression result;
	result.set(">>", e0, e1);
	return result;
}

expression operator+(expression e0, expression e1)
{
	expression result;
	result.set("+", e0, e1);
	return result;
}

expression operator-(expression e0, expression e1)
{
	expression result;
	result.set("-", e0, e1);
	return result;
}

expression operator*(expression e0, expression e1)
{
	expression result;
	result.set("*", e0, e1);
	return result;
}

expression operator/(expression e0, expression e1)
{
	expression result;
	result.set("/", e0, e1);
	return result;
}

expression operator%(expression e0, expression e1)
{
	expression result;
	result.set("%", e0, e1);
	return result;
}

expression operator&&(expression e0, expression e1)
{
	expression result;
	result.set("&&", e0, e1);
	return result;
}

expression operator||(expression e0, expression e1)
{
	expression result;
	result.set("||", e0, e1);
	return result;
}

expression operator|(expression e0, operand e1)
{
	expression result;
	result.set("|", e0, e1);
	return result;
}

expression operator&(expression e0, operand e1)
{
	expression result;
	result.set("&", e0, e1);
	return result;
}

expression operator^(expression e0, operand e1)
{
	expression result;
	result.set("^", e0, e1);
	return result;
}

expression operator==(expression e0, operand e1)
{
	expression result;
	result.set("==", e0, e1);
	return result;
}

expression operator!=(expression e0, operand e1)
{
	expression result;
	result.set("~=", e0, e1);
	return result;
}

expression operator<(expression e0, operand e1)
{
	expression result;
	result.set("<", e0, e1);
	return result;
}

expression operator>(expression e0, operand e1)
{
	expression result;
	result.set(">", e0, e1);
	return result;
}

expression operator<=(expression e0, operand e1)
{
	expression result;
	result.set("<=", e0, e1);
	return result;
}

expression operator>=(expression e0, operand e1)
{
	expression result;
	result.set(">=", e0, e1);
	return result;
}

expression operator<<(expression e0, operand e1)
{
	expression result;
	result.set("<<", e0, e1);
	return result;
}

expression operator>>(expression e0, operand e1)
{
	expression result;
	result.set(">>", e0, e1);
	return result;
}

expression operator+(expression e0, operand e1)
{
	expression result;
	result.set("+", e0, e1);
	return result;
}

expression operator-(expression e0, operand e1)
{
	expression result;
	result.set("-", e0, e1);
	return result;
}

expression operator*(expression e0, operand e1)
{
	expression result;
	result.set("*", e0, e1);
	return result;
}

expression operator/(expression e0, operand e1)
{
	expression result;
	result.set("/", e0, e1);
	return result;
}

expression operator%(expression e0, operand e1)
{
	expression result;
	result.set("%", e0, e1);
	return result;
}

expression operator&&(expression e0, operand e1)
{
	expression result;
	result.set("&&", e0, e1);
	return result;
}

expression operator||(expression e0, operand e1)
{
	expression result;
	result.set("||", e0, e1);
	return result;
}

expression operator|(operand e0, expression e1)
{
	expression result;
	result.set("|", e0, e1);
	return result;
}

expression operator&(operand e0, expression e1)
{
	expression result;
	result.set("&", e0, e1);
	return result;
}

expression operator^(operand e0, expression e1)
{
	expression result;
	result.set("^", e0, e1);
	return result;
}

expression operator==(operand e0, expression e1)
{
	expression result;
	result.set("==", e0, e1);
	return result;
}

expression operator!=(operand e0, expression e1)
{
	expression result;
	result.set("~=", e0, e1);
	return result;
}

expression operator<(operand e0, expression e1)
{
	expression result;
	result.set("<", e0, e1);
	return result;
}

expression operator>(operand e0, expression e1)
{
	expression result;
	result.set(">", e0, e1);
	return result;
}

expression operator<=(operand e0, expression e1)
{
	expression result;
	result.set("<=", e0, e1);
	return result;
}

expression operator>=(operand e0, expression e1)
{
	expression result;
	result.set(">=", e0, e1);
	return result;
}

expression operator<<(operand e0, expression e1)
{
	expression result;
	result.set("<<", e0, e1);
	return result;
}

expression operator>>(operand e0, expression e1)
{
	expression result;
	result.set(">>", e0, e1);
	return result;
}

expression operator+(operand e0, expression e1)
{
	expression result;
	result.set("+", e0, e1);
	return result;
}

expression operator-(operand e0, expression e1)
{
	expression result;
	result.set("-", e0, e1);
	return result;
}

expression operator*(operand e0, expression e1)
{
	expression result;
	result.set("*", e0, e1);
	return result;
}

expression operator/(operand e0, expression e1)
{
	expression result;
	result.set("/", e0, e1);
	return result;
}

expression operator%(operand e0, expression e1)
{
	expression result;
	result.set("%", e0, e1);
	return result;
}

expression operator&&(operand e0, expression e1)
{
	expression result;
	result.set("&&", e0, e1);
	return result;
}

expression operator||(operand e0, expression e1)
{
	expression result;
	result.set("||", e0, e1);
	return result;
}

expression operator|(operand e0, operand e1)
{
	expression result;
	result.set("|", e0, e1);
	return result;
}

expression operator&(operand e0, operand e1)
{
	expression result;
	result.set("&", e0, e1);
	return result;
}

expression operator^(operand e0, operand e1)
{
	expression result;
	result.set("^", e0, e1);
	return result;
}

expression operator==(operand e0, operand e1)
{
	expression result;
	result.set("==", e0, e1);
	return result;
}

expression operator!=(operand e0, operand e1)
{
	expression result;
	result.set("~=", e0, e1);
	return result;
}

expression operator<(operand e0, operand e1)
{
	expression result;
	result.set("<", e0, e1);
	return result;
}

expression operator>(operand e0, operand e1)
{
	expression result;
	result.set(">", e0, e1);
	return result;
}

expression operator<=(operand e0, operand e1)
{
	expression result;
	result.set("<=", e0, e1);
	return result;
}

expression operator>=(operand e0, operand e1)
{
	expression result;
	result.set(">=", e0, e1);
	return result;
}

expression operator<<(operand e0, operand e1)
{
	expression result;
	result.set("<<", e0, e1);
	return result;
}

expression operator>>(operand e0, operand e1)
{
	expression result;
	result.set(">>", e0, e1);
	return result;
}

expression operator+(operand e0, operand e1)
{
	expression result;
	result.set("+", e0, e1);
	return result;
}

expression operator-(operand e0, operand e1)
{
	expression result;
	result.set("-", e0, e1);
	return result;
}

expression operator*(operand e0, operand e1)
{
	expression result;
	result.set("*", e0, e1);
	return result;
}

expression operator/(operand e0, operand e1)
{
	expression result;
	result.set("/", e0, e1);
	return result;
}

expression operator%(operand e0, operand e1)
{
	expression result;
	result.set("%", e0, e1);
	return result;
}

expression operator&&(operand e0, operand e1)
{
	expression result;
	result.set("&&", e0, e1);
	return result;
}

expression operator||(operand e0, operand e1)
{
	expression result;
	result.set("||", e0, e1);
	return result;
}

int passes_guard(const state &encoding, const state &global, const expression &guard, state *total) {
	vector<value> expressions;
	vector<value> gexpressions;

	for (int i = 0; i < (int)guard.operations.size(); i++) {
		value g = guard.operations[i].evaluate(global, expressions);
		value l = guard.operations[i].evaluate(encoding, expressions);

		if (l.data == value::unstable or g.data == value::unstable
			or (g.data == value::neutral and l.data >= value::valid)
			or (g.data >= value::valid and l.data == value::neutral)
			or (g.data >= value::valid and l.data == value::valid and g.data != l.data)) {
			l.data = value::unstable;
		}

		expressions.push_back(l);
		gexpressions.push_back(g);
	}

	// If the final value in the expression stack is valid, then we've passed the
	// guard, and we can back propagate information back to individual variables.
	// If it is neutral, then we have not yet passed the guard and therefore
	// cannot propagate information back.

	// If the guard passes, then we have waited for every transition on the
	// variable. Therefore, we know not only that the input signals are valid or
	// neutral, but we also know their value. This valid is copied over from the
	// global state.

	// This validity/neutrality information propagates differently through
	// different operations.
	if (total != nullptr and expressions.back().data >= value::valid) {
		for (int i = (int)guard.operations.size()-1; i >= 0; i--) {
			guard.operations[i].propagate(*total, global, expressions, gexpressions, expressions[i]);
		}
	}

	if (expressions.empty()) {
		return 1;
	} else if (expressions.back().data == value::neutral) {
		return -1;
	} else if (expressions.back().data == value::unstable) {
		return 0;
	}
	
	return 1;
}

expression weakest_guard(const expression &guard, const expression &exclude) {
	// TODO(edward.bingham) implement
	return guard;
}

}
