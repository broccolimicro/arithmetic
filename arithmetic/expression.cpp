/*
 * expression.cpp
 *
 *  Created on: Jul 1, 2015
 *      Author: nbingham
 */

#include "expression.h"

#include <sstream>
#include <common/standard.h>
#include <common/text.h>

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
	else if (v.data == value::valid)
		type = valid;
	else if (v.data > value::valid) {
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
		os << "gnd";
	} else if (o.type == operand::unstable) {
		os << "X";
	} else if (o.type == operand::unknown) {
		os << "U";
	} else if (o.type == operand::valid) {
		os << "vdd";
	} else if (o.type == operand::constant) {
		os << o.index;
	} else if (o.type == operand::expression) {
		os << "e" << o.index;
	} else if (o.type == operand::variable) {
		os << "v" << o.index;
	}
	return os;
}

bool are_same(operand o0, operand o1) {
	return o0.type == o1.type and o0.index == o1.index;
}

value operand::get(state values, vector<value> expressions) const
{
	switch (type)
	{
	case neutral:  return value(value::neutral);
	case unstable: return value(value::unstable);
	case unknown:  return value(value::unknown);
	case valid:    return value(value::valid);
	case constant: return value(index);
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
	default:       return value(value::unstable);
	}
}

void operand::set(state &values, vector<value> &expressions, value v) const {
	if (type == expression and index >= 0 and index < (int)expressions.size()) {
		expressions[index] = v;
	} else if (type == variable and index >= 0) {
		values.set(index, v);
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

	if (func == "!")
		this->func = 0;
	else if (func == "+")
		this->func = 1;
	else if (func == "-")
		this->func = 2;
	else if (func == "(bool)")
		this->func = 3;
	else if (func == "~")
		this->func = 4;
}

void operation::set(string func, operand arg0, operand arg1)
{
	operands.push_back(arg0);
	operands.push_back(arg1);
	this->func = -1;

	if (func == "||")
		this->func = 5;
	else if (func == "&&")
		this->func = 6;
	else if (func == "^")
		this->func = 7;
	else if (func == "==")
		this->func = 8;
	else if (func == "~=")
		this->func = 9;
	else if (func == "<")
		this->func = 10;
	else if (func == ">")
		this->func = 11;
	else if (func == "<=")
		this->func = 12;
	else if (func == ">=")
		this->func = 13;
	else if (func == "<<")
		this->func = 14;
	else if (func == ">>")
		this->func = 15;
	else if (func == "+")
		this->func = 16;
	else if (func == "-")
		this->func = 17;
	else if (func == "*")
		this->func = 18;
	else if (func == "/")
		this->func = 19;
	else if (func == "%")
		this->func = 20;
	else if (func == "?")
		this->func = 21;
	else if (func == "&")
		this->func = 22;
	else if (func == "|")
		this->func = 23;
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

string operation::get() const {
	// DESIGN(edward.bingham) bitwise and boolean operations have been switched
	// to be consistent with HSE and boolean logic expressions

	switch (func)
	{
	case 0: return "!";  // bitwise not
	case 1: return "+";
	case 2: return "-";
	case 3: return "(bool)"; // boolean check
	case 4: return "~";  // boolean not
	case 5: return "||"; // bitwise or
	case 6: return "&&"; // bitwise and
	case 7: return "^";  // bitwise xor
	case 8: return "==";
	case 9: return "!=";
	case 10: return "<";
	case 11: return ">";
	case 12: return "<=";
	case 13: return ">=";
	case 14: return "<<";
	case 15: return ">>";
	case 16: return "+";
	case 17: return "-";
	case 18: return "*";
	case 19: return "/";
	case 20: return "%";
	case 21: return "?";
	case 22: return "&"; // boolean or
	case 23: return "|"; // boolean and
	default: return "";
	}
}

value operation::evaluate(state values, vector<value> expressions) const
{
	vector<value> args;
	args.reserve(operands.size());
	for (int i = 0; i < (int)operands.size(); i++) {
		args.push_back(operands[i].get(values, expressions));
	}

	// TODO(edward.bingham) create a single evaluation function for values that
	// is then called to implement the operators defined in the source.

	int i = 0;
	switch (func)
	{
	case 0: //cout << "~" << args[0] << " = " << ~args[0] << endl;
		return !args[0];
	case 1: //cout << "+" << args[0] << " = " << +args[0] << endl;
		return args[0];
	case 2: //cout << "-" << args[0] << " = " << -args[0] << endl;
		return -args[0];
	case 3:
		return is_valid(args[0]);
	case 4: //cout << "!" << args[0] << " = " << !args[0] << endl;
		return ~args[0];
	case 5: //cout << args[0] << "|" << args[1] << " = " << (args[0] |  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] || args[i];
		}
		return args[0];
	case 6: //cout << args[0] << "&" << args[1] << " = " << (args[0] &  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] && args[i];
		}
		return args[0];
	case 7: //cout << args[0] << "^" << args[1] << " = " << (args[0] ^  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] ^ args[i];
		}
		return args[0];
	case 8: //cout << args[0] << "==" << args[1] << " = " << (args[0] ==  args[1]) << endl;
		return (args[0] == args[1]);
	case 9: //cout << args[0] << "!=" << args[1] << " = " << (args[0] !=  args[1]) << endl;
		return (args[0] != args[1]);
	case 10: //cout << args[0] << "<" << args[1] << " = " << (args[0] <  args[1]) << endl;
		return (args[0] <  args[1]);
	case 11: //cout << args[0] << ">" << args[1] << " = " << (args[0] >  args[1]) << endl;
		return (args[0] >  args[1]);
	case 12: //cout << args[0] << "<=" << args[1] << " = " << (args[0] <=  args[1]) << endl;
		return (args[0] <= args[1]);
	case 13: //cout << args[0] << ">=" << args[1] << " = " << (args[0] >=  args[1]) << endl;
		return (args[0] >= args[1]);
	case 14: //cout << args[0] << "<<" << args[1] << " = " << (args[0] <<  args[1]) << endl;
		return (args[0] << args[1]);
	case 15: //cout << args[0] << ">>" << args[1] << " = " << (args[0] >>  args[1]) << endl;
		return (args[0] >> args[1]);
	case 16: //cout << args[0] << "+" << args[1] << " = " << (args[0] +  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] + args[i];
		}
		return args[0];
	case 17: //cout << args[0] << "-" << args[1] << " = " << (args[0] -  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] - args[i];
		}
		return args[0];
	case 18: //cout << args[0] << "*" << args[1] << " = " << (args[0] *  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] * args[i];
		}
		return args[0];
	case 19: //cout << args[0] << "/" << args[1] << " = " << (args[0] /  args[1]) << endl;
		return (args[0] /  args[1]);
	case 20: //cout << args[0] << "%" << args[1] << " = " << (args[0] %  args[1]) << endl;
		return (args[0] %  args[1]);
	case 21: // TODO(edward.bingham) operator?
		return args[0];
	case 22: //cout << args[0] << "&&" << args[1] << " = " << (args[0] &&  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] & args[i];
		}
		return args[0];
	case 23: //cout << args[0] << "||" << args[1] << " = " << (args[0] ||  args[1]) << endl;
		for (i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] | args[i];
		}
		return args[0];
	default: return  args[0];
	}
}

void operation::propagate(state &result, const state &global, vector<value> &expressions, const vector<value> gexpressions, value v) const
{
	if (v.data >= value::valid or v.data == value::unknown) {
		if (func >= 0 and func <= 2) {
			// !a, a, -a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func == 3 or func < 0) {
			// (bool)
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func == 4) {
			// ~a
			operands[0].set(result, expressions, value(value::neutral));
		} else if (func >= 5 and func <= 22) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
			operands[1].set(result, expressions, operands[1].get(global, gexpressions));
		} else if (func == 23) {
			// a|b
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
			// !a, a, -a
			operands[0].set(result, expressions, value(value::neutral));
		} else if (func == 3 or func < 0) {
			operands[0].set(result, expressions, value(value::neutral));
		} else if (func == 4) {
			// ~a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func >= 5 and func <= 22) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b
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
		} else if (func == 23) {
			// a|b
			operands[0].set(result, expressions, value(value::neutral));
			operands[1].set(result, expressions, value(value::neutral));
		} else {
			// a
			operands[0].set(result, expressions, value(value::neutral));
		}
	} else {
		if ((func >= 0 and func <= 4) or func < 0) {
			// !a, a, -a, ~a
			operands[0].set(result, expressions, value(value::unknown));
		} else if (func >= 5 and func <= 23) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b, a|b
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
}

expression::expression(bool v)
{
	set(operand(value(v ? value::valid : value::neutral)));
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

void minimize(expression &F) {
	// TODO(edward.bingham) implement expression simplification using term replacement
	// 1. find a way to represent replacement rules
	//   a. regular expressions-like thing from the root of an expression or from
	//   the leaves? What are L-Systems? What about just expression->expression?
	//   Rete's algorithm?
	// 2. create a library of replacement rules
	//   a. A replacement rule could be unidirectional or bidirectional.
	// 3. follow all unidirectional replacement rules that match
	// 4. search all bidirectional replacement rules that match
	// 5. repeat 3 and 4
	// 6. ensure the expression is left in a normal form
	//   a. push all constants to the right
	//   b. prefer addition of negative over subtraction
	//   c. prefer multiplication of inverse over division
	//   d. merge all constants and all same-term literals

	// Rewrite rules
	// a & a = (bool)a
	// a | a = (bool)a
	// a+a = 2a
	// ~~a = (bool)a
	// --a = a
	// a - b = a + (-b)
	// a / b = a * (1/b)
	// (bool)(a + b) = (bool)a & (bool)b
	// (bool)(bool)a = (bool)a
	// (bool)~a = ~a
	// ~(bool)a = ~a
	// (bool)a & b = a & b
	// a & (b & c) = a & b & c
	// a & (b | c) = a & b | a & c
	// a * (b + c) = a * b + a * c
	// a < b = a - b < 0

	// How do I represent a rewrite rule?
	// pair of expressions
	// How do I match?
	// A literal in the key is a literal, constant, or sub expression in the
	// matched expression. Token propagation of matched subexpressions.
	// How do I manage the list of rewrite rules?
	// Load them into a static variable on program initialization.
	// Is this an efficient representation of the rewrite rules?
	// Rete's algorithm doesn't handle search, it's continuous evaluation.
	// Regular expressions? All of the rewrite rules can be represented in a
	// single expression structure where the rewrite operator can be represented
	// by the operation("->", key, rewrite).
}

void expression::init_rewrite() {
	/*
	push("->", push("&", -1, -1), push("(bool)", -1))
	push("->", push("|", -1, -1), push("(bool)", -1))
	push("->", push("+", -1, -1), push("*", 2, -1))
	push("->", push("+", -1, push("*", -2, -1)), push("*", push("+", 1, -2), -1))
	*/
}

int expression::find(operation arg) {
	for (int i = 0; i < (int)operations.size(); i++) {
		if (operations[i].func == arg.func
			and operations[i].operands.size() == arg.operands.size()) {
			bool found = true;
			for (int j = 0; j < (int)arg.operands.size(); j++) {
				if (operations[i].operands[j].type != arg.operands[j].type or operations[i].operands[j].index != arg.operands[j].index) {
					found = false;
					break;
				}
			}
			if (found) {
				return i;
			}
		}
	}
	return -1;
}

int expression::push(operation arg) {
	if (arg.func == 5 or arg.func == 6 or
			arg.func == 22 or arg.func == 23) {
		// & | && ||
		for (int i = (int)arg.operands.size()-1; i >= 1; i--) {
			bool found = false;
			for (int j = 0; j < i; j++) {
				if (are_same(arg.operands[i], arg.operands[j])) {
					found = true;
					break;
				}
			}

			if (found) {
				arg.operands.erase(arg.operands.begin()+i);
			}
		}
		if (arg.operands.size() == 1) {
			arg.func = -1;
		}
	}

	int pos = find(arg);
	if (pos < 0) {
		pos = operations.size();
		operations.push_back(arg);
	}
	return pos;
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
	push(operation(func, operand(operations.size()-1, operand::expression)));
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

	vector<int> offset;
	for (auto j = arg1.operations.begin(); j != arg1.operations.end(); j++) {
		for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
			if (k->type == operand::expression) {
				k->index = offset[k->index];
			}
		}

		offset.push_back(push(*j));
	}

	operand op1(offset.back(), operand::expression);

	push(operation(func, op0, op1));
}

void expression::set(string func, expression arg0, operand arg1)
{
	operations = arg0.operations;
	operand op0((int)operations.size()-1, operand::expression);


	push(operation(func, op0, arg1));
}

void expression::set(string func, operand arg0, expression arg1)
{
	operations = arg1.operations;
	operand op1((int)operations.size()-1, operand::expression);

	push(operation(func, arg0, op1));
}

void expression::set(string func, vector<expression> args)
{
	operations.clear();
	vector<operand> operands;

	for (int i = 0; i < (int)args.size(); i++) {
		vector<int> offset;
		for (auto j = args[i].operations.begin(); j != args[i].operations.end(); j++) {
			for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
				if (k->type == operand::expression) {
					k->index = offset[k->index];
				}
			}

			offset.push_back(push(*j));
		}
		operands.push_back(operand(offset.back(), false));
	}

	push(operation(func, operands));
}

void expression::push(string func) {
	operand op0((int)operations.size()-1, operand::expression);
	push(operation(func, op0));
}

void expression::push(string func, operand arg0) {
	operand op0((int)operations.size()-1, operand::expression);
	push(operation(func, op0, arg0));
}

void expression::push(string func, expression arg0) {
	operand op0((int)operations.size()-1, operand::expression);

	vector<int> offset;
	for (auto j = arg0.operations.begin(); j != arg0.operations.end(); j++) {
		for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
			if (k->type == operand::expression) {
				k->index = offset[k->index];
			}
		}

		offset.push_back(push(*j));
	}

	operand op1(offset.back(), operand::expression);
	push(operation(func, op0, op1));
}

value expression::evaluate(state values) const
{
	vector<value> expressions;

	for (int i = 0; i < (int)operations.size(); i++) {
		expressions.push_back(operations[i].evaluate(values, expressions));
	}

	if (expressions.size() > 0)
		return expressions.back();
	else
		return value();
}

bool expression::is_null() const
{
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->type == operand::variable
				or j->type == operand::valid
				or j->type == operand::constant
				or j->type == operand::unknown
				or j->type == operand::neutral) {
				return false;
			}
		}
	}
	return true;
}

bool expression::is_constant() const
{
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->type == operand::variable
				or j->type == operand::unstable) {
				return false;
			}
		}
	}
	return true;
}

bool expression::is_valid() const
{
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->type == operand::variable
				or j->type == operand::unstable
				or j->type == operand::neutral) {
				return false;
			}
		}
	}
	return true;
}

bool expression::is_neutral() const
{
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->type == operand::variable
				or j->type == operand::unstable
				or j->type == operand::valid
				or j->type == operand::constant) {
				return false;
			}
		}
	}
	return true;
}

bool expression::is_wire() const
{
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (i->func == 3 or i->func == 4 or i->func == 22 or i->func == 23) {
			return true;
		}
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->type == operand::neutral
				or j->type == operand::valid) {
				return true;
			}
		}
	}
	return false;
}

expression &expression::operator=(operand e)
{
	set("", e);
	return *this;
}

expression operator!(expression e)
{
	expression result;
	result.set("!", e);
	return result;
}

expression operator-(expression e)
{
	expression result;
	result.set("-", e);
	return result;
}

expression is_valid(expression e)
{
	if (e.is_valid()) {
		return true;
	} else if (e.is_neutral()) {
		return false;
	} else if (e.is_wire()) {
		return e;
	}
	return expression("(bool)", e);
}

expression operator~(expression e)
{
	if (e.is_valid()) {
		return false;
	} else if (e.is_neutral()) {
		return true;
	}
	return expression("~", e);
}

expression operator||(expression e0, expression e1)
{
	expression result;
	result.set("||", e0, e1);
	return result;
}

expression operator&&(expression e0, expression e1)
{
	expression result;
	result.set("&&", e0, e1);
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

expression operator&(expression e0, expression e1)
{
	if (e0.is_null() or e1.is_null()) {
		return expression(value(value::unstable));
	} else if (e0.is_neutral() or e1.is_neutral()) {
		return false;
	} else if (e0.is_valid()) {
		return is_valid(e1);
	} else if (e1.is_valid()) {
		return is_valid(e0);
	}
	return expression("&", e0, e1);
}

expression operator|(expression e0, expression e1)
{
	if (e0.is_null() or e1.is_null()) {
		return expression(value(value::unstable));
	} if (e0.is_valid() or e1.is_valid()) {
		return true;
	} else if (e0.is_neutral()) {
		return is_valid(e1);
	} else if (e1.is_neutral()) {
		return is_valid(e0);
	}
	return expression("|", e0, e1);
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
		value g = guard.operations[i].evaluate(global, gexpressions);
		value l = guard.operations[i].evaluate(encoding, expressions);

		if (l.data == value::unstable or g.data == value::unstable
			or (g.data == value::neutral and l.data >= value::valid)
			or (g.data >= value::valid and l.data == value::neutral)
			or (g.data >= value::valid and l.data >= value::valid and g.data != l.data)) {
			l.data = value::unstable;
		}

		expressions.push_back(l);
		gexpressions.push_back(g);
	}

	if (expressions.back().data == value::unknown or expressions.back().data >= value::valid) {
		if (gexpressions.back().data == value::neutral or gexpressions.back().data == value::unknown) {
			expressions.back().data = value::unstable;
		} else if (gexpressions.back().data >= value::valid) {
			expressions.back().data = gexpressions.back().data;
		}
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

	if (expressions.empty() or expressions.back().data == value::neutral) {
		return -1;
	} else if (expressions.back().data == value::unstable) {
		return 0;
	}
	
	return 1;
}

expression weakest_guard(const expression &guard, const expression &exclude) {
	// TODO(edward.bingham) Remove terms from the guard until guard overlaps exclude (using cylidrical algebraic decomposition)
	// 1. put the guard in conjunctive normal form using the boolean operations & | ~
	// 2. for each term in the conjunctive normal form, pick a comparison and eliminate it, then check overlap. 
	return guard;
}

}
