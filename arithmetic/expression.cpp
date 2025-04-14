#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>

namespace arithmetic
{

vector<Operator> Operation::operators;
int Operation::BITWISE_NOT = -1;
int Operation::IDENTITY = -1;
int Operation::NEGATION = -1;
int Operation::VALIDITY = -1;
int Operation::BOOLEAN_NOT = -1;
int Operation::BITWISE_OR = -1;
int Operation::BITWISE_AND = -1;
int Operation::BITWISE_XOR = -1;
int Operation::EQUAL = -1;
int Operation::NOT_EQUAL = -1;
int Operation::LESS = -1;
int Operation::GREATER = -1;
int Operation::LESS_EQUAL = -1;
int Operation::GREATER_EQUAL = -1;
int Operation::SHIFT_LEFT = -1;
int Operation::SHIFT_RIGHT = -1;
int Operation::ADD = -1;
int Operation::SUBTRACT = -1;
int Operation::MULTIPLY = -1;
int Operation::DIVIDE = -1;
int Operation::MOD = -1;
int Operation::TERNARY = -1;
int Operation::BOOLEAN_OR = -1;
int Operation::BOOLEAN_AND = -1;
int Operation::BOOLEAN_XOR = -1;
int Operation::ARRAY = -1;
int Operation::INDEX = -1;

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
	} else {
		os << "error(" << o.type << ")";
	}
	return os;
}

bool areSame(Operand o0, Operand o1) {
	return ((o0.isConst() and o1.isConst() and areSame(o0.cnst, o1.cnst))
		or (o0.isVar() and o1.isVar() and o0.index == o1.index)
		or (o0.isExpr() and o1.isExpr() and o0.index == o1.index));
}

Operator::Operator() {
	commutative = false;
	reflexive = true;
}

Operator::Operator(vector<string> infix, string prefix, string postfix, bool commutative, bool reflexive) {
	this->prefix = prefix;
	this->infix = infix;
	this->postfix = postfix;
	this->commutative = commutative;
	this->reflexive = reflexive;
}

Operator::~Operator() {
}

bool canMap(Operand o0, Operand o1, const Expression &e0, const Expression &e1, bool init, map<size_t, Operand> *vars) {
	if (o1.isConst()) {
		return o0.isConst() and areSame(o0.cnst, o1.cnst);
	} else if (o1.isVar()) {
		if (vars != nullptr) {
			auto ins = vars->insert({o1.index, o0});
			return ins.second or areSame(o0, ins.first->second);
		}
		return true;
	} else if (o1.isExpr()) {
		if (not o0.isExpr()) {
			return false;
		}
		auto op0 = e0.operations.begin() + o0.index;
		auto op1 = e1.operations.begin() + o1.index;
		return (op0->func == op1->func
			and (op0->operands.size() == op1->operands.size()
				or ((op1->isCommutative() or init)
					and op0->operands.size() > op1->operands.size())));
	}
	return false;
}

Operation::Operation() {
	func = -1;
}

Operation::Operation(int func, vector<Operand> args) {
	set(func, args);
}

Operation::~Operation() {
}

int Operation::push(Operator op) {
	int result = (int)operators.size();
	operators.push_back(op);
	return result;
}

void Operation::loadOperators() {
	// DESIGN(edward.bingham) bitwise and boolean operations have been switched
	// to be consistent with HSE and boolean logic expressions

	// DESIGN(edward.bingham) order of these operations matters for the propagate function!

	// DESIGN(edward.bingham) Channel receive will not be used as an operator in
	// the expression engine. Channel actions should be decomposed into their
	// appropriate protocols while expanding the CHP.

	if (Operation::operators.empty()) {
		printf("loading operators\n");

		BITWISE_NOT   = push(Operator({}, "!"));
		IDENTITY      = push(Operator({}, "+", "", false, true));
		NEGATION      = push(Operator({}, "-"));
		VALIDITY      = push(Operator({}, "(bool)"));
		BOOLEAN_NOT   = push(Operator({}, "~"));
		BITWISE_OR    = push(Operator({"||"}, "", "", true));
		BITWISE_AND   = push(Operator({"&&"}, "", "", true));
		BITWISE_XOR   = push(Operator({"^^"}, "", "", true));
		EQUAL         = push(Operator({"=="}));
		NOT_EQUAL     = push(Operator({"!="}));
		LESS          = push(Operator({"<"}));
		GREATER       = push(Operator({">"}));
		LESS_EQUAL    = push(Operator({"<="}));
		GREATER_EQUAL = push(Operator({">="}));
		SHIFT_LEFT    = push(Operator({"<<"}));
		SHIFT_RIGHT   = push(Operator({">>"}));
		ADD           = push(Operator({"+"}, "", "", true));
		SUBTRACT      = push(Operator({"-"}));
		MULTIPLY      = push(Operator({"*"}, "", "", true));
		DIVIDE        = push(Operator({"/"}));
		MOD           = push(Operator({"%"}));
		TERNARY       = push(Operator({"?", ":"}));
		BOOLEAN_OR    = push(Operator({"|"}, "", "", true));
		BOOLEAN_AND   = push(Operator({"&"}, "", "", true));
		BOOLEAN_XOR   = push(Operator({"^"}, "", "", true));
		ARRAY         = push(Operator({","}, "[", "]"));
		INDEX         = push(Operator({"[", "]["}, "", "]"));

		printf("loaded %d operators\n", (int)Operation::operators.size());
	} 
}

pair<Type, double> Operation::funcCost(int func, vector<Type> args) {
	// If I have two fixed-point inputs, then the offset determines the
	// complexity of the operator because it can affect the overlap of the two
	// inputs. Again, because encodings may not be base-2, the offset may not be
	// an integer.

	int i;
	Type result(0.0, 0.0, 0.0);
	double cost = 0.0;
	std::array<double, 2> ovr;
	if (func == Operation::BITWISE_NOT) { // bitwise not (!) -- rotate digit for non-base-2
		result = args[0];
		result.delay += 1.0;
		cost = args[0].width;
		return {result, cost};
	} else if (func == Operation::IDENTITY) { // identity
		result = args[0];
		cost = 0.0;
		return {result, cost};
	} else if (func == Operation::NEGATION) { // negation (-)
		result = args[0];
		result.delay += args[0].width;
		cost = args[0].width;
		return {result, cost};
	} else if (func == Operation::VALIDITY
		or func == Operation::BOOLEAN_NOT) { // boolean not (~)
		result = Type(0.0, 0.0, log2(args[0].width));
		cost = args[0].width;
		return {result, cost};
	} else if (func == Operation::BITWISE_OR) { // bitwise or (||) -- max of digit for non-base-2
		result = args[0];
		for (i = 1; i < (int)args.size(); i++) {
			ovr = overlap(result, args[i]);
			cost += ovr[0];
			result.width = ovr[1];
			if (args[i].coeff < result.coeff) {
				result.coeff = args[i].coeff;
			}
			result.delay = max(result.delay, args[i].delay);
		}
		result.delay += (double)args.size() - 1.0;
		return {result, cost};
	} else if (func == Operation::BITWISE_AND) { // bitwise and (&&)
		result = args[0];
		for (i = 1; i < (int)args.size(); i++) {
			ovr = overlap(result, args[i]);
			result.width = ovr[0];
			if (args[i].coeff > result.coeff) {
				result.coeff = args[i].coeff;
			}
			result.delay = max(result.delay, args[i].delay);
		}
		result.delay += (double)args.size() - 1.0;
		cost = result.width*(double)(args.size()-1);
		return {result, cost};
	} else if (func == Operation::EQUAL
		or func == Operation::NOT_EQUAL) { // inequality (!=)
		result = Type(0.0, 0.0, 0.0);
		ovr = overlap(args[0], args[1]);
		result.delay = max(args[0].delay, args[1].delay) + log2(ovr[1]);
		cost = ovr[1];
		return {result, cost};
	} else if (func == Operation::LESS
		or func == Operation::GREATER
		or func == Operation::LESS_EQUAL
		or func == Operation::GREATER_EQUAL) { // ">=";
		result = Type(0.0, 0.0, 0.0);
		ovr = overlap(args[0], args[1]);
		result.delay = max(args[0].delay, args[1].delay) + ovr[1];
		cost = ovr[1];
		return {result, cost};
	} else if (func == Operation::SHIFT_LEFT) { // shift left  "<<" (arithmetic/logical based on type?)
		result = Type(args[0].coeff*pow(2.0, args[1].coeff),
		              args[0].width + args[1].coeff*pow(2.0, args[1].width) - args[1].coeff,
									max(args[0].delay, args[1].delay)+args[1].width);
		cost = args[0].width*args[1].width;
		return {result, cost};
	} else if (func == Operation::SHIFT_RIGHT) { // shift right ">>" (arithmetic/logical based on type?)
		result = Type(args[0].coeff/pow(2.0, args[1].coeff*pow(2.0, args[1].width)),
		              args[0].width + args[1].coeff*pow(2.0, args[1].width) - args[1].coeff,
									max(args[0].delay, args[1].delay)+args[1].width);
		cost = args[0].width*args[1].width;
		return {result, cost};
	} else if (func == Operation::ADD or func == Operation::SUBTRACT) { // subtraction "-"
		// TODO(edward.bingham) I should really keep track of overlapping intervals
		// here and then the critical path is the total overlap at the bit with
		// maximum overlap. If there are multiple bits with the same maximum
		// overlap, then we use the least significant one. Then we look at the
		// location of the first overlap relative to the width of the final result
		// and add that in for the carry chain. Complexity is dependent upon the
		// wallace/dadda tree used to implement the reduction along with the ripple
		// carry or carry look-ahead used to implement the final sum.
		result = args[0];
		cost = 0.0;
		for (i = 1; i < (int)args.size(); i++) {
			ovr = overlap(result, args[i]);
			if (ovr[0] > 0.0) {
				cost += ovr[0];
			}
			
			result.width = ovr[1];
			if (args[i].coeff < result.coeff) {
				result.coeff = args[i].coeff;
			}
			result.delay = max(result.delay, args[i].delay);
		}
		result.delay += (double)args.size() + result.width;
		return {result, cost};
	} else if (func == Operation::MULTIPLY) { // multiply "*"
		// TODO(edward.bingham) This assumes a set of N sequental multiplications.
		// Doing so completely ignores multiplication trees and bit-level
		// parallelism. It's likely going to make iterative multiplication look
		// more expensive than it really is.
		result = args[0];
		cost = 0.0;
		for (i = 1; i < (int)args.size(); i++) {
			cost += result.width*args[i].width;
			result.width += args[i].width;
			result.coeff *= args[i].coeff;
			result.delay = max(result.delay, args[i].delay);
		}
		result.delay += result.width;
		return {result, cost};
	} else if (func == Operation::DIVIDE or func == Operation::MOD) { // modulus "%"
		// TODO(edward.bingham) I'm really not sure about this
		result = args[0];
		cost = 0.0;
		for (i = 1; i < (int)args.size(); i++) {
			cost += result.width*args[i].width;
			result.delay = max(result.delay, args[i].delay) + result.width*args[i].width;
			result.coeff /= args[i].coeff;
		}
		return {result, cost};
	} else if (func == Operation::TERNARY) { // ternary operator "a ? b : c"
		ovr = overlap(args[1], args[2]);
		result.coeff = min(args[1].coeff, args[2].coeff);
		result.width = ovr[1];
		result.delay = max(max(args[0].delay, args[1].delay), args[2].delay) + 1.0;
		cost = ovr[1];
		return {result, cost};
	} else if (func == Operation::BOOLEAN_AND
		or func == Operation::BOOLEAN_OR
		or func == Operation::BOOLEAN_XOR) { // boolean AND "&"
		result = Type(0.0, 0.0, args[0].delay);
		for (i = 1; i < (int)args.size(); i++) {
			result.delay = max(result.delay, args[i].delay);
		}
		cost = 0.0;
		return {result, cost};
	} else if (func == Operation::ARRAY) { // array ","
		// TODO(edward.bingham) we need to create a way to handle arrayed types
		result = args[0];
		for (i = 1; i < (int)args.size(); i++) {
			result.delay = max(result.delay, args[i].delay);
		}
		result.bounds.push_back((int)args.size());
		cost = 0.0;
		return {result, cost};
	} else if (func == Operation::INDEX) { // index "."
		// Need to select the appropriate type from the array
		// but we can compute cost
		result = args[0];
		result.bounds.pop_back();
		result.delay = max(result.delay, args[1].delay)+1.0;
		cost = pow(2.0, args[1].width);
		return {result, cost};
	}
	return {result, cost};
}

void Operation::set(int func, vector<Operand> args) {
	this->func = func;
	this->operands = args;
}

bool Operation::isCommutative() const {
	if (func >= 0 and func < (int)Operation::operators.size()) {
		return Operation::operators[func].commutative;
	}
	return false;
}

bool Operation::isReflexive() const {
	if (func >= 0 and func < (int)Operation::operators.size()) {
		return Operation::operators[func].reflexive;
	}
	return true;
}

value Operation::evaluate(int func, vector<value> args) {
	if (func == Operation::BITWISE_NOT) {
		return !args[0];
	} else if (func == Operation::IDENTITY) {
		return args[0];
	} else if (func == Operation::NEGATION) {
		return -args[0];
	} else if (func == Operation::VALIDITY) {
		return valid(args[0]);
	} else if (func == Operation::BOOLEAN_NOT) {
		return ~args[0];
	} else if (func == Operation::BITWISE_OR) {
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] || args[i];
		}
		return args[0];
	} else if (func == Operation::BITWISE_AND) {
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] && args[i];
		}
		return args[0];
	} else if (func == Operation::BOOLEAN_XOR) {
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] ^ args[i];
		}
		return args[0];
	} else if (func == Operation::EQUAL) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] == args[1]);
	} else if (func == Operation::NOT_EQUAL) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] != args[1]);
	} else if (func == Operation::LESS) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] <  args[1]);
	} else if (func == Operation::GREATER) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] >  args[1]);
	} else if (func == Operation::LESS_EQUAL) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] <= args[1]);
	} else if (func == Operation::GREATER_EQUAL) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] >= args[1]);
	} else if (func == Operation::SHIFT_LEFT) {
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] << args[1]);
	} else if (func == Operation::SHIFT_RIGHT) { 
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] >> args[1]);
	} else if (func == Operation::ADD) { 
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] + args[i];
		}
		return args[0];
	} else if (func == Operation::SUBTRACT) { 
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] - args[i];
		}
		return args[0];
	} else if (func == Operation::MULTIPLY) { 
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] * args[i];
		}
		return args[0];
	} else if (func == Operation::DIVIDE) { 
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] /  args[1]);
	} else if (func == Operation::MOD) { 
		if (args.size() == 1u) {
			return args[0];
		}
		return (args[0] %  args[1]);
	} else if (func == Operation::TERNARY) { // ternary operator
		if (args.size() == 1u) {
			return args[0];
		} else if (args.size() == 2u) {
			args.push_back(value::X());
		}
		return args[0] ? args[1] : args[2];
	} else if (func == Operation::BOOLEAN_AND) { 
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] & args[i];
		}
		return args[0];
	} else if (func == Operation::BOOLEAN_OR) { 
		for (int i = 1; i < (int)args.size(); i++) {
			args[0] = args[0] | args[i];
		}
		return args[0];
	} else if (func == Operation::ARRAY) { // concat arrays
		return value::arrOf(args);
	} else if (func == Operation::INDEX) { // index
		return index(args[0], args[1]);
	}
	return args[0];
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
	os << "f(" << o.func << "): ";
	Operator op;
	if (o.func >= 0 and o.func < (int)Operation::operators.size()) {
		op = Operation::operators[o.func];
	} else {
		op = Operation::operators[Operation::IDENTITY];
		printf("error: unrecognized operator\n");
	}

	os << op.prefix;

	for (int i = 0; i < (int)o.operands.size(); i++) {
		if (i != 0 and i-1 < (int)op.infix.size()) {
			os << op.infix[i-1];
		} else if (i != 0 and not op.infix.empty()) {
			os << op.infix.back();
		}
		os << o.operands[i];
	}
	os << op.postfix;
	return os;
}

Expression::Expression() {
}

Expression::Expression(Operand arg0) {
	set(arg0);
}

Expression::Expression(int func, Operand arg0) {
	set(func, arg0);
}

Expression::Expression(int func, Expression arg0) {
	set(func, arg0);
}

Expression::Expression(int func, Operand arg0, Operand arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(int func, Expression arg0, Operand arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(int func, Operand arg0, Expression arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(int func, Expression arg0, Expression arg1) {
	set(func, arg0, arg1);
}

Expression::Expression(int func, vector<Expression> args) {
	set(func, args);
}

Expression::~Expression() {
}

int Expression::push(Operation arg) {
	// add to operations list if doesn't exist
	int pos = -1;
	for (int i = 0; i < (int)operations.size() and pos < 0; i++) {
		if (areSame(operations[i], arg)) {
			pos = i;
		}
	}
	if (pos < 0) {
		pos = operations.size();
		operations.push_back(arg);
	}
	return pos;
}

void Expression::set(Operand arg0) {
	operations.clear();
	operations.push_back(Operation(Operation::IDENTITY, {arg0}));
}

void Expression::set(int func, Operand arg0) {
	operations.clear();
	operations.push_back(Operation(func, {arg0}));
}

void Expression::set(int func, Expression arg0) {
	operations = arg0.operations;
	Operand op0 = Operand::exprOf(operations.size()-1);
	while (op0.isExpr() and operations.back().isReflexive()) {
		op0 = operations.back().operands[0];
		operations.pop_back();
	}
	push(Operation(func, {op0}));
}

void Expression::set(int func, Operand arg0, Operand arg1) {
	operations.clear();
	operations.push_back(Operation(func, {arg0, arg1}));
}

void Expression::set(int func, Expression arg0, Expression arg1) {
	operations = arg0.operations;
	Operand op0 = Operand::exprOf(operations.size()-1);
	while (op0.isExpr() and operations.back().isReflexive()) {
		op0 = operations.back().operands[0];
		operations.pop_back();
	}

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
	while (op1.isExpr() and operations.back().isReflexive()) {
		op1 = operations.back().operands[0];
		operations.pop_back();
	}

	push(Operation(func, {op0, op1}));
}

void Expression::set(int func, Expression arg0, Operand arg1)
{
	operations = arg0.operations;
	Operand op0 = Operand::exprOf(operations.size()-1);
	while (op0.isExpr() and operations.back().isReflexive()) {
		op0 = operations.back().operands[0];
		operations.pop_back();
	}
	push(Operation(func, {op0, arg1}));
}

void Expression::set(int func, Operand arg0, Expression arg1)
{
	operations = arg1.operations;
	Operand op1 = Operand::exprOf(operations.size()-1);
	while (op1.isExpr() and operations.back().isReflexive()) {
		op1 = operations.back().operands[0];
		operations.pop_back();
	}
	push(Operation(func, {arg0, op1}));
}

void Expression::set(int func, vector<Expression> args)
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
		while (operands.back().isExpr() and operations.back().isReflexive()) {
			operands.back() = operations.back().operands[0];
			operations.pop_back();
		}
	}

	push(Operation(func, operands));
}

void Expression::push(int func) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	push(Operation(func, {op0}));
}

void Expression::push(int func, Operand arg0) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	push(Operation(func, {op0, arg0}));
}

void Expression::push(int func, Expression arg0) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	while (op0.isExpr() and operations.back().isReflexive()) {
		op0 = operations.back().operands[0];
		operations.pop_back();
	}

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
	while (op1.isExpr() and operations.back().isReflexive()) {
		op1 = operations.back().operands[0];
		operations.pop_back();
	}
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
		if (i->func == Operation::VALIDITY or i->func == Operation::BOOLEAN_NOT or i->func == Operation::BOOLEAN_AND or i->func == Operation::BOOLEAN_OR) {
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

void Expression::insert(size_t index, size_t num) {
	size_t sz = operations.size();
	operations.resize(sz+num);
	std::move(operations.begin()+index, operations.begin()+sz, operations.begin()+index+num);
	for (auto i = operations.begin()+index+num; i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (j->isExpr() and j->index >= index) {
				j->index += num;
			}
		}
	}
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

void Expression::erase(vector<size_t> index, bool doSort) {
	if (doSort) {
		sort(index.begin(), index.end());
	}

	int j = (int)operations.size()-1;
	for (auto i = index.rbegin(); i != index.rend(); i++) {
		for (; j > (int)*i; j--) {
			for (int k = (int)operations[j].operands.size()-1; k >= 0; k--) {
				if (operations[j].operands[k].isExpr()) {
					if (std::find(index.begin(), index.end(), operations[j].operands[k].index) != index.end()) {
						operations[j].operands.erase(operations[j].operands.begin() + k);
					} else {
						for (auto l = i; l != index.rend(); l++) {
							if (operations[j].operands[k].index > *l) {
								operations[j].operands[k].index--;
							}
						}
					}
				}
			}
		}

		operations.erase(operations.begin()+*i);
		j--;
	}
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

Expression &Expression::canonicalize() {
	vector<Operand> replace(operations.size(), Operand::varOf(0));
	// propagate constants up through the precedence levels
	for (int i = 0; i < (int)operations.size(); i++) {
		if (operations[i].isCommutative()) {
			sort(operations[i].operands.begin(), operations[i].operands.end(),
				[](const Operand &a, const Operand &b) {
					return a.type < b.type or (a.type == b.type
						and ((a.type == Operand::CONST and order(a.cnst, b.cnst) < 0)
							or (a.type != Operand::CONST and a.index < b.index)));
				});
		}

		// merge adjacent constants from left to right as a result of operator precedence
		auto j = operations[i].operands.begin();
		if (j->isExpr() and not replace[j->index].isVar()) {
			*j = replace[j->index];
		}

		while (j != operations[i].operands.end() and next(j) != operations[i].operands.end()) {
			auto k = next(j);
			if (k->isExpr() and not replace[k->index].isVar()) {
				*k = replace[k->index];
			}

			if (j->isConst() and k->isConst()) {
				*j = Operation::evaluate(operations[i].func, {j->get(), k->get()});
				operations[i].operands.erase(k);
			} else {
				j++;
			}
		}

		if (operations[i].operands.size() == 1u and operations[i].operands[0].isConst()) {
			replace[i] = Operation::evaluate(operations[i].func, {operations[i].operands[0].get()});
		} if (operations[i].operands.size() == 1u and operations[i].isCommutative()) {
			replace[i] = operations[i].operands[0];
		} else {
			for (int k = i-1; k >= 0; k--) {
				if (replace[k].isVar() and areSame(operations[i], operations[k])) {
					replace[i] = Operand::exprOf(k);
					break;
				}
			}
		}
	}

	// eliminate dangling expressions
	if (replace.back().isConst()) {
		operations.clear();
		set(Operation::IDENTITY, replace.back());
		return *this;
	} else if (replace.back().isExpr()) {
		size_t top = replace.back().index;
		operations.erase(operations.begin()+top+1, operations.end());
		replace.erase(replace.begin()+top+1, replace.end());
	}

	std::set<int> references;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (not replace[i].isVar() and i != (int)operations.size()-1) {
			continue;
		}

		if (i != (int)operations.size()-1 and references.find(i) == references.end()) {
			replace[i].type = Operand::CONST;
		}

		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
			if (j->isExpr() and (int)j->index < i) {
				references.insert(j->index);
			}
		}
	}

	for (int i = (int)replace.size()-1; i >= 0; i--) {
		if (not replace[i].isVar()) {
			erase(i);
		}
	}
	return *this;
}

struct CombinationIterator {
	std::vector<int> indices;
	int n, k;

	CombinationIterator(int n, int k) : n(n), k(k) {
		indices.resize(k);
		for (int i = 0; i < k; ++i) {
			indices[i] = i;
		}
	}

	~CombinationIterator() {
	}

	std::vector<int>::iterator begin() {
		return indices.begin();
	}

	std::vector<int>::iterator end() {
		return indices.end();
	}

	bool nextShift() {
		if (indices[0] >= n-k) {
			return false;
		}
		
		for (auto i = indices.begin(); i != indices.end(); i++) {
			(*i)++;
		}
		return true;
	}

	bool nextComb() {
		if (indices[0] >= n-k) {
			return false;
		}

		// Find the rightmost index that can be incremented
		for (int i = k - 1; i >= 0; --i) {
			if (indices[i] < n - k + i) {
				++indices[i];
				for (int j = i + 1; j < k; ++j) {
					indices[j] = indices[j - 1] + 1;
				}
				return true;
			}
		}

		return false;
	}

	bool nextPerm() {
		if (std::next_permutation(indices.begin(), indices.end())) {
			return true;
		}

		std::sort(indices.begin(), indices.end());
		return nextComb();
	}
};

Cost Expression::cost(vector<Type> vars) const {
	// TODO(edward.bingham) I need to handle arrays operations properly. Types
	// don't currently represent arrays the same way that values do.

	// TODO(edward.bingham) create a function in value that emits the proper type.

	double complexity = 0.0;
	vector<Type> expr;
	for (int i = 0; i < (int)operations.size(); i++) {
		vector<Type> args;
		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
			if (j->isConst()) {
				if (j->cnst.type == value::BOOL) {
					args.push_back(Type(0.0, 0.0, 0.0));
				} else if (j->cnst.type == value::INT) {
					args.push_back(Type((double)j->cnst.ival, 0.0, 0.0));
				} else if (j->cnst.type == value::REAL) {
					args.push_back(Type(j->cnst.rval, 0.0, 0.0));
				}
			} else if (j->isVar()) {
				args.push_back(vars[j->index]);
			} else if (j->isExpr()) {
				args.push_back(expr[j->index]);
			}
		}
		auto result = operations[i].funcCost(operations[i].func, args);
		expr.push_back(result.first);
		complexity += result.second;
	}

	double delay = 0.0;
	if (not expr.empty()) {
		delay = expr.back().delay;
	}
	return Cost(complexity, delay);
}

vector<Expression::Match> Expression::search(const Expression &rules, size_t count, bool fwd, bool bwd) {
	using Leaf = pair<Operand, Operand>;
	vector<pair<vector<Leaf>, Match> > stack;

	// TODO(edward.bingham) I need a way to canonicalize expressions and hash
	// them so that I can do the state search algorithm.

	// TODO(edward.bingham) rules aren't currently able to match with a variable
	// number of operands. I need to create a comprehension functionality to
	// support those more complex matches.

	if (rules.operations.empty() or rules.operations.back().func != Operation::ARRAY) {
		printf("error: no rules rules found\n");
	}
	auto rulesBegin = rules.operations.back().operands.begin();
	auto rulesEnd = rules.operations.back().operands.end();
	for (auto i = rulesBegin; i != rulesEnd; i++) {
		// ==, <, >
		if (not i->isExpr()
			or (rules.operations[i->index].func != Operation::EQUAL
				and rules.operations[i->index].func != Operation::LESS
				and rules.operations[i->index].func != Operation::GREATER)) {
			printf("error: invalid format for rules rule\n");
		}
	}

	// initialize the initial matches
	for (int i = 0; i < (int)operations.size(); i++) {
		// search through the "rules" rules and add all of the matching starts
		for (auto j = rulesBegin; j != rulesEnd; j++) {
			if (not j->isExpr()) {
				continue;
			}
			auto rule = rules.operations.begin() + j->index;
			// ==, <, >
			if (rule->operands.size() != 2u
				or (rule->func != Operation::EQUAL
					and rule->func != Operation::LESS
					and rule->func != Operation::GREATER)) {
				continue;
			}
			auto lhs = rule->operands.begin();
			auto rhs = std::next(lhs);
			// map left to right
			if (rule->func == Operation::GREATER or (fwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap(Operand::exprOf(i), *lhs, *this, rules, true, &match.vars)) {
					match.replace = *rhs;
					leaves.push_back({Operand::exprOf(i), *lhs});
					stack.push_back({leaves, match});
				}
			}

			// map right to left
			if (rule->func == Operation::LESS or (bwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap(Operand::exprOf(i), *rhs, *this, rules, true, &match.vars)) {
					match.replace = *lhs;
					leaves.push_back({Operand::exprOf(i), *rhs});
					stack.push_back({leaves, match});
				}
			}
		}
	}

	// Find expression matches with depth-first search
	vector<Match> result;
	while (not stack.empty()) {
		vector<Leaf> leaves = stack.back().first;
		Match curr = stack.back().second;
		stack.pop_back();

		Operand from = leaves.back().first;
		Operand to = leaves.back().second;
		if (from.isExpr() and to.isExpr()) {
			curr.expr.push_back(from.index);
		}
		leaves.pop_back();

		if (to.isExpr()) {
			auto fOp = operations.begin() + from.index;
			auto tOp = rules.operations.begin() + to.index;

			bool commute = tOp->isCommutative();
			CombinationIterator it((int)fOp->operands.size(), (int)tOp->operands.size());
			do {
				Match nextMatch = curr;
				vector<Leaf> nextLeaves = leaves;
				vector<size_t> operands;
				bool found = true;
				for (auto i = it.begin(); i != it.end() and found; i++) {
					nextLeaves.push_back({fOp->operands[*i], tOp->operands[i-it.begin()]});
					operands.push_back(*i);
					found = canMap(fOp->operands[*i], tOp->operands[i-it.begin()], *this, rules, false, &nextMatch.vars);
				}

				if (found) {
					if (nextMatch.top.empty()) {
						sort(operands.begin(), operands.end());
						nextMatch.top = operands;
					}
					stack.push_back({nextLeaves, nextMatch});
				}
			} while (commute ? it.nextPerm() : it.nextShift());
		} else if (leaves.empty()) {
			sort(curr.expr.begin(), curr.expr.end());
			result.push_back(curr);
			if (count != 0 and result.size() >= count) {
				return result;
			}
		} else {
			stack.push_back({leaves, curr});
		}
	}
	return result;
}

size_t Expression::count(Operand start) const {
	if (not start.isExpr()) {
		return 1u;
	}
	size_t result = 0u;
	vector<size_t> stack(1, start.index);
	while (not stack.empty()) {
		size_t curr = stack.back();
		stack.pop_back();

		++result;
		for (auto op = operations[curr].operands.begin(); op != operations[curr].operands.end(); op++) {
			if (op->isExpr()) {
				stack.push_back(op->index);
			}
		}
	}
	return result;
}

void Expression::replace(Operand from, Operand to) {
	for (int i = operations.size()-1; i >= 0; i--) {
		for (int j = (int)operations[i].operands.size()-1; j >= 0; j--) {
			if (areSame(operations[i].operands[j], from)) {
				operations[i].operands[j] = to;
			}
		}
	}
}

void Expression::replace(const Expression &rules, Match match) {
	if (not match.replace.isExpr()) {
		size_t ins = 0;
		size_t slot = match.expr.back();
		match.expr.pop_back();

		if (match.top.empty()) {
			operations[slot].operands.clear();
			operations[slot].func = -1;
		} else {
			// If this match doesn't cover all operands, we only want to replace
			// the ones that are covered.
			for (int i = (int)match.top.size()-1; i >= 0; i--) {
				operations[slot].operands.erase(operations[slot].operands.begin() + match.top[i]);
			}
			ins = match.top[0];
		}

		if (match.replace.isVar()) {
			auto v = match.vars.find(match.replace.index);
			if (v != match.vars.end()) {
				operations[slot].operands.insert(
					operations[slot].operands.begin()+ins,
					v->second);
				++ins;
			} else {
				printf("variable not mapped\n");
			}
		} else {
			operations[slot].operands.insert(
				operations[slot].operands.begin()+ins,
				match.replace);
			++ins;
		}

		match.top.clear();
	} else {
		size_t top = operations.size()-1;
		if (not match.expr.empty()) {
			top = match.expr.back();
		}

		size_t num = rules.count(match.replace);
		if (num > match.expr.size()) {
			num -= match.expr.size();
			insert(top, num);
			for (size_t i = 0; i < num; i++) {
				match.expr.push_back(i+top+1);
			}
			top = match.expr.back();
		}

		vector<size_t> stack(1, match.replace.index);
		map<size_t, size_t> exprMap;
		while (not stack.empty()) {
			size_t curr = stack.back();
			stack.pop_back();

			auto pos = exprMap.insert({curr, match.expr.back()});
			if (pos.second) {
				match.expr.pop_back();
			}
			size_t slot = pos.first->second;

			operations[slot].func = rules.operations[curr].func;
			size_t ins = 0;
			if (match.top.empty()) {
				operations[slot].operands.clear();
			} else {
				// If this match doesn't cover all operands, we only want to replace
				// the ones that are covered.
				for (int i = (int)match.top.size()-1; i >= 0; i--) {
					operations[slot].operands.erase(operations[slot].operands.begin() + match.top[i]);
				}
				ins = match.top[0];
			}
			for (auto op = rules.operations[curr].operands.begin(); op != rules.operations[curr].operands.end(); op++) {
				if (op->isExpr()) {
					auto o = exprMap.insert({op->index, match.expr.back()});
					if (o.second) {
						match.expr.pop_back();
					}
					operations[slot].operands.insert(
						operations[slot].operands.begin()+ins,
						Operand::exprOf(o.first->second));
					++ins;
					stack.push_back(op->index);
				} else if (op->isVar()) {
					auto v = match.vars.find(op->index);
					if (v != match.vars.end()) {
						operations[slot].operands.insert(
							operations[slot].operands.begin()+ins,
							v->second);
						++ins;
					} else {
						printf("variable not mapped\n");
					}
				} else {
					operations[slot].operands.insert(
						operations[slot].operands.begin()+ins,
						*op);
					++ins;
				}
			}
			match.top.clear();
		}
	}

	if (not match.expr.empty()) {
		erase(match.expr);
	}
}

Expression &Expression::minimize(Expression directed) {
	static const Expression rules = rewriteBasic();
	if (directed.operations.empty()) {
		directed = rules;
	}

	canonicalize();
	vector<Match> tokens = search(directed, 1u);
	while (not tokens.empty()) {
		replace(directed, tokens.back());
		canonicalize();
		tokens = search(directed, 1u);
	}

	// TODO(edward.bingham) Then I need to implement encoding
	// Use the unidirectional expression rewrite system?
	// propagate validity?
	
	return *this;
}

Expression espresso(Expression expr, vector<Type> vars, Expression directed, Expression undirected) {
	static const Expression rules = rewriteUndirected();
	if (undirected.operations.empty()) {
		undirected = rules;
	}

	// TODO(edward.bingham) Implement the bidirectional search function to search
	// rule applications that minimize the cost function but then apply all of
	// the unidirectional rules in between

	// TODO(edward.bingham) Create a set of unidirectional rewrite rules to
	// change bitwise operators into arithmetic operators, and then a set of
	// unidirectional rules to switch them back

	// TODO(edward.bingham) Tie this all together into an easy-to-use
	// minimization system.

	Expression result;
	Cost best(1e20, 1e20);
	expr.minimize(directed);

	vector<pair<Cost, Expression> > stack;
	stack.push_back({expr.cost(vars), expr});
	while (not stack.empty()) {
		pair<Cost, Expression> curr = stack.back();
		stack.pop_back();

		vector<Expression::Match> opts = curr.second.search(undirected);
		for (auto i = opts.begin(); i != opts.end(); i++) {
			Expression next = curr.second;
			next.replace(undirected, *i);
			Cost cost = next.cost(vars);
			stack.push_back({cost, next});
		}
	}

	return result;
}

Expression &Expression::operator=(Operand e)
{
	set(Operation::IDENTITY, e);
	return *this;
}

string Expression::to_string() {
	if (operations.empty()) {
		return "(gnd)\n";
	}

	stringstream os;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		os << i << ": " << operations[i] << endl;
	}
	return os.str();
}

ostream &operator<<(ostream &os, Expression e) {
	os << e.to_string();
	return os;
}

Expression operator!(Expression e)
{
	Expression result;
	result.set(Operation::BITWISE_NOT, e);
	return result;
}

Expression operator-(Expression e)
{
	Expression result;
	result.set(Operation::NEGATION, e);
	return result;
}

Expression is_valid(Expression e)
{
	/*if (e.is_valid()) {
		return Operand(true);
	} else if (e.is_neutral()) {
		return Operand(false);
	} else if (e.is_wire()) {
		return e;
	}*/
	Expression result;
	result.set(Operation::VALIDITY, e);
	return result;
}

Expression operator~(Expression e)
{
	/*if (e.is_valid()) {
		return Operand(false);
	} else if (e.is_neutral()) {
		return Operand(true);
	}*/
	Expression result;
	result.set(Operation::BOOLEAN_NOT, e);
	return result;
}

Expression operator||(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_NOT, e0, e1);
	return result;
}

Expression operator&&(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator^(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression operator==(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&(Expression e0, Expression e1)
{
	/*if (e0.is_null() or e1.is_null()) {
		return Expression(value::X());
	} else if (e0.is_neutral() or e1.is_neutral()) {
		return Operand(false);
	} else if (e0.is_valid()) {
		return is_valid(e1);
	} else if (e1.is_valid()) {
		return is_valid(e0);
	}*/
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator|(Expression e0, Expression e1)
{
	/*if (e0.is_null() or e1.is_null()) {
		return Expression(value::X());
	} if (e0.is_valid() or e1.is_valid()) {
		return Operand(true);
	} else if (e0.is_neutral()) {
		return is_valid(e1);
	} else if (e1.is_neutral()) {
		return is_valid(e0);
	}*/
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator|(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression operator==(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression operator|(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression operator==(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression operator|(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression operator==(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression array(vector<Expression> e) {
	Expression result;
	result.set(Operation::ARRAY, e);
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
