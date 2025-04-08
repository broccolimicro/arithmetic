#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <common/standard.h>
#include <common/text.h>

namespace arithmetic
{

Operand::Operand(value v) {
	type = CONST;
	cnst = v;
}

Operand::Operand(bool bval) {
	type = CONST;
	cnst = value::boolOf(bval);
}

Operand::Operand(int64_t ival) {
	type = CONST;
	cnst = value::intOf(ival);
}

Operand::Operand(int ival) {
	type = CONST;
	cnst = value::intOf(ival);
}

Operand::Operand(double rval) {
	type = CONST;
	cnst = value::realOf(rval);
}

Operand::~Operand() {
}

bool Operand::isConst() const {
	return type == CONST;
}

bool Operand::isExpr() const {
	return type == EXPR;
}

bool Operand::isVar() const {
	return type == VAR;
}

value Operand::get(state values, vector<value> expressions) const {
	switch (type)
	{
	case CONST:
		return cnst;
	case VAR:
		if (index < values.size()) {
			return values[index];
		}
	case EXPR:
		if (index < expressions.size()) {
			return expressions[index];
		}
	}
	return value::X();
}

void Operand::set(state &values, vector<value> &expressions, value v) const {
	if (isExpr()) {
		if (index < expressions.size()) {
			expressions[index] = v;
		} else {
			printf("error: Expression index '%lu' out of bounds '%lu'\n", index, expressions.size());
		}
	} else if (isVar()) {
		values.set(index, v);
	}
}

Operand Operand::X() {
	return Operand(value::X());
}

Operand Operand::U() {
	return Operand(value::U());
}

Operand Operand::boolOf(bool bval) {
	return Operand(value::boolOf(bval));
}

Operand Operand::intOf(int64_t ival) {
	return Operand(value::intOf(ival));
}

Operand Operand::realOf(double rval) {
	return Operand(value::realOf(rval));
}

Operand Operand::arrOf(vector<value> arr) {
	return Operand(value::arrOf(arr));
}

Operand Operand::structOf(vector<value> arr) {
	return Operand(value::structOf(arr));
}

Operand Operand::exprOf(size_t index) {
	Operand result;
	result.type = EXPR;
	result.index = index;
	return result;
}

void Operand::offsetExpr(int off) {
	if (isExpr()) {
		index += off;
	}
}

Operand Operand::varOf(size_t index) {
	Operand result;
	result.type = VAR;
	result.index = index;
	return result;
}

void Operand::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	if (isVar()) {
		if (index < uid_map.size()) {
			index = uid_map[index];
		} else {
			type = CONST;
			cnst = value::X();
		}
	}
}

void Operand::apply(vector<Operand> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	if (isVar()) {
		if (index < uid_map.size()) {
			type = uid_map[index].type;
			index = uid_map[index].index;
		} else {
			type = CONST;
			cnst = value::X();
		}
	}
}

ostream &operator<<(ostream &os, Operand o) {
	if (o.isConst()) {
		os << o.cnst;
	} else if (o.isVar()) {
		os << "v" << o.index;
	} else if (o.isExpr()) {
		os << "e" << o.index;
	}
	return os;
}

bool areSame(Operand o0, Operand o1) {
	return ((o0.isConst() and o1.isConst() and areSame(o0.cnst, o1.cnst))
		or (o0.isVar() and o1.isVar() and o0.index == o1.index)
		or (o0.isExpr() and o1.isExpr() and o0.index == o1.index));
}

Operation::Operation() {
	func = -1;
}

Operation::Operation(string func, vector<Operand> args) {
	set(func, args);
}

Operation::~Operation() {
}

string Operation::funcString(int func) {
	// DESIGN(edward.bingham) bitwise and boolean operations have been switched
	// to be consistent with HSE and boolean logic expressions

	// DESIGN(edward.bingham) order of these operations matters for the propagate function!

	switch (func)
	{
	case 0: return "!";  // bitwise not
	case 1: return "+";  // identity
	case 2: return "-";  // negation
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
	case 14: return "<<"; // shift left  (arithmetic/logical based on type?)
	case 15: return ">>"; // shift right (arithmetic/logical based on type?)
	case 16: return "+";
	case 17: return "-";
	case 18: return "*";
	case 19: return "/";
	case 20: return "%";
	case 21: return "?"; // channel receive? or should we make this the ternary operator (?:)?
	case 22: return "&"; // boolean or
	case 23: return "|"; // boolean and
	case 24: return "[,]"; // array
	case 25: return "[]"; // index
	default: return "";
	}
}

int Operation::funcIndex(string func, int args) {
	// unary operators
	if (func == "!" and args == 1)
		return 0;
	else if (func == "+" and args == 1) // identity
		return 1;
	else if (func == "-" and args == 1)
		return 2;
	else if (func == "(bool)" and args == 1)
		return 3;
	else if (func == "~" and args == 1)
		return 4;

	// binary operators
	else if (func == "||")
		return 5;
	else if (func == "&&")
		return 6;
	else if (func == "^")
		return 7;
	else if (func == "==")
		return 8;
	else if (func == "~=")
		return 9;
	else if (func == "<")
		return 10;
	else if (func == ">")
		return 11;
	else if (func == "<=")
		return 12;
	else if (func == ">=")
		return 13;
	else if (func == "<<")
		return 14;
	else if (func == ">>")
		return 15;
	else if (func == "+")
		return 16;
	else if (func == "-")
		return 17;
	else if (func == "*")
		return 18;
	else if (func == "/")
		return 19;
	else if (func == "%")
		return 20;
	else if (func == "?")
		return 21;
	else if (func == "&")
		return 22;
	else if (func == "|")
		return 23;

	// concat arrays
	else if (func == "[,]")
		return 24;
	// index
	else if (func == "[]")
		return 25;

	return -1;
}

void Operation::set(string func, vector<Operand> args) {
	this->func = Operation::funcIndex(func, (int)args.size());
	this->operands = args;
}

string Operation::get() const {
	return Operation::funcString(func);
}

bool Operation::is_commutative() const {
	return (func >= 5 and func <= 9)
		or func == 16
		or func == 18
		or func == 22
		or func == 23
		or func == 25;
}

value Operation::evaluate(int func, vector<value> args) {
	int i = 0;
	switch (func) {
	case 0: //cout << "~" << args[0] << " = " << ~args[0] << endl;
		return !args[0];
	case 1: //cout << "+" << args[0] << " = " << +args[0] << endl;
		return args[0];
	case 2: //cout << "-" << args[0] << " = " << -args[0] << endl;
		return -args[0];
	case 3:
		return valid(args[0]);
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
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] == args[1]);
	case 9: //cout << args[0] << "!=" << args[1] << " = " << (args[0] !=  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] != args[1]);
	case 10: //cout << args[0] << "<" << args[1] << " = " << (args[0] <  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] <  args[1]);
	case 11: //cout << args[0] << ">" << args[1] << " = " << (args[0] >  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] >  args[1]);
	case 12: //cout << args[0] << "<=" << args[1] << " = " << (args[0] <=  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] <= args[1]);
	case 13: //cout << args[0] << ">=" << args[1] << " = " << (args[0] >=  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] >= args[1]);
	case 14: //cout << args[0] << "<<" << args[1] << " = " << (args[0] <<  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] << args[1]);
	case 15: //cout << args[0] << ">>" << args[1] << " = " << (args[0] >>  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
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
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] /  args[1]);
	case 20: //cout << args[0] << "%" << args[1] << " = " << (args[0] %  args[1]) << endl;
		if (args.size() == 1u) {
			return args[0];
		}
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
	case 24: // concat arrays
		return value::arrOf(args);
	case 25: // index
		return index(args[0], args[1]);
	default:
		return args[0];
	}
}

value Operation::evaluate(state values, vector<value> expressions) const {
	vector<value> args;
	args.reserve(operands.size());
	for (int i = 0; i < (int)operands.size(); i++) {
		args.push_back(operands[i].get(values, expressions));
	}

	return Operation::evaluate(func, args);
}

void Operation::propagate(state &result, const state &global, vector<value> &expressions, const vector<value> gexpressions, value v) const
{
	if (v.isValid() or v.isUnknown()) {
		if (func >= 0 and func <= 2) {
			// !a, a, -a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func == 3 or func < 0) {
			// (bool)
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func == 4) {
			// ~a
			operands[0].set(result, expressions, false);
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
			if (v0.isValid()) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, value::U());
			}
			value v1 = operands[1].get(global, expressions);
			if (v1.isValid()) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, value::U());
			}
		} else {
			// a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		}
	} else if (v.isNeutral()) {
		if (func >= 0 and func <= 2) {
			// !a, a, -a
			operands[0].set(result, expressions, false);
		} else if (func == 3 or func < 0) {
			operands[0].set(result, expressions, false);
		} else if (func == 4) {
			// ~a
			operands[0].set(result, expressions, operands[0].get(global, gexpressions));
		} else if (func >= 5 and func <= 22) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b
			value v0 = operands[0].get(global, gexpressions);
			if (v0.isNeutral()) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, value::U());
			}
			value v1 = operands[1].get(global, expressions);
			if (v1.isNeutral()) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, value::U());
			}
		} else if (func == 23) {
			// a|b
			operands[0].set(result, expressions, false);
			operands[1].set(result, expressions, false);
		} else {
			// a
			operands[0].set(result, expressions, false);
		}
	} else {
		if ((func >= 0 and func <= 4) or func < 0) {
			// !a, a, -a, ~a
			operands[0].set(result, expressions, value::U());
		} else if (func >= 5 and func <= 23) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b, a|b
			operands[0].set(result, expressions, value::U());
			operands[1].set(result, expressions, value::U());
		} else {
			// a
			operands[0].set(result, expressions, value::U());
		}
	} 
}

void Operation::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].apply(uid_map);
	}
}

void Operation::apply(vector<Operand> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].apply(uid_map);
	}
}

void Operation::offsetExpr(int off) {
	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].offsetExpr(off);
	}
}

bool areSame(Operation o0, Operation o1) {
	if (o0.func != o1.func or 
		o0.operands.size() != o1.operands.size()) {
		return false;
	}

	for (int j = 0; j < (int)o0.operands.size(); j++) {
		if (not areSame(o0.operands[j], o1.operands[j])) {
			return false;
		}
	}
	return true;
}

ostream &operator<<(ostream &os, Operation o) {
	for (auto v = o.operands.begin(); v != o.operands.end(); v++) {
		if (v != o.operands.begin()) {
			os << o.get();
		}
		os << *v;
	}
	return os;
}

Expression::Expression() {
}

Expression::Expression(Operand arg0) {
	set(arg0);
}

Expression::Expression(string func, Operand arg0) {
	set(func, arg0);
}

Expression::Expression(string func, Expression arg0) {
	set(func, arg0);
}

Expression::Expression(string func, Operand arg0, Operand arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(string func, Expression arg0, Operand arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(string func, Operand arg0, Expression arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(string func, Expression arg0, Expression arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(string func, vector<Expression> args) {
	set(func, args);
}

Expression::~Expression() {
}

int Expression::find(Operation arg) {
	// TODO(edward.bingham) Maybe want to look for one Operation contains another
	// as well rather than just strict matching if we're going to support more
	// than two operands for commutative operators
	for (int i = 0; i < (int)operations.size(); i++) {
		if (areSame(operations[i], arg)) {
			return i;
		}
	}
	return -1;
}

int Expression::push(Operation arg) {
	// simplify the Expression using a simple simplification rule
	// TODO(edward.bingham) move this into the minimization function and just call that on arg
	if (arg.func == 5 or arg.func == 6 or
			arg.func == 22 or arg.func == 23) {
		// & | && ||
		for (int i = (int)arg.operands.size()-1; i >= 1; i--) {
			bool found = false;
			for (int j = 0; j < i; j++) {
				if (areSame(arg.operands[i], arg.operands[j])) {
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

	// add to operations list if doesn't exist
	int pos = find(arg);
	if (pos < 0) {
		pos = operations.size();
		operations.push_back(arg);
	}
	return pos;
}

void Expression::set(Operand arg0) {
	operations.clear();
	operations.push_back(Operation("", {arg0}));
}

void Expression::set(string func, Operand arg0) {
	operations.clear();
	operations.push_back(Operation(func, {arg0}));
}

void Expression::set(string func, Expression arg0) {
	operations = arg0.operations;
	push(Operation(func, {Operand::exprOf(operations.size()-1)}));
}

void Expression::set(string func, Operand arg0, Operand arg1) {
	operations.clear();
	operations.push_back(Operation(func, {arg0, arg1}));
}

void Expression::set(string func, Expression arg0, Expression arg1) {
	operations = arg0.operations;
	Operand op0 = Operand::exprOf(operations.size()-1);

	vector<int> offset;
	for (auto j = arg1.operations.begin(); j != arg1.operations.end(); j++) {
		for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
			if (k->isExpr()) {
				k->index = offset[k->index];
			}
		}

		offset.push_back(push(*j));
	}

	Operand op1 = Operand::exprOf(offset.back());

	push(Operation(func, {op0, op1}));
}

void Expression::set(string func, Expression arg0, Operand arg1)
{
	operations = arg0.operations;
	Operand op0 = Operand::exprOf(operations.size()-1);


	push(Operation(func, {op0, arg1}));
}

void Expression::set(string func, Operand arg0, Expression arg1)
{
	operations = arg1.operations;
	Operand op1 = Operand::exprOf(operations.size()-1);

	push(Operation(func, {arg0, op1}));
}

void Expression::set(string func, vector<Expression> args)
{
	operations.clear();
	vector<Operand> operands;

	for (int i = 0; i < (int)args.size(); i++) {
		vector<int> offset;
		for (auto j = args[i].operations.begin(); j != args[i].operations.end(); j++) {
			for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
				if (k->isExpr()) {
					k->index = offset[k->index];
				}
			}

			offset.push_back(push(*j));
		}
		operands.push_back(Operand::exprOf(offset.back()));
	}

	push(Operation(func, operands));
}

void Expression::push(string func) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	push(Operation(func, {op0}));
}

void Expression::push(string func, Operand arg0) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	push(Operation(func, {op0, arg0}));
}

void Expression::push(string func, Expression arg0) {
	Operand op0 = Operand::exprOf(operations.size()-1);

	vector<int> offset;
	for (auto j = arg0.operations.begin(); j != arg0.operations.end(); j++) {
		for (auto k = j->operands.begin(); k != j->operands.end(); k++) {
			if (k->isExpr()) {
				k->index = offset[k->index];
			}
		}

		offset.push_back(push(*j));
	}

	Operand op1 = Operand::exprOf(offset.back());
	push(Operation(func, {op0, op1}));
}

value Expression::evaluate(state values) const
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

bool Expression::is_null() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isVar()
				or (j->isConst()
					and not j->cnst.isUnstable())) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::is_constant() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and j->cnst.isUnstable())) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::is_valid() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isNeutral()))) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::is_neutral() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isValid()))) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::is_wire() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (i->func == 3 or i->func == 4 or i->func == 22 or i->func == 23) {
			return true;
		}
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isConst() and (j->cnst.isNeutral() or j->cnst.isValid())) {
				return true;
			}
		}
	}
	return false;
}

void Expression::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	for (int i = 0; i < (int)operations.size(); i++) {
		operations[i].apply(uid_map);
	}
}

void Expression::apply(vector<Expression> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	vector<Operand> top;
	int offset = 0;

	vector<Operation> result;
	for (int i = 0; i < (int)uid_map.size(); i++) {
		if (uid_map[i].operations.empty()) {
			top.push_back(false);
		} else {
			for (int j = 0; j < (int)uid_map[i].operations.size(); j++) {
				Operation op = uid_map[i].operations[j];
				op.offsetExpr(offset);
				result.push_back(op);
			}
			top.push_back(Operand::exprOf((int)result.size()-1));
		}
		offset = (int)result.size();
	}
	for (int i = 0; i < (int)operations.size(); i++) {
		result.push_back(operations[i]);
		result.back().offsetExpr(offset);
		result.back().apply(top);
	}

	operations = result;
}

// Erase the Operation at "index" from this Expression, fix all of the indexing issues that might create in other operations
void Expression::erase(size_t index) {
	for (int i = (int)operations.size()-1; i > (int)index; i--) {
		for (int j = (int)operations[i].operands.size()-1; j >= 0; j--) {
			if (operations[i].operands[j].isExpr()) {
				if (operations[i].operands[j].index == index) {
					operations[i].operands.erase(operations[i].operands.begin() + j);
				} else if (operations[i].operands[j].index > index) {
					operations[i].operands[j].index--;
				}
			}
		}
	}
	operations.erase(operations.begin()+index);
}

Expression &Expression::erase_dangling() {
	std::set<int> references;
	vector<int> toErase;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (i != (int)operations.size()-1 and references.find(i) != references.end()) {
			toErase.push_back(i);
		}

		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
			if (j->isExpr()) {
				if ((int)j->index < i) {
					references.insert(j->index);
				}
			}
		}
	}

	for (auto i = toErase.begin(); i != toErase.end(); i++) {
		erase(*i);
	}
	return *this;
}

void Expression::replace(Operand o0, Operand o1) {
	
}

Expression &Expression::propagate_constants() {
	vector<Operand> replace(operations.size(), Operand::exprOf(-1));
	// propagate constants up through the precedence levels
	for (int i = 0; i < (int)operations.size(); i++) {
		// merge adjacent constants from left to right as a result of operator precedence
		auto j = operations[i].operands.begin();
		if (j->isExpr() and replace[j->index].isConst()) {
			*j = replace[j->index];
		}

		while (j != operations[i].operands.end() and next(j) != operations[i].operands.end()) {
			auto k = next(j);
			if (k->isExpr() and replace[k->index].isConst()) {
				*k = replace[k->index];
			}

			if (j->isConst() and k->isConst()) {
				*j = Operation::evaluate(operations[i].func, {j->get(), k->get()});
				operations[i].operands.erase(k);
			} else {
				j++;
			}
		}

		if (operations[i].operands.size() == 1u and operations[i].operands[0].isConst() and i != (int)operations.size()-1) {
			Operand v = Operation::evaluate(operations[i].func, {operations[i].operands[0].get()});
			replace[i] = v;
		}
	}

	// eliminate dangling expressions
	std::set<int> references;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (replace[i].isConst()) {
			continue;
		}

		if (i != (int)operations.size()-1 and references.find(i) != references.end()) {
			replace[i].type = Operand::CONST;
		}

		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
			if (j->isExpr() and (int)j->index < i) {
				references.insert(j->index);
			}
		}
	}

	for (int i = (int)replace.size()-1; i >= 0; i--) {
		if (replace[i].isConst()) {
			erase(i);
		}
	}
	return *this;
}



Expression &Expression::minimize(Expression rewrite) {
	// TODO(edward.bingham) I need a way to canonicalize expressions and hash
	// them so that I can do the state search algorithm.
	/*if (rewrite.operations.empty()) {
		rewrite = basic_rewrite();
	}

	struct token {
		int rule;
		// (index of this.operations[], index of rule.operations[])
		vector<pair<Operand, Operand> > leaves;
		vector<pair<Operand, Operand> > branches;

		// map variable index to Operand in this
		map<int, Operand> mapping;
	};

	vector<token> tokens;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		// search through the current stack and propagate subsequent tokens
		for (auto t = tokens.begin(); t != tokens.end(); t++) {
			auto rule = rewrite.begin()+t->rule;
			for (auto l = t->leaves.begin(); l != t->leaves.end(); l++) {
				// top of the Expression tree "->"
				//rule->operations.back()

				// At a given leaf, I need to match all of the operators in the rewrite rule to some of the operators in this Expression. This needs to account for commutivity. Associativity is encoded in the rewrite rules.
				
			}
		}

		// search through the "rewrite" rules and add all of the matching starts
		for (int j = 0; j < (int)rewrite.size(); j++) {
			if (rewrite[j].operations.empty()) {
				continue;
			}
			auto rule = std::prev(rewrite[j].operations.end());
			// func must be rewrite "->"
			if (rule->func != 24 or rule->operands.size() != 2u) {
				continue;
			} 
			auto lhs = rule.operands.begin();
			auto rhs = std::next(lhs);
			if (lhs->isConst()
				or lhs->isVar()) {
				// an Operation matches a constant or
				// variable if the Operation is identity
				// ("+") and has a single Operand that is a
				// constant

				for (int k = 0; k < (int)operations[i].operands.size(); k++) {
					if (operations[i].operands[k].type == lhs->type
						and ((lhs->isConst()
							and operations[i].operands[k].index == lhs->index)
							or lhs->isVar())
						and operations[i].operands[k].hasData == lhs->hasData
						and operations[i].operands[k].hasValid == lhs->hasValid) {
						// we found a matching Operand
					}
				}
				
			} else if (lhs->isExpr()) {
				auto expr = rewrite[j].operations.begin() + lhs->index;
				if (expr->func == operations[i].func
					and (expr->operands.size() == operations[i].operands.size()
						or (expr->operands.size() < operations[i].operands.size()
							and expr->is_commutative()))) {
					// we can start a matching Expression
				}
			}
		}
	}*/



	// TODO(edward.bingham) implement Expression simplification using term replacement
	// 1. find a way to represent replacement rules
	//   a. regular expressions-like thing from the root of an Expression or from
	//   the leaves? What are L-Systems? What about just Expression->Expression?
	//   Rete's algorithm?
	// 2. create a library of replacement rules
	//   a. A replacement rule could be unidirectional or bidirectional.
	// 3. follow all unidirectional replacement rules that match
	// 4. search all bidirectional replacement rules that match
	// 5. repeat 3 and 4
	// 6. ensure the Expression is left in a normal form
	//   a. push all constants to the left
	//   b. prefer addition of negative over subtraction
	//   c. prefer multiplication of inverse over division
	//   d. merge all constants and all same-term literals

	// Maybe express normal form using a collection of unidirectional rewrite rules?
	// Take a look at rewrite.cpp for example rewrite rules
	// How do I represent a rewrite rule?
	// pair of expressions
	// How do I match?
	// A literal in the key is a literal, constant, or sub Expression in the
	// matched Expression. Token propagation of matched subexpressions.
	// How do I manage the list of rewrite rules?
	// Load them into a static variable on program initialization.
	// Is this an efficient representation of the rewrite rules?
	// Rete's algorithm doesn't handle search, it's continuous evaluation.
	// Regular expressions? All of the rewrite rules can be represented in a
	// single Expression structure where the rewrite operator can be represented
	// by the Operation("->", key, rewrite).

	// structure encodings as a set of rewrite rules and canonicalization rules
	return *this;
}

Expression &Expression::operator=(Operand e)
{
	set("", e);
	return *this;
}

string Expression::to_string(int index) {
	if (operations.empty()) {
		return "(gnd)";
	}

	if (index < 0) {
		index = (int)operations.size()-1;
	}

	stringstream os;
	os << "(";
	if (index < (int)operations.size()) {
		os << operations[index];
	} else {
		os << "{out of range " << index << "/" << operations.size() << "}";
	}
	os << ")";
	return os.str();
}

ostream &operator<<(ostream &os, Expression e)
{
	os << e.to_string();
	return os;
}

Expression operator!(Expression e)
{
	Expression result;
	result.set("!", e);
	return result;
}

Expression operator-(Expression e)
{
	Expression result;
	result.set("-", e);
	return result;
}

Expression is_valid(Expression e)
{
	if (e.is_valid()) {
		return Operand(true);
	} else if (e.is_neutral()) {
		return Operand(false);
	} else if (e.is_wire()) {
		return e;
	}
	return Expression("(bool)", e);
}

Expression operator~(Expression e)
{
	if (e.is_valid()) {
		return Operand(false);
	} else if (e.is_neutral()) {
		return Operand(true);
	}
	return Expression("~", e);
}

Expression operator||(Expression e0, Expression e1)
{
	Expression result;
	result.set("||", e0, e1);
	return result;
}

Expression operator&&(Expression e0, Expression e1)
{
	Expression result;
	result.set("&&", e0, e1);
	return result;
}

Expression operator^(Expression e0, Expression e1)
{
	Expression result;
	result.set("^", e0, e1);
	return result;
}

Expression operator==(Expression e0, Expression e1)
{
	Expression result;
	result.set("==", e0, e1);
	return result;
}

Expression operator!=(Expression e0, Expression e1)
{
	Expression result;
	result.set("~=", e0, e1);
	return result;
}

Expression operator<(Expression e0, Expression e1)
{
	Expression result;
	result.set("<", e0, e1);
	return result;
}

Expression operator>(Expression e0, Expression e1)
{
	Expression result;
	result.set(">", e0, e1);
	return result;
}

Expression operator<=(Expression e0, Expression e1)
{
	Expression result;
	result.set("<=", e0, e1);
	return result;
}

Expression operator>=(Expression e0, Expression e1)
{
	Expression result;
	result.set(">=", e0, e1);
	return result;
}

Expression operator<<(Expression e0, Expression e1)
{
	Expression result;
	result.set("<<", e0, e1);
	return result;
}

Expression operator>>(Expression e0, Expression e1)
{
	Expression result;
	result.set(">>", e0, e1);
	return result;
}

Expression operator+(Expression e0, Expression e1)
{
	Expression result;
	result.set("+", e0, e1);
	return result;
}

Expression operator-(Expression e0, Expression e1)
{
	Expression result;
	result.set("-", e0, e1);
	return result;
}

Expression operator*(Expression e0, Expression e1)
{
	Expression result;
	result.set("*", e0, e1);
	return result;
}

Expression operator/(Expression e0, Expression e1)
{
	Expression result;
	result.set("/", e0, e1);
	return result;
}

Expression operator%(Expression e0, Expression e1)
{
	Expression result;
	result.set("%", e0, e1);
	return result;
}

Expression operator&(Expression e0, Expression e1)
{
	if (e0.is_null() or e1.is_null()) {
		return Expression(value::X());
	} else if (e0.is_neutral() or e1.is_neutral()) {
		return Operand(false);
	} else if (e0.is_valid()) {
		return is_valid(e1);
	} else if (e1.is_valid()) {
		return is_valid(e0);
	}
	return Expression("&", e0, e1);
}

Expression operator|(Expression e0, Expression e1)
{
	if (e0.is_null() or e1.is_null()) {
		return Expression(value::X());
	} if (e0.is_valid() or e1.is_valid()) {
		return Operand(true);
	} else if (e0.is_neutral()) {
		return is_valid(e1);
	} else if (e1.is_neutral()) {
		return is_valid(e0);
	}
	return Expression("|", e0, e1);
}

Expression operator|(Expression e0, Operand e1)
{
	Expression result;
	result.set("|", e0, e1);
	return result;
}

Expression operator&(Expression e0, Operand e1)
{
	Expression result;
	result.set("&", e0, e1);
	return result;
}

Expression operator^(Expression e0, Operand e1)
{
	Expression result;
	result.set("^", e0, e1);
	return result;
}

Expression operator==(Expression e0, Operand e1)
{
	Expression result;
	result.set("==", e0, e1);
	return result;
}

Expression operator!=(Expression e0, Operand e1)
{
	Expression result;
	result.set("~=", e0, e1);
	return result;
}

Expression operator<(Expression e0, Operand e1)
{
	Expression result;
	result.set("<", e0, e1);
	return result;
}

Expression operator>(Expression e0, Operand e1)
{
	Expression result;
	result.set(">", e0, e1);
	return result;
}

Expression operator<=(Expression e0, Operand e1)
{
	Expression result;
	result.set("<=", e0, e1);
	return result;
}

Expression operator>=(Expression e0, Operand e1)
{
	Expression result;
	result.set(">=", e0, e1);
	return result;
}

Expression operator<<(Expression e0, Operand e1)
{
	Expression result;
	result.set("<<", e0, e1);
	return result;
}

Expression operator>>(Expression e0, Operand e1)
{
	Expression result;
	result.set(">>", e0, e1);
	return result;
}

Expression operator+(Expression e0, Operand e1)
{
	Expression result;
	result.set("+", e0, e1);
	return result;
}

Expression operator-(Expression e0, Operand e1)
{
	Expression result;
	result.set("-", e0, e1);
	return result;
}

Expression operator*(Expression e0, Operand e1)
{
	Expression result;
	result.set("*", e0, e1);
	return result;
}

Expression operator/(Expression e0, Operand e1)
{
	Expression result;
	result.set("/", e0, e1);
	return result;
}

Expression operator%(Expression e0, Operand e1)
{
	Expression result;
	result.set("%", e0, e1);
	return result;
}

Expression operator&&(Expression e0, Operand e1)
{
	Expression result;
	result.set("&&", e0, e1);
	return result;
}

Expression operator||(Expression e0, Operand e1)
{
	Expression result;
	result.set("||", e0, e1);
	return result;
}

Expression operator|(Operand e0, Expression e1)
{
	Expression result;
	result.set("|", e0, e1);
	return result;
}

Expression operator&(Operand e0, Expression e1)
{
	Expression result;
	result.set("&", e0, e1);
	return result;
}

Expression operator^(Operand e0, Expression e1)
{
	Expression result;
	result.set("^", e0, e1);
	return result;
}

Expression operator==(Operand e0, Expression e1)
{
	Expression result;
	result.set("==", e0, e1);
	return result;
}

Expression operator!=(Operand e0, Expression e1)
{
	Expression result;
	result.set("~=", e0, e1);
	return result;
}

Expression operator<(Operand e0, Expression e1)
{
	Expression result;
	result.set("<", e0, e1);
	return result;
}

Expression operator>(Operand e0, Expression e1)
{
	Expression result;
	result.set(">", e0, e1);
	return result;
}

Expression operator<=(Operand e0, Expression e1)
{
	Expression result;
	result.set("<=", e0, e1);
	return result;
}

Expression operator>=(Operand e0, Expression e1)
{
	Expression result;
	result.set(">=", e0, e1);
	return result;
}

Expression operator<<(Operand e0, Expression e1)
{
	Expression result;
	result.set("<<", e0, e1);
	return result;
}

Expression operator>>(Operand e0, Expression e1)
{
	Expression result;
	result.set(">>", e0, e1);
	return result;
}

Expression operator+(Operand e0, Expression e1)
{
	Expression result;
	result.set("+", e0, e1);
	return result;
}

Expression operator-(Operand e0, Expression e1)
{
	Expression result;
	result.set("-", e0, e1);
	return result;
}

Expression operator*(Operand e0, Expression e1)
{
	Expression result;
	result.set("*", e0, e1);
	return result;
}

Expression operator/(Operand e0, Expression e1)
{
	Expression result;
	result.set("/", e0, e1);
	return result;
}

Expression operator%(Operand e0, Expression e1)
{
	Expression result;
	result.set("%", e0, e1);
	return result;
}

Expression operator&&(Operand e0, Expression e1)
{
	Expression result;
	result.set("&&", e0, e1);
	return result;
}

Expression operator||(Operand e0, Expression e1)
{
	Expression result;
	result.set("||", e0, e1);
	return result;
}

Expression operator|(Operand e0, Operand e1)
{
	Expression result;
	result.set("|", e0, e1);
	return result;
}

Expression operator&(Operand e0, Operand e1)
{
	Expression result;
	result.set("&", e0, e1);
	return result;
}

Expression operator^(Operand e0, Operand e1)
{
	Expression result;
	result.set("^", e0, e1);
	return result;
}

Expression operator==(Operand e0, Operand e1)
{
	Expression result;
	result.set("==", e0, e1);
	return result;
}

Expression operator!=(Operand e0, Operand e1)
{
	Expression result;
	result.set("~=", e0, e1);
	return result;
}

Expression operator<(Operand e0, Operand e1)
{
	Expression result;
	result.set("<", e0, e1);
	return result;
}

Expression operator>(Operand e0, Operand e1)
{
	Expression result;
	result.set(">", e0, e1);
	return result;
}

Expression operator<=(Operand e0, Operand e1)
{
	Expression result;
	result.set("<=", e0, e1);
	return result;
}

Expression operator>=(Operand e0, Operand e1)
{
	Expression result;
	result.set(">=", e0, e1);
	return result;
}

Expression operator<<(Operand e0, Operand e1)
{
	Expression result;
	result.set("<<", e0, e1);
	return result;
}

Expression operator>>(Operand e0, Operand e1)
{
	Expression result;
	result.set(">>", e0, e1);
	return result;
}

Expression operator+(Operand e0, Operand e1)
{
	Expression result;
	result.set("+", e0, e1);
	return result;
}

Expression operator-(Operand e0, Operand e1)
{
	Expression result;
	result.set("-", e0, e1);
	return result;
}

Expression operator*(Operand e0, Operand e1)
{
	Expression result;
	result.set("*", e0, e1);
	return result;
}

Expression operator/(Operand e0, Operand e1)
{
	Expression result;
	result.set("/", e0, e1);
	return result;
}

Expression operator%(Operand e0, Operand e1)
{
	Expression result;
	result.set("%", e0, e1);
	return result;
}

Expression operator&&(Operand e0, Operand e1)
{
	Expression result;
	result.set("&&", e0, e1);
	return result;
}

Expression operator||(Operand e0, Operand e1)
{
	Expression result;
	result.set("||", e0, e1);
	return result;
}

Expression array(vector<Expression> e) {
	Expression result;
	result.set("[,]", e);
	return result;
}

int passes_guard(const state &encoding, const state &global, const Expression &guard, state *total) {
	vector<value> expressions;
	vector<value> gexpressions;

	for (int i = 0; i < (int)guard.operations.size(); i++) {
		value g = guard.operations[i].evaluate(global, gexpressions);
		value l = guard.operations[i].evaluate(encoding, expressions);

		if (l.isUnstable() or g.isUnstable()
			or (g.isNeutral() and l.isValid())
			or (g.isValid() and l.isNeutral())
			or (g.isValid() and l.isValid() and not areSame(g, l))) {
			l = value::X();
		}

		expressions.push_back(l);
		gexpressions.push_back(g);
	}

	if (expressions.back().isUnknown() or expressions.back().isValid()) {
		if (gexpressions.back().isNeutral() or gexpressions.back().isUnknown()) {
			expressions.back() = value::X();
		} else if (gexpressions.back().isValid()) {
			expressions.back() = gexpressions.back();
		}
	}

	// If the final value in the Expression stack is valid, then we've passed the
	// guard, and we can back propagate information back to individual variables.
	// If it is neutral, then we have not yet passed the guard and therefore
	// cannot propagate information back.

	// If the guard passes, then we have waited for every transition on the
	// variable. Therefore, we know not only that the input signals are valid or
	// neutral, but we also know their value. This valid is copied over from the
	// global state.

	// This validity/neutrality information propagates differently through
	// different operations.
	if (total != nullptr and expressions.back().isValid()) {
		for (int i = (int)guard.operations.size()-1; i >= 0; i--) {
			guard.operations[i].propagate(*total, global, expressions, gexpressions, expressions[i]);
		}
	}

	if (expressions.empty() or expressions.back().isNeutral()) {
		return -1;
	} else if (expressions.back().isUnstable()) {
		return 0;
	}
	
	return 1;
}

Expression weakest_guard(const Expression &guard, const Expression &exclude) {
	// TODO(edward.bingham) Remove terms from the guard until guard overlaps exclude (using cylidrical algebraic decomposition)
	// 1. put the guard in conjunctive normal form using the boolean operations & | ~
	// 2. for each term in the conjunctive normal form, pick a comparison and eliminate it, then check overlap. 
	return guard;
}

}
