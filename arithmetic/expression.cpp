#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>

namespace arithmetic {

Expression::Expression() {
	Operation::loadOperators();
	top = Operand::undef();
}

Expression::Expression(int func, vector<Operand> args) {
	Operation::loadOperators();
	top = pushExpr(Operation(func, args));
}

Expression::~Expression() {
}

vector<Operand> Expression::exprIndex() const {
	vector<Operand> result;
	result.reserve(operations.size());
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			result.push_back(i->op());
		}
	}
	return result;
}

const Operation *Expression::getExpr(size_t index) const {
	if (index < operations.size() and not operations[index].isUndef()) {
		return &operations[index];
	}
	return nullptr;
}

bool Expression::setExpr(Operation o) {
	if (o.exprIndex >= operations.size()) {
		operations.resize(o.exprIndex+1);
	}
	operations[o.exprIndex] = o;
	return true;
}

Operand Expression::pushExpr(Operation o) {
	if (next.empty()) {
		o.exprIndex = operations.size();
		operations.push_back(o);
	} else {
		o.exprIndex = next.back();
		next.pop_back();
		operations[o.exprIndex] = o;
	}
	return o.op();
}

bool Expression::eraseExpr(size_t index) {
	if (index < operations.size() and not operations[index].isUndef()) {
		operations[index] = Operation::undef();
		next.push_back(index);
		return true;
	}
	return false;
}

void Expression::clear() {
	top = Operand::undef();
	operations.clear();
	next.clear();
}

size_t Expression::size() const {
	return operations.size() - next.size();
}

Operand Expression::append(Expression arg) {
	Mapping m;
	for (auto i = arg.operations.begin(); i != arg.operations.end(); i++) {
		if (not i->isUndef()) {
			m.set(i->op(), pushExpr(Operation(*i).apply(m)));
		}
	}
	return m.map(arg.top);
}

vector<Operand> Expression::append(vector<Expression> arg) {
	vector<Operand> result;
	for (auto i = arg.begin(); i != arg.end(); i++) {
		result.push_back(append(*i));
	}
	return result;
}

Expression &Expression::push(int func, vector<Operand> args) {
	// add to operations list if doesn't exist
	Operation arg(func, args);
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef() and *i == arg) {
			top = i->op();
			return *this;
		}
	}
	top = pushExpr(arg);
	return *this;
}

bool Expression::isNull() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
				if (j->isVar() or (j->isConst() and not j->cnst.isUnstable())) {
					return false;
				}
			}
		}
	}
	return true;
}

bool Expression::isConstant() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
				if (j->isVar() or (j->isConst() and j->cnst.isUnstable())) {
					return false;
				}
			}
		}
	}
	return true;
}

bool Expression::isValid() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
				if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isNeutral()))) {
					return false;
				}
			}
		}
	}
	return true;
}

bool Expression::isNeutral() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
				if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isValid()))) {
					return false;
				}
			}
		}
	}
	return true;
}

bool Expression::isWire() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (i->func == Operation::VALIDITY or i->func == Operation::BOOLEAN_NOT or i->func == Operation::BOOLEAN_AND or i->func == Operation::BOOLEAN_OR) {
			return true;
		}
		if (not i->isUndef()) {
			for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
				if (j->isConst() and (j->cnst.isNeutral() or j->cnst.isValid())) {
					return true;
				}
			}
		}
	}
	return false;
}

Expression &Expression::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return *this;
	}

	for (auto i = operations.begin(); i != operations.end(); i++) {
		if (not i->isUndef()) {
			i->apply(uidMap);
		}
	}
	return *this;
}

bool areSame(Expression e0, Expression e1) {
	if (e0.top != e1.top) {
		return false;
	}
	auto i = ConstDownIterator(e0, {e0.top}), j = ConstDownIterator(e1, {e1.top});
	for (; not i.done() and not j.done(); ++i, ++j) {
		if (*i != *j) {
			return false;
		}
	}
	return i.done() and j.done();
}

Expression &Expression::operator=(Operand e) {
	clear();
	top = e;
	return *this;
}

Expression Expression::undef() {
	Expression result;
	result.top = Operand::undef();
	return result;
}

Expression Expression::X() {
	Expression result;
	result.top = Operand::X();
	return result;
}

Expression Expression::U() {
	Expression result;
	result.top = Operand::U();
	return result;
}

Expression Expression::boolOf(bool bval) {
	Expression result;
	result.top = Operand::boolOf(bval);
	return result;
}

Expression Expression::intOf(int64_t ival) {
	Expression result;
	result.top = Operand::intOf(ival);
	return result;
}

Expression Expression::realOf(double rval) {
	Expression result;
	result.top = Operand::realOf(rval);
	return result;
}

Expression Expression::arrOf(vector<Value> arr) {
	Expression result;
	result.top = Operand::arrOf(arr);
	return result;
}

Expression Expression::structOf(vector<Value> arr) {
	Expression result;
	result.top = Operand::structOf(arr);
	return result;
}

Expression Expression::stringOf(string sval) {
	Expression result;
	result.top = Operand::stringOf(sval);
	return result;
}

Expression Expression::varOf(size_t index) {
	Expression result;
	result.top = Operand::varOf(index);
	return result;
}

Expression Expression::typeOf(int type) {
	Expression result;
	result.top = Operand::typeOf(type);
	return result;
}

string Expression::to_string() const {
	ostringstream oss;
	oss << "top: " << top << "/" << operations.size() << endl;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (not operations[i].isUndef()) {
			oss << i << ": " << operations[i] << endl;
		}
	}
	return oss.str();
}

ostream &operator<<(ostream &os, Expression e) {
	os << e.to_string();
	return os;
}

Expression operator~(Expression e)  { return e.push(Operation::BOOLEAN_NOT, {e.top}); }
Expression operator-(Expression e)  { return e.push(Operation::NEGATION,    {e.top}); }
Expression ident(Expression e)      { return e.push(Operation::IDENTITY,    {e.top}); }
Expression isValid(Expression e)    { return e.push(Operation::VALIDITY,    {e.top}); }
Expression isNegative(Expression e) { return e.push(Operation::NEGATIVE,    {e.top}); }
Expression operator!(Expression e)  { return e.push(Operation::BITWISE_NOT, {e.top}); }
Expression inv(Expression e)        { return e.push(Operation::INVERSE,     {e.top}); }
Expression operator||(Expression e0, Expression e1) { return e0.push(Operation::BITWISE_OR,   {e0.top, e0.append(e1)}); }
Expression operator&&(Expression e0, Expression e1) { return e0.push(Operation::BITWISE_AND,   {e0.top, e0.append(e1)}); }
Expression operator^ (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_XOR,   {e0.top, e0.append(e1)}); }
Expression bitwiseXor(Expression e0, Expression e1) { return e0.push(Operation::BITWISE_XOR,   {e0.top, e0.append(e1)}); }
Expression operator==(Expression e0, Expression e1) { return e0.push(Operation::EQUAL,         {e0.top, e0.append(e1)}); }
Expression operator!=(Expression e0, Expression e1) { return e0.push(Operation::NOT_EQUAL,     {e0.top, e0.append(e1)}); }
Expression operator< (Expression e0, Expression e1) { return e0.push(Operation::LESS,          {e0.top, e0.append(e1)}); }
Expression operator> (Expression e0, Expression e1) { return e0.push(Operation::GREATER,       {e0.top, e0.append(e1)}); }
Expression operator<=(Expression e0, Expression e1) { return e0.push(Operation::LESS_EQUAL,    {e0.top, e0.append(e1)}); }
Expression operator>=(Expression e0, Expression e1) { return e0.push(Operation::GREATER_EQUAL, {e0.top, e0.append(e1)}); }
Expression operator<<(Expression e0, Expression e1) { return e0.push(Operation::SHIFT_LEFT,    {e0.top, e0.append(e1)}); }
Expression operator>>(Expression e0, Expression e1) { return e0.push(Operation::SHIFT_RIGHT,   {e0.top, e0.append(e1)}); }
Expression operator+ (Expression e0, Expression e1) { return e0.push(Operation::ADD,           {e0.top, e0.append(e1)}); }
Expression operator- (Expression e0, Expression e1) { return e0.push(Operation::SUBTRACT,      {e0.top, e0.append(e1)}); }
Expression operator* (Expression e0, Expression e1) { return e0.push(Operation::MULTIPLY,      {e0.top, e0.append(e1)}); }
Expression operator/ (Expression e0, Expression e1) { return e0.push(Operation::DIVIDE,        {e0.top, e0.append(e1)}); }
Expression operator% (Expression e0, Expression e1) { return e0.push(Operation::MOD,           {e0.top, e0.append(e1)}); }
Expression operator& (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_AND,   {e0.top, e0.append(e1)}); }
Expression operator| (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_OR,    {e0.top, e0.append(e1)}); }
Expression operator| (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_OR,    {e0.top, e1}); }
Expression operator& (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_AND,   {e0.top, e1}); }
Expression operator^ (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_XOR,   {e0.top, e1}); }
Expression bitwiseXor(Expression e0, Operand e1) { return e0.push(Operation::BITWISE_XOR,   {e0.top, e1}); }
Expression operator==(Expression e0, Operand e1) { return e0.push(Operation::EQUAL,         {e0.top, e1}); }
Expression operator!=(Expression e0, Operand e1) { return e0.push(Operation::NOT_EQUAL,     {e0.top, e1}); }
Expression operator<(Expression e0, Operand e1)  { return e0.push(Operation::LESS,          {e0.top, e1}); }
Expression operator>(Expression e0, Operand e1)  { return e0.push(Operation::GREATER,       {e0.top, e1}); }
Expression operator<=(Expression e0, Operand e1) { return e0.push(Operation::LESS_EQUAL,    {e0.top, e1}); }
Expression operator>=(Expression e0, Operand e1) { return e0.push(Operation::GREATER_EQUAL, {e0.top, e1}); }
Expression operator<<(Expression e0, Operand e1) { return e0.push(Operation::SHIFT_LEFT,    {e0.top, e1}); }
Expression operator>>(Expression e0, Operand e1) { return e0.push(Operation::SHIFT_RIGHT,   {e0.top, e1}); }
Expression operator+ (Expression e0, Operand e1) { return e0.push(Operation::ADD,           {e0.top, e1}); }
Expression operator- (Expression e0, Operand e1) { return e0.push(Operation::SUBTRACT,      {e0.top, e1}); }
Expression operator* (Expression e0, Operand e1) { return e0.push(Operation::MULTIPLY,      {e0.top, e1}); }
Expression operator/ (Expression e0, Operand e1) { return e0.push(Operation::DIVIDE,        {e0.top, e1}); }
Expression operator% (Expression e0, Operand e1) { return e0.push(Operation::MOD,           {e0.top, e1}); }
Expression operator&&(Expression e0, Operand e1) { return e0.push(Operation::BITWISE_AND,   {e0.top, e1}); }
Expression operator||(Expression e0, Operand e1) { return e0.push(Operation::BITWISE_OR,    {e0.top, e1}); }
Expression operator| (Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_OR,    {e0, e1.top}); }
Expression operator& (Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_AND,   {e0, e1.top}); }
Expression operator^ (Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_XOR,   {e0, e1.top}); }
Expression bitwiseXor(Operand e0, Expression e1) { return e1.push(Operation::BITWISE_XOR,   {e0, e1.top}); }
Expression operator==(Operand e0, Expression e1) { return e1.push(Operation::EQUAL,         {e0, e1.top}); }
Expression operator!=(Operand e0, Expression e1) { return e1.push(Operation::NOT_EQUAL,     {e0, e1.top}); }
Expression operator< (Operand e0, Expression e1) { return e1.push(Operation::LESS,          {e0, e1.top}); }
Expression operator> (Operand e0, Expression e1) { return e1.push(Operation::GREATER,       {e0, e1.top}); }
Expression operator<=(Operand e0, Expression e1) { return e1.push(Operation::LESS_EQUAL,    {e0, e1.top}); }
Expression operator>=(Operand e0, Expression e1) { return e1.push(Operation::GREATER_EQUAL, {e0, e1.top}); }
Expression operator<<(Operand e0, Expression e1) { return e1.push(Operation::SHIFT_LEFT,    {e0, e1.top}); }
Expression operator>>(Operand e0, Expression e1) { return e1.push(Operation::SHIFT_RIGHT,   {e0, e1.top}); }
Expression operator+ (Operand e0, Expression e1) { return e1.push(Operation::ADD,           {e0, e1.top}); }
Expression operator- (Operand e0, Expression e1) { return e1.push(Operation::SUBTRACT,      {e0, e1.top}); }
Expression operator* (Operand e0, Expression e1) { return e1.push(Operation::MULTIPLY,      {e0, e1.top}); }
Expression operator/ (Operand e0, Expression e1) { return e1.push(Operation::DIVIDE,        {e0, e1.top}); }
Expression operator% (Operand e0, Expression e1) { return e1.push(Operation::MOD,           {e0, e1.top}); }
Expression operator&&(Operand e0, Expression e1) { return e1.push(Operation::BITWISE_AND,   {e0, e1.top}); }
Expression operator||(Operand e0, Expression e1) { return e1.push(Operation::BITWISE_OR,    {e0, e1.top}); }

Expression booleanOr(Expression e0)  { return e0.push(Operation::BOOLEAN_OR,  {e0.top}); }
Expression booleanAnd(Expression e0) { return e0.push(Operation::BOOLEAN_AND, {e0.top}); }
Expression booleanXor(Expression e0) { return e0.push(Operation::BOOLEAN_XOR, {e0.top}); }
Expression bitwiseOr(Expression e0)  { return e0.push(Operation::BITWISE_OR,  {e0.top}); }
Expression bitwiseAnd(Expression e0) { return e0.push(Operation::BITWISE_AND, {e0.top}); }
Expression bitwiseXor(Expression e0) { return e0.push(Operation::BITWISE_XOR, {e0.top}); }
Expression add(Expression e0)        { return e0.push(Operation::ADD,         {e0.top}); }
Expression mult(Expression e0)       { return e0.push(Operation::MULTIPLY,    {e0.top}); }

Expression array(vector<Expression> e0)      { Expression e; return e.push(Operation::ARRAY,       e.append(e0)); }
Expression booleanOr(vector<Expression> e0)  { Expression e; return e.push(Operation::BOOLEAN_OR,  e.append(e0)); }
Expression booleanAnd(vector<Expression> e0) { Expression e; return e.push(Operation::BOOLEAN_AND, e.append(e0)); }
Expression booleanXor(vector<Expression> e0) { Expression e; return e.push(Operation::BOOLEAN_XOR, e.append(e0)); }
Expression bitwiseOr(vector<Expression> e0)  { Expression e; return e.push(Operation::BITWISE_OR,  e.append(e0)); }
Expression bitwiseAnd(vector<Expression> e0) { Expression e; return e.push(Operation::BITWISE_AND, e.append(e0)); }
Expression bitwiseXor(vector<Expression> e0) { Expression e; return e.push(Operation::BITWISE_XOR, e.append(e0)); }
Expression add(vector<Expression> e0)        { Expression e; return e.push(Operation::ADD,         e.append(e0)); }
Expression mult(vector<Expression> e0)       { Expression e; return e.push(Operation::MULTIPLY,    e.append(e0)); }

Expression call(int func, vector<Expression> args) {
	args.insert(args.begin(), Expression::typeOf(func));

	Expression result;
	return result.push(Operation::CALL, result.append(args));
}

int passesGuard(const State &encoding, const State &global, const Expression &guard, State *total) {
	vector<Value> expressions;
	vector<Value> gexpressions;

	for (int i = 0; i < (int)guard.operations.size(); i++) {
		Value g = guard.operations[i].evaluate(global, gexpressions);
		Value l = guard.operations[i].evaluate(encoding, expressions);

		if (l.isUnstable() or g.isUnstable()
			or (g.isNeutral() and l.isValid())
			or (g.isValid() and l.isNeutral())
			or (g.isValid() and l.isValid() and not areSame(g, l))) {
			l = Value::X();
		}

		expressions.push_back(l);
		gexpressions.push_back(g);
	}

	if (expressions.back().isUnknown() or expressions.back().isValid()) {
		if (gexpressions.back().isNeutral() or gexpressions.back().isUnknown()) {
			expressions.back() = Value::X();
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

Expression weakestGuard(const Expression &guard, const Expression &exclude) {
	// TODO(edward.bingham) Remove terms from the guard until guard overlaps exclude (using cylidrical algebraic decomposition)
	// 1. put the guard in conjunctive normal form using the boolean operations & | ~
	// 2. for each term in the conjunctive normal form, pick a comparison and eliminate it, then check overlap. 
	return guard;
}

}
