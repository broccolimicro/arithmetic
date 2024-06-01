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
	type = invalid;
}

operand::operand(int index, int type)
{
	this->index = index;
	this->type = type;
}

operand::operand(value v)
{
	if (v.data == value::invalid)
		type = invalid;
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
	if (o.type == operand::invalid) {
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

value operand::get(vector<value> values, vector<value> expressions) const
{
	switch (type)
	{
	case invalid:		return value(value::invalid);
	case unstable:		return value(value::unstable);
	case unknown:		return value(value::unknown);
	case constant:		return value(index);
	case expression:
		if (index >= 0 && index < (int)expressions.size())
			return expressions[index];
		else
			return value(value::invalid);
	case variable:
		if (index >= 0 && index < (int)values.size())
			return values[index];
		else
			return value(value::invalid);
	default:			return value(value::unstable);
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

value operation::evaluate(vector<value> values, vector<value> expressions) const
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

expression::expression()
{

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

value expression::evaluate(vector<value> values) const
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
			if (operations[i].operands[j].type != operand::constant && operations[i].operands[j].type != operand::expression && operations[i].operands[j].type != operand::invalid)
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

}
