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

Expression::Expression(const State &state) {
	Operation::loadOperators();

	vector<Operand> args;
	for (int i = 0; i < (int)state.values.size(); i++) {
		if (state.values[i].isUnknown()) {
			continue;
		}

		if (state.values[i].isNeutral()) {
			args.push_back(pushExpr(Operation(Operation::WIRE_NOT, {Operand::varOf(i)})));
		} else if (state.values[i].type == Value::WIRE) {
			args.push_back(Operand::varOf(i));
		} else {
			Operand op = pushExpr(Operation(Operation::EQUAL, {Operand::varOf(i), Operand(state.values[i])}));
			args.push_back(pushExpr(Operation(Operation::TRUTHINESS, {op})));
		}
	}

	if (args.empty()) {
		top = Operand::vdd();
	} else if (args.size() == 1u) {
		top = args[0];
	} else {
		top = pushExpr(Operation(Operation::WIRE_AND, args));
	}
}

Expression::Expression(const Region &region) {
	Operation::loadOperators();

	vector<Expression> states;
	for (const State &state : region.states) {
		states.push_back(Expression(state));
	}

	vector<Operand> args = append(states);

	if (args.empty()) {
		top = Operand::gnd();
	} else if (args.size() == 1u) {
		top = args[0];
	} else {
		top = pushExpr(Operation(Operation::WIRE_OR, args));
	}
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

Expression Expression::structOf(string type, vector<Value> arr) {
	Expression result;
	result.top = Operand::structOf(type, arr);
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

Expression Expression::labelOf(string tag) {
	Expression result;
	result.top = Operand::labelOf(tag);
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
	return arg.top.applyExprs(sub.append(arg.sub, {arg.top}));
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

bool Expression::isUndef() const {
	return top.isUndef();
}

bool Expression::isNull() const {
	// TODO(edward.bingham) This is wrong. I should do constant propagation here
	// then check if the top Expression is null after constant propagation using quantified element elimination
	// TODO(edward.bingham) implement quantified element elimination using cylindrical algebraic decomposition.
	if (top.isVar() or top.isUndef() or (top.isConst() and not top.cnst.isUnstable())) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		auto expr = getExpr(i->index);
		if (expr->func == Operation::CALL) {
			return false;
		}

		for (auto j = expr->operands.begin(); j != expr->operands.end(); j++) {
			if (j->isVar() or j->isUndef() or (j->isConst() and not j->cnst.isUnstable())) {
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
	if (top.isVar() or top.isUndef() or (top.isConst() and top.cnst.isUnstable())) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		auto expr = getExpr(i->index);
		if (expr->func == Operation::CALL) {
			return false;
		}

		for (auto j = expr->operands.begin(); j != expr->operands.end(); j++) {
			if (j->isVar() or j->isUndef() or (j->isConst() and j->cnst.isUnstable())) {
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
	if (top.isVar() or top.isUndef() or (top.isConst() and (top.cnst.isUnstable() or top.cnst.isNeutral()))) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		auto expr = getExpr(i->index);
		if (expr->func == Operation::CALL) {
			return false;
		}

		for (auto j = expr->operands.begin(); j != expr->operands.end(); j++) {
			if (j->isVar() or j->isUndef() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isNeutral()))) {
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
	if (top.isVar() or top.isUndef() or (top.isConst() and (top.cnst.isUnstable() or top.cnst.isValid()))) {
		return false;
	}
	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		auto expr = getExpr(i->index);
		if (expr->func == Operation::CALL) {
			return false;
		}

		for (auto j = expr->operands.begin(); j != expr->operands.end(); j++) {
			if (j->isVar() or j->isUndef() or (j->isConst() and (j->cnst.isUnstable() or j->cnst.isValid()))) {
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

bool Expression::isMember() const {
	return arithmetic::isMember(sub, top);
}

Expression Expression::popMember() {
	return arithmetic::popMember(sub, top);
}

Expression &Expression::applyVars(const Mapping<size_t> &m) {
	top.applyVars(m);

	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		setExpr(Operation(*getExpr(i->index)).applyVars(m));
	}
	return *this;
}

Expression &Expression::applyVars(const Mapping<int> &m) {
	top.applyVars(m);

	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		setExpr(Operation(*getExpr(i->index)).applyVars(m));
	}
	return *this;
}

Expression &Expression::apply(const Mapping<Operand> &m) {
	top = m.map(top);

	vector<Operand> idx = exprIndex();
	for (auto i = idx.begin(); i != idx.end(); i++) {
		setExpr(Operation(*getExpr(i->index)).apply(m));
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
	if ((not e0.top.isExpr() or not e1.top.isExpr()) and e0.top != e1.top) {
		return false;
	}
	auto i = ConstDownIterator(e0, {e0.top}), j = ConstDownIterator(e1, {e1.top});
	for (; not i.done() and not j.done(); ++i, ++j) {
		const Operation &o0 = i.get();
		const Operation &o1 = j.get();

		if (o0.func != o1.func or
			o0.operands.size() != o1.operands.size()) {
			return false;
		}

		for (int k = 0; k < (int)o0.operands.size(); k++) {
			if ((not o0.operands[k].isExpr() or not o1.operands[k].isExpr())
				and o0.operands[k] != o1.operands[k]) {
				return false;
			}
		}
	}
	return i.done() and j.done();
}

string Expression::to_string(bool debug, ucs::ConstNetlist symbols) const {
	return arithmetic::to_string(*this, this->top, debug, symbols);
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
Expression operator|(Expression e0, Expression e1) { return e0.push(Operation::WIRE_OR,        {e0.top, e0.append(e1)}); }
Expression operator&(Expression e0, Expression e1) { return e0.push(Operation::WIRE_AND,       {e0.top, e0.append(e1)}); }
Expression operator^ (Expression e0, Expression e1) { return e0.push(Operation::WIRE_XOR,      {e0.top, e0.append(e1)}); }
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
Expression operator/ (Expression e0, Expression e1) { return e0.push(Operation::INTDIV,        {e0.top, e0.append(e1)}); }
Expression operator% (Expression e0, Expression e1) { return e0.push(Operation::INTMOD,        {e0.top, e0.append(e1)}); }
Expression operator&& (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_AND,  {e0.top, e0.append(e1)}); }
Expression operator|| (Expression e0, Expression e1) { return e0.push(Operation::BOOLEAN_OR,   {e0.top, e0.append(e1)}); }
Expression operator|| (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_OR,   {e0.top, e1}); }
Expression operator&& (Expression e0, Operand e1) { return e0.push(Operation::BOOLEAN_AND,  {e0.top, e1}); }
Expression operator^ (Expression e0, Operand e1) { return e0.push(Operation::WIRE_XOR,      {e0.top, e1}); }
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
Expression operator/ (Expression e0, Operand e1) { return e0.push(Operation::INTDIV,        {e0.top, e1}); }
Expression operator% (Expression e0, Operand e1) { return e0.push(Operation::INTMOD,        {e0.top, e1}); }
Expression operator& (Expression e0, Operand e1) { return e0.push(Operation::WIRE_AND,      {e0.top, e1}); }
Expression operator| (Expression e0, Operand e1) { return e0.push(Operation::WIRE_OR,       {e0.top, e1}); }
Expression operator||(Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_OR,    {e0, e1.top}); }
Expression operator&&(Operand e0, Expression e1) { return e1.push(Operation::BOOLEAN_AND,   {e0, e1.top}); }
Expression operator^ (Operand e0, Expression e1) { return e1.push(Operation::WIRE_XOR,      {e0, e1.top}); }
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
Expression operator/ (Operand e0, Expression e1) { return e1.push(Operation::INTDIV,        {e0, e1.top}); }
Expression operator% (Operand e0, Expression e1) { return e1.push(Operation::INTMOD,        {e0, e1.top}); }
Expression operator& (Operand e0, Expression e1) { return e1.push(Operation::WIRE_AND,      {e0, e1.top}); }
Expression operator| (Operand e0, Expression e1) { return e1.push(Operation::WIRE_OR,       {e0, e1.top}); }

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

Expression memberCall(Expression recv, string funcName, vector<Expression> args) {
	args.insert(args.begin(), recv);
	args.insert(args.begin(), Expression::labelOf(funcName));

	Expression result;
	return result.push(Operation::MEMBER_CALL, result.append(args));
}

Expression memberCall(string funcName, vector<Expression> args) {
	args.insert(args.begin(), Expression::labelOf(funcName));

	Expression result;
	return result.push(Operation::MEMBER_CALL, result.append(args));
}

Expression call(string funcName, vector<Expression> args) {
	args.insert(args.begin(), Expression::labelOf(funcName));

	Expression result;
	return result.push(Operation::CALL, result.append(args));
}

Expression cast(string typeName, Expression e0) {
	return e0.push(Operation::CAST, {Operand::labelOf(typeName), e0.top});
}

Expression construct(string funcName, vector<Expression> args) {
	args.insert(args.begin(), Expression::labelOf(funcName));

	Expression result;
	return result.push(Operation::STRUCT, result.append(args));
}

int passesGuard(const State &encoding, const State &global, const Expression &guard, State *total) {
	vector<ValRef> expressions;
	vector<ValRef> gexpressions;

	for (ConstUpIterator i(guard.sub, {guard.top}); not i.done(); ++i) {
		ValRef g = i->evaluate(global, gexpressions);
		ValRef l = i->evaluate(encoding, expressions);

		if (l.val.isUnstable() or g.val.isUnstable()
			or (g.val.isNeutral() and l.val.isValid())
			or (g.val.isValid() and l.val.isNeutral())
			or (g.val.isValid() and l.val.isValid() and not areSame(g.val, l.val))) {
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

	ValRef gtop = guard.top.get(global, gexpressions);
	ValRef top = guard.top.get(encoding, expressions);
	if (top.val.isUnknown() or top.val.isValid()) {
		if (gtop.val.isNeutral() or gtop.val.isUnknown()) {
			top.val = Value::X();
		} else if (gtop.val.isValid()) {
			top = gtop;
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
	if (total != nullptr and top.val.isValid()) {
		for (ConstDownIterator i(guard.sub, {guard.top}); not i.done(); ++i) {
			i->propagate(*total, global, expressions, gexpressions, expressions[i->exprIndex].val);
		}
	}

	if (top.val.isNeutral()) {
		return -1;
	} else if (top.val.isUnstable()) {
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

vector<int> passesConstraint(const Region &r0, const Expression &mutex) {
	vector<int> result;
	for (int i = 0; i < (int)r0.states.size(); i++) {
		ValRef v0 = evaluate(mutex, mutex.top, r0.states[i]);
		if (v0.val.isValid()) {
			result.push_back(i);
		}
	}
	return result;
}

Expression &Expression::minimize(RuleSet rules) {
	this->top = arithmetic::minimize(*this, {this->top}, rules).map(this->top);
	return *this;
}

Expression Expression::minimized(RuleSet rules) const {
	Expression duplicate(*this);
	return duplicate.minimize(rules);
}

void Expression::tidy() {
	this->top = arithmetic::tidy(*this, {this->top}).map(this->top);
}

}
