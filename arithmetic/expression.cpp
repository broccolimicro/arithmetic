#include "expression.h"

#include <array>

#include <common/standard.h>
#include <common/text.h>

#include "state.h"
#include "rewrite.h"
#include "algorithm.h"


namespace arithmetic {

Expression::Expression(Operand top) {
	Operation::loadOperators();
	this->top = top;
}

Expression::Expression(int func, vector<Operand> args) {
	Operation::loadOperators();
	top = pushExpr(Operation(func, args));
}

Expression::Expression(int func, vector<Expression> args) {
	Operation::loadOperators();
	top = pushExpr(Operation(func, append(args)));
}

Expression::~Expression() {
}

Expression Expression::undef() {
	Expression result;
	result.top = Operand::undef();
	return result;
}

Expression Expression::X(Value::ValType type) {
	Expression result;
	result.top = Operand::X(type);
	return result;
}

Expression Expression::U(Value::ValType type) {
	Expression result;
	result.top = Operand::U(type);
	return result;
}

Expression Expression::gnd(Value::ValType type) {
	Expression result;
	result.top = Operand::gnd(type);
	return result;
}

Expression Expression::vdd() {
	Expression result;
	result.top = Operand::vdd();
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

Expression Expression::structOf(string name, vector<Value> arr) {
	Expression result;
	result.top = Operand::structOf(name, arr);
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

Expression Expression::typeOf(Operand::Type type) {
	Expression result;
	result.top = Operand::typeOf(type);
	return result;
}

vector<Operand> Expression::exprIndex() const {
	return sub.exprIndex();
}

const Operation *Expression::getExpr(size_t index) const {
	return sub.getExpr(index);
}

bool Expression::setExpr(Operation o) {
	return sub.setExpr(o);
}

Operand Expression::pushExpr(Operation o) {
	return sub.pushExpr(o);
}

bool Expression::eraseExpr(size_t index) {
	return sub.eraseExpr(index);
}

void Expression::clear() {
	sub.clear();
	top = Operand::undef();
}

size_t Expression::size() const {
	return sub.size();
}

Operand Expression::append(Expression arg) {
	return sub.append(arg.sub, {arg.top}).map(arg.top);
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
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		if (*getExpr(i->index) == arg) {
			top = *i;
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
	if (top.isVar() or top.isType() or (top.isConst() and not top.cnst.isUnstable())) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		for (auto j = getExpr(i->index)->operands.begin(); j != getExpr(i->index)->operands.end(); j++) {
			if (j->isVar() or j->isType() or (j->isConst() and not j->cnst.isUnstable())) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::isConstant() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	if (top.isVar() or top.isType() or (top.isConst() and top.cnst.isUnstable())) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		for (auto j = getExpr(i->index)->operands.begin(); j != getExpr(i->index)->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and j->cnst.isUnstable())) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::isValid() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is constant after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	if (top.isVar() or (top.isConst() and (top.cnst.isUnstable() or top.cnst.isNeutral()))) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		for (auto j = getExpr(i->index)->operands.begin(); j != getExpr(i->index)->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isNeutral()))) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::isNeutral() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	if (top.isVar() or (top.isConst() and (top.cnst.isUnstable() or top.cnst.isValid()))) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		for (auto j = getExpr(i->index)->operands.begin(); j != getExpr(i->index)->operands.end(); j++) {
			if (j->isVar() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isValid()))) {
				return false;
			}
		}
	}
	return true;
}

bool Expression::isWire() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	if (top.isConst() and (top.cnst.isNeutral() or top.cnst.isValid())) {
		return true;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		if (getExpr(i->index)->func == Operation::VALIDITY
			or getExpr(i->index)->func == Operation::WIRE_NOT
			or getExpr(i->index)->func == Operation::WIRE_AND
			or getExpr(i->index)->func == Operation::WIRE_OR) {
			return true;
		}
		for (auto j = getExpr(i->index)->operands.begin(); j != getExpr(i->index)->operands.end(); j++) {
			if (j->isConst() and (j->cnst.isNeutral() or j->cnst.isValid())) {
				return true;
			}
		}
	}
	return false;
}

Expression &Expression::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return *this;
	}
	this->top.apply(uidMap);

	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		setExpr(Operation(*getExpr(i->index)).apply(uidMap));
	}
	return *this;
}

Expression Expression::operator()(Expression idx) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, e.append(idx)});
}

Expression Expression::operator()(Expression from, Expression to) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, e.append(from), e.append(to)});
}

Expression Expression::operator()(Operand idx) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, idx});
}

Expression Expression::operator()(Operand from, Operand to) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, from, to});
}

Expression Expression::operator()(Operand from, Expression to) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, from, e.append(to)});
}

Expression Expression::operator()(Expression from, Operand to) const {
	Expression e = *this;
	return e.push(Operation::INDEX, {e.top, e.append(from), to});
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

string Expression::to_string(bool debug) const {
	return arithmetic::to_string(*this, this->top, debug);
}

ostream &operator<<(ostream &os, Expression e) {
	os << e.to_string();
	return os;
}

Expression operator~(Expression e)  { return e.push(Operation::WIRE_NOT,    {e.top}); }
Expression operator-(Expression e)  { return e.push(Operation::NEGATION,    {e.top}); }
Expression ident(Expression e)      { return e.push(Operation::IDENTITY,    {e.top}); }
Expression isValid(Expression e)    { return e.push(Operation::VALIDITY,    {e.top}); }
Expression isTrue(Expression e)     { return e.push(Operation::TRUTHINESS,  {e.top}); }
Expression isNegative(Expression e) { return e.push(Operation::NEGATIVE,    {e.top}); }
Expression operator!(Expression e)  { return e.push(Operation::BOOLEAN_NOT, {e.top}); }
Expression inv(Expression e)        { return e.push(Operation::INVERSE,     {e.top}); }
Expression operator|(Expression e0, Expression e1) { return e0.push(Operation::WIRE_OR,   {e0.top, e0.append(e1)}); }
Expression operator&(Expression e0, Expression e1) { return e0.push(Operation::WIRE_AND,   {e0.top, e0.append(e1)}); }
Expression operator^ (Expression e0, Expression e1) { return e0.push(Operation::WIRE_XOR,   {e0.top, e0.append(e1)}); }
Expression booleanXor(Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_XOR,   {e0.top, e0.append(e1)}); }
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
Expression operator&& (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_AND,   {e0.top, e0.append(e1)}); }
Expression operator|| (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_OR,    {e0.top, e0.append(e1)}); }
Expression operator|| (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_OR,    {e0.top, e1}); }
Expression operator&& (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_AND,   {e0.top, e1}); }
Expression operator^ (Expression e0, Operand e1) { return e0.push(Operation::WIRE_XOR,   {e0.top, e1}); }
Expression booleanXor(Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_XOR,   {e0.top, e1}); }
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
Expression operator& (Expression e0, Operand e1) { return e0.push(Operation::WIRE_AND,   {e0.top, e1}); }
Expression operator| (Expression e0, Operand e1) { return e0.push(Operation::WIRE_OR,    {e0.top, e1}); }
Expression operator||(Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_OR,    {e0, e1.top}); }
Expression operator&&(Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_AND,   {e0, e1.top}); }
Expression operator^ (Operand e0, Expression e1) { return e1.push(Operation::WIRE_XOR,   {e0, e1.top}); }
Expression booleanXor(Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_XOR,   {e0, e1.top}); }
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
Expression operator& (Operand e0, Expression e1) { return e1.push(Operation::WIRE_AND,   {e0, e1.top}); }
Expression operator| (Operand e0, Expression e1) { return e1.push(Operation::WIRE_OR,    {e0, e1.top}); }

Expression booleanOr(Expression e0)  { return e0.push(Operation::BOOLEAN_OR,  {e0.top}); }
Expression booleanAnd(Expression e0) { return e0.push(Operation::BOOLEAN_AND, {e0.top}); }
Expression booleanXor(Expression e0) { return e0.push(Operation::BOOLEAN_XOR, {e0.top}); }
Expression wireOr(Expression e0)  { return e0.push(Operation::WIRE_OR,  {e0.top}); }
Expression wireAnd(Expression e0) { return e0.push(Operation::WIRE_AND, {e0.top}); }
Expression wireXor(Expression e0) { return e0.push(Operation::WIRE_XOR, {e0.top}); }
Expression add(Expression e0)        { return e0.push(Operation::ADD,         {e0.top}); }
Expression mult(Expression e0)       { return e0.push(Operation::MULTIPLY,    {e0.top}); }

Expression array(vector<Expression> e0)      { Expression e; return e.push(Operation::ARRAY,       e.append(e0)); }
Expression booleanOr(vector<Expression> e0)  { Expression e; return e.push(Operation::BOOLEAN_OR,  e.append(e0)); }
Expression booleanAnd(vector<Expression> e0) { Expression e; return e.push(Operation::BOOLEAN_AND, e.append(e0)); }
Expression booleanXor(vector<Expression> e0) { Expression e; return e.push(Operation::BOOLEAN_XOR, e.append(e0)); }
Expression wireOr(vector<Expression> e0)  { Expression e; return e.push(Operation::WIRE_OR,  e.append(e0)); }
Expression wireAnd(vector<Expression> e0) { Expression e; return e.push(Operation::WIRE_AND, e.append(e0)); }
Expression wireXor(vector<Expression> e0) { Expression e; return e.push(Operation::WIRE_XOR, e.append(e0)); }
Expression add(vector<Expression> e0)        { Expression e; return e.push(Operation::ADD,         e.append(e0)); }
Expression mult(vector<Expression> e0)       { Expression e; return e.push(Operation::MULTIPLY,    e.append(e0)); }

Expression call(string func_name, vector<Expression> args) {
	args.insert(args.begin(), Expression::stringOf(func_name));

	Expression result;
	return result.push(Operation::CALL, result.append(args));
}

int passesGuard(const State &encoding, const State &global, const Expression &guard, State *total) {
	vector<Value> expressions;
	vector<Value> gexpressions;

	for (ConstUpIterator i(guard.sub, {guard.top}); not i.done(); ++i) {
		Value g = i->evaluate(global, gexpressions);
		Value l = i->evaluate(encoding, expressions);

		if (l.isUnstable() or g.isUnstable()
			or (g.isNeutral() and l.isValid())
			or (g.isValid() and l.isNeutral())
			or (g.isValid() and l.isValid() and not areSame(g, l))) {
			l = Value::X();
		}

		if (i->exprIndex >= expressions.size()) {
			expressions.resize(i->exprIndex+1, Value::X());
		}
		if (i->exprIndex >= gexpressions.size()) {
			gexpressions.resize(i->exprIndex+1, Value::X());
		}
		expressions[i->exprIndex] = l;
		gexpressions[i->exprIndex] = g;
	}

	if (expressions[guard.top.index].isUnknown() or expressions[guard.top.index].isValid()) {
		if (gexpressions[guard.top.index].isNeutral() or gexpressions[guard.top.index].isUnknown()) {
			expressions[guard.top.index] = Value::X();
		} else if (gexpressions[guard.top.index].isValid()) {
			expressions[guard.top.index] = gexpressions[guard.top.index];
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
		for (ConstDownIterator i(guard.sub, {guard.top}); not i.done(); ++i) {
			i->propagate(*total, global, expressions, gexpressions, expressions[i->exprIndex]);
		}
	}

	if (expressions.empty() or expressions[guard.top.index].isNeutral()) {
		return -1;
	} else if (expressions[guard.top.index].isUnstable()) {
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

void Expression::minimize(RuleSet rules) {
	this->top = arithmetic::minimize(*this, {this->top}, rules).map(this->top);
}

void Expression::tidy() {
	this->top = arithmetic::tidy(*this, {this->top}).map(this->top);
}

}
