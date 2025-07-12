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
int Operation::NEGATIVE = -1;
int Operation::VALIDITY = -1;
int Operation::BOOLEAN_NOT = -1;
int Operation::INVERSE = -1;
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
int Operation::CALL = -1;
int Operation::MEMBER = -1;

Operand::Operand(Value v) {
	Operation::loadOperators();
	type = CONST;
	cnst = v;
}

Operand::Operand(bool bval) {
	Operation::loadOperators();
	type = CONST;
	cnst = Value::boolOf(bval);
}

Operand::Operand(int64_t ival) {
	Operation::loadOperators();
	type = CONST;
	cnst = Value::intOf(ival);
}

Operand::Operand(int ival) {
	Operation::loadOperators();
	type = CONST;
	cnst = Value::intOf(ival);
}

Operand::Operand(double rval) {
	Operation::loadOperators();
	type = CONST;
	cnst = Value::realOf(rval);
}

Operand::Operand(string sval) {
	Operation::loadOperators();
	type = CONST;
	cnst = Value::stringOf(sval);
}

Operand::~Operand() {
}

bool Operand::isUndef() const {
	return type < 0;
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

bool Operand::isType() const {
	return type == TYPE;
}

Value Operand::get(State values, vector<Value> expressions) const {
	switch (type)
	{
	case CONST:
		return cnst;
	case VAR:
		if (index < values.size()) {
			return values[index];
		} else {
			printf("error: variable not defined %d/%d\n", (int)index, (int)values.size());
		}
		return Value::X();
	case EXPR:
		if (index < expressions.size()) {
			return expressions[index];
		} else {
			printf("error: expression not defined %d/%d\n", (int)index, (int)expressions.size());
		}
		return Value::X();
	default:
	return Value::X();
	}
}

void Operand::set(State &values, vector<Value> &expressions, Value v) const {
	if (isExpr()) {
		if (index < expressions.size()) {
			expressions[index] = v;
		} else {
			printf("error: Expression index '%lu' out of bounds '%lu'\n", index, expressions.size());
		}
	} else if (isVar()) {
		values.set(index, v);
	} else if (isConst()) {
		printf("error: const not writeable\n");
	}
}

Operand Operand::undef() {
	Operand result;
	result.type = UNDEF;
	result.index = 0;
	return result;
}

Operand Operand::X() {
	return Operand(Value::X());
}

Operand Operand::U() {
	return Operand(Value::U());
}

Operand Operand::boolOf(bool bval) {
	return Operand(Value::boolOf(bval));
}

Operand Operand::intOf(int64_t ival) {
	return Operand(Value::intOf(ival));
}

Operand Operand::realOf(double rval) {
	return Operand(Value::realOf(rval));
}

Operand Operand::arrOf(vector<Value> arr) {
	return Operand(Value::arrOf(arr));
}

Operand Operand::structOf(vector<Value> arr) {
	return Operand(Value::structOf(arr));
}

Operand Operand::stringOf(string sval) {
	return Operand(Value::stringOf(sval));
}

Operand Operand::exprOf(size_t index) {
	Operand result;
	result.type = EXPR;
	result.index = index;
	return result;
}

Operand &Operand::offsetExpr(int off) {
	if (isExpr()) {
		index += off;
	}
	return *this;
}

Operand Operand::varOf(size_t index) {
	Operand result;
	result.type = VAR;
	result.index = index;
	return result;
}

void Operand::apply(vector<int> varMap) {
	if (varMap.empty()) {
		return;
	}

	if (isVar()) {
		if (index < varMap.size()) {
			index = varMap[index];
		} else {
			type = CONST;
			cnst = Value::X();
		}
	}
}

void Operand::apply(vector<Operand> varMap) {
	if (varMap.empty()) {
		return;
	}

	if (isVar()) {
		if (index < varMap.size()) {
			type = varMap[index].type;
			index = varMap[index].index;
		} else {
			type = CONST;
			cnst = Value::X();
		}
	}
}

void Operand::replace(vector<Operand> exprMap) {
	if (exprMap.empty()) {
		return;
	}

	if (isExpr() and index < exprMap.size() and not exprMap[index].isUndef()) {
		*this = exprMap[index];
	}
}

Operand Operand::typeOf(int type) {
	Operand result;
	result.type = TYPE;
	result.index = type;
	return result;
}

ostream &operator<<(ostream &os, Operand o) {
	if (o.isConst()) {
		os << o.cnst;
	} else if (o.isVar()) {
		os << "v" << o.index;
	} else if (o.isExpr()) {
		os << "e" << o.index;
	} else if (o.isType()) {
		os << "f" << o.index;
	} else {
		os << "undef";
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

Operator::Operator(string prefix, string trigger, string infix, string postfix, bool commutative, bool reflexive) {
	this->prefix = prefix;
	this->trigger = trigger;
	this->infix = infix;
	this->postfix = postfix;
	this->commutative = commutative;
	this->reflexive = reflexive;
}

Operator::~Operator() {
}

Operation::Operation() {
	exprIndex = std::numeric_limits<size_t>::max();
	func = -1;
}

Operation::Operation(int func, vector<Operand> args, size_t exprIndex) {
	this->exprIndex = exprIndex;
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
		//printf("loading operators\n");

		BITWISE_NOT   = push(Operator("!", "", "", ""));
		IDENTITY      = push(Operator("+", "", "", "", false, true));
		NEGATION      = push(Operator("-", "", "", ""));
		NEGATIVE      = push(Operator("ltz(", "", "", ")"));
		VALIDITY      = push(Operator("val(", "", "", ")"));
		BOOLEAN_NOT   = push(Operator("~", "", "", ""));
		INVERSE       = push(Operator("inv(", "", "", ")"));
		BITWISE_OR    = push(Operator("", "", "||", "", true));
		BITWISE_AND   = push(Operator("", "", "&&", "", true));
		BITWISE_XOR   = push(Operator("", "", "^^", "", true));
		EQUAL         = push(Operator("", "", "==", ""));
		NOT_EQUAL     = push(Operator("", "", "~=", ""));
		LESS          = push(Operator("", "", "<", ""));
		GREATER       = push(Operator("", "", ">", ""));
		LESS_EQUAL    = push(Operator("", "", "<=", ""));
		GREATER_EQUAL = push(Operator("", "", ">=", ""));
		SHIFT_LEFT    = push(Operator("", "", "<<", ""));
		SHIFT_RIGHT   = push(Operator("", "", ">>", ""));
		ADD           = push(Operator("", "", "+", "", true));
		SUBTRACT      = push(Operator("", "", "-", ""));
		MULTIPLY      = push(Operator("", "", "*", "", true));
		DIVIDE        = push(Operator("", "", "/", ""));
		MOD           = push(Operator("", "", "%", ""));
		TERNARY       = push(Operator("", "?", ":", ""));
		BOOLEAN_OR    = push(Operator("", "", "|", "", true));
		BOOLEAN_AND   = push(Operator("", "", "&", "", true));
		BOOLEAN_XOR   = push(Operator("", "", "^", "", true));
		ARRAY         = push(Operator("[", "", ",", "]"));
		INDEX         = push(Operator("", "[", ":", "]"));
		CALL          = push(Operator("", "(", ",", ")"));
		MEMBER        = push(Operator("", ".", "", ""));

		//printf("loaded %d operators\n", (int)Operation::operators.size());
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
	} else if (func == Operation::NEGATIVE) {
		result = Type(0.0, 0.0, 0.0);
		cost = 0.0;
		return {result, cost};
	} else if (func == Operation::INVERSE) { // 1/x
		// TODO(edward.bingham) I'm really not sure about this
		result = Type(1.0/args[0].coeff, args[0].width, args[0].delay + args[0].width);
		cost = args[0].width*args[0].width;
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

Value Operation::evaluate(int func, vector<Value> args) {
	if (func == Operation::BITWISE_NOT) {
		return !args[0];
	} else if (func == Operation::IDENTITY) {
		return args[0];
	} else if (func == Operation::NEGATION) {
		return -args[0];
	} else if (func == Operation::NEGATIVE) {
		return args[0] < Value(0);
	} else if (func == Operation::VALIDITY) {
		return valid(args[0]);
	} else if (func == Operation::BOOLEAN_NOT) {
		return ~args[0];
	} else if (func == Operation::INVERSE) {
		return inv(args[0]);
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
			args.push_back(Value::X());
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
		return Value::arrOf(args);
	} else if (func == Operation::INDEX) { // index
		return index(args[0], args[1]);
	}
	printf("internal: function %d not implemented\n", func);
	return args[0];
}

Value Operation::evaluate(int func, vector<Value> args, TypeSet types) {
	if (func == Operation::MEMBER) {
		return member(args[0], args[1], types);
	}
	return evaluate(func, args);
}

Value Operation::evaluate(State values, vector<Value> expressions) const {
	vector<Value> args;
	args.reserve(operands.size());
	for (int i = 0; i < (int)operands.size(); i++) {
		args.push_back(operands[i].get(values, expressions));
	}

	return Operation::evaluate(func, args);
}

Value Operation::evaluate(State values, vector<Value> expressions, TypeSet types) const {
	vector<Value> args;
	args.reserve(operands.size());
	for (int i = 0; i < (int)operands.size(); i++) {
		args.push_back(operands[i].get(values, expressions));
	}

	return Operation::evaluate(func, args, types);
}

void Operation::propagate(State &result, const State &global, vector<Value> &expressions, const vector<Value> gexpressions, Value v) const
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
			Value v0 = operands[0].get(global, gexpressions);
			if (v0.isValid()) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, Value::U());
			}
			Value v1 = operands[1].get(global, expressions);
			if (v1.isValid()) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, Value::U());
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
			Value v0 = operands[0].get(global, gexpressions);
			if (v0.isNeutral()) {
				operands[0].set(result, expressions, v0);
			} else {
				operands[0].set(result, expressions, Value::U());
			}
			Value v1 = operands[1].get(global, expressions);
			if (v1.isNeutral()) {
				operands[0].set(result, expressions, v1);
			} else {
				operands[0].set(result, expressions, Value::U());
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
			operands[0].set(result, expressions, Value::U());
		} else if (func >= 5 and func <= 23) {
			// a||b, a&&b, a^b, a<<b, a>>b
			// a+b, a-b, a*b, a/b, a%b
			// a==b, a!=b, a<b, a>b, a<=b, a>=b
			// a&b, a|b
			operands[0].set(result, expressions, Value::U());
			operands[1].set(result, expressions, Value::U());
		} else {
			// a
			operands[0].set(result, expressions, Value::U());
		}
	} 
}

void Operation::apply(vector<int> varMap) {
	if (varMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].apply(varMap);
	}
}

void Operation::apply(vector<Operand> varMap) {
	if (varMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].apply(varMap);
	}
}

void Operation::replace(vector<Operand> exprMap) {
	if (exprMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].replace(exprMap);
	}
}

Operation Operation::extract(vector<size_t> idx, size_t exprIndex) {
	Operation result(func, {}, exprIndex);
	for (int i = (int)idx.size()-1; i >= 0; i--) {
		result.operands.push_back(operands[idx[i]]);
		if (i != 0) {
			operands.erase(operands.begin()+idx[i]);
		}
	}
	operands[idx[0]] = Operand::exprOf(exprIndex);
	return result;
}

Operation &Operation::offsetExpr(int off) {
	exprIndex += off;
	for (int i = 0; i < (int)operands.size(); i++) {
		operands[i].offsetExpr(off);
	}
	return *this;
}

void Operation::tidy() {
	if (isCommutative()) {
		// Move constants to the left hand side for evaluation
		// Order variables by index
		sort(operands.begin(), operands.end(),
			[](const Operand &a, const Operand &b) {
				return a.type < b.type or (a.type == b.type
					and a.isVar() and a.index < b.index);
			});
	}

	// merge adjacent constants from left to right as a result of operator precedence
	auto i = operands.begin();
	while (i != operands.end() and next(i) != operands.end()) {
		auto j = next(i);
		if (i->isConst() and j->isConst()) {
			*i = evaluate(func, {i->get(), j->get()});
			operands.erase(j);
		} else {
			i++;
		}
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
	os << "e" << o.exprIndex << " = ";
	Operator op;
	if (o.func >= 0 and o.func < (int)Operation::operators.size()) {
		op = Operation::operators[o.func];
	} else {
		op = Operation::operators[Operation::IDENTITY];
		printf("error: unrecognized operator\n");
	}

	os << op.prefix;
	if (not o.operands.empty()) {
		os << o.operands[0];
	}

	if (not op.trigger.empty()) {
		os << op.trigger;
	} else if (o.operands.size() > 1u) {
		os << op.infix;
	}

	for (int i = 1; i < (int)o.operands.size(); i++) {
		if (i != 1) {
			os << op.infix;
		}
		os << o.operands[i];
	}
	os << op.postfix;
	os << "  (" << o.func << ")";
	return os;
}

}
