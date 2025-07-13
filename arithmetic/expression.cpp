#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>

namespace arithmetic {

UpIterator Expression::exprUp(size_t exprIndex) {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return UpIterator(*this, {exprIndex});
}

ConstUpIterator Expression::exprUp(size_t exprIndex) const {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return ConstUpIterator(*this, {exprIndex});
}

DownIterator Expression::exprDown(size_t exprIndex) {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return DownIterator(*this, {exprIndex});
}

ConstDownIterator Expression::exprDown(size_t exprIndex) const {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return ConstDownIterator(*this, {exprIndex});
}

void Expression::breakMap() const {
	exprMapIsDirty = true;
}

void Expression::fixMap() const {
	exprMap.clear();
	for (int i = 0; i < (int)operations.size(); i++) {
		if (operations[i].exprIndex >= exprMap.size()) {
			exprMap.resize(operations[i].exprIndex+1, -1);
		}
		exprMap[operations[i].exprIndex] = i;
	}
	exprMapIsDirty = false;
}

void Expression::setExpr(size_t exprIndex, size_t index) const {
	if (not exprMapIsDirty) {
		if (exprIndex >= exprMap.size()) {
			exprMap.resize(exprIndex+1);
		}
		exprMap[exprIndex] = index;
	}
}

int Expression::lookup(size_t exprIndex) const {
	if (exprMapIsDirty) {
		fixMap();
	}

	if (exprIndex >= exprMap.size()) {
		printf("internal:%s:%d: expression index not found in mapping %lu: %s\n", __FILE__, __LINE__, exprIndex, ::to_string(exprMap).c_str());
		return -1;
	}
	int idx = exprMap[exprIndex];
	if (idx < 0 or idx >= (int)operations.size()) {
		printf("internal:%s:%d: operation not found in expression %lu: %s\n", __FILE__, __LINE__, exprIndex, ::to_string(exprMap).c_str());
		return -1;
	}
	return idx;
}

Expression::Expression(Operand arg0) {
	Operation::loadOperators();
	exprMapIsDirty = false;
	nextExprIndex = 0;

	top = arg0;
}

Expression::Expression(int func, vector<Operand> args) {
	Operation::loadOperators();
	exprMapIsDirty = false;
	nextExprIndex = 0;

	operations.push_back(Operation(func, args, nextExprIndex++));
	setExpr(operations.back().exprIndex, operations.size()-1);
	top = Operand::exprOf(operations.back().exprIndex);
}

Expression::~Expression() {
}

vector<Operand> Expression::exprIndex() const {
	vector<Operand> result;
	result.reserve(operations.size());
	for (auto i = operations.begin(); i != operations.end(); i++) {
		result.push_back(Operand::exprOf(i->exprIndex));
	}
	return result;
}

const Operation *Expression::getExpr(size_t index) const {
	int i = lookup(index);
	if (i < 0) {
		return nullptr;
	}
	return &operations[i];
}

bool Expression::setExpr(Operation o) {
	int i = lookup(o.exprIndex);
	if (i >= 0) {
		operations[i] = o;
		return true;
	}
	return false;
}

Operand Expression::pushExpr(Operation o) {
	o.exprIndex = nextExprIndex++;
	operations.push_back(o);
	setExpr(o.exprIndex, operations.size()-1);
	return Operand::exprOf(o.exprIndex);
}

bool Expression::eraseExpr(size_t index) {
	int i = lookup(index);
	if (i >= 0) {
		operations.erase(operations.begin()+i);
		breakMap();
		return true;
	}
	return false;
}

void Expression::clear() {
	nextExprIndex = 0;
	top = Operand::undef();
	exprMap.clear();
	exprMapIsDirty = false;
}

Operand Expression::append(Expression arg) {
	for (auto i = arg.operations.begin(); i != arg.operations.end(); i++) {
		operations.push_back(i->offsetExpr(nextExprIndex));
		setExpr(operations.back().exprIndex, operations.size()-1);
	}
	Operand result = arg.top.offsetExpr(nextExprIndex);
	nextExprIndex += arg.nextExprIndex;
	return result;
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
	int pos = -1;
	for (int i = 0; i < (int)operations.size() and pos < 0; i++) {
		if (areSame(operations[i], arg)) {
			pos = i;
		}
	}
	if (pos < 0) {
		pos = (int)operations.size();
		pushExpr(arg);
	}
	top = Operand::exprOf(operations[pos].exprIndex);
	return *this;
}

bool Expression::isNull() const {
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

bool Expression::isConstant() const {
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

bool Expression::isValid() const {
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

bool Expression::isNeutral() const {
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

bool Expression::isWire() const {
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

void Expression::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	breakMap();
	for (auto i = operations.begin(); i != operations.end(); i++) {
		i->apply(uidMap);
	}
}

void Expression::apply(vector<Expression> sub) {
	if (sub.empty()) {
		return;
	}

	breakMap();
	size_t sz = operations.size();

	vector<Operand> result;
	for (auto i = sub.begin(); i != sub.end(); i++) {
		for (auto j = i->operations.begin(); j != i->operations.end(); i++) {
			operations.push_back(j->offsetExpr(nextExprIndex));
		}
		result.push_back(i->top.offsetExpr(nextExprIndex));
		nextExprIndex += i->nextExprIndex;
	}
	for (int i = 0; i < (int)sz; i++) {
		operations[i].apply(result);
	}
	top.apply(result);
}

Operand Expression::extract(size_t exprIndex, vector<size_t> idx) {
	Operation o0 = *getExpr(exprIndex);
	Operand result = pushExpr(Operation());
	setExpr(o0.extract(idx, result.index));
	setExpr(o0);
	return result;
}

void Expression::replace(Operand from, Operand to) {
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (areSame(*j, from)) {
				*j = to;
			}
		}
	}
}

void Expression::replace(const Expression &rules, Match match) {
	if (not match.replace.isExpr()) {
		size_t ins = 0;
		size_t slotIndex = match.expr.back();
		match.expr.pop_back();

		auto slotPtr = getExpr(slotIndex);
		if (slotPtr == nullptr) {
			printf("internal:%s:%d: expression %lu not found\n", __FILE__, __LINE__, slotIndex);
			return;
		}
		Operation slot = *slotPtr;

		if (match.top.empty()) {
			slot.operands.clear();
			slot.func = -1;
		} else {
			// If this match doesn't cover all operands, we only want to replace
			// the ones that are covered.
			for (int i = (int)match.top.size()-1; i >= 0; i--) {
				slot.operands.erase(slot.operands.begin() + match.top[i]);
			}
			ins = match.top[0];
		}

		if (match.replace.isVar()) {
			auto v = match.vars.find(match.replace.index);
			if (v != match.vars.end()) {
				slot.operands.insert(
					slot.operands.begin()+ins,
					v->second.begin(), v->second.end());
			} else {
				printf("variable not mapped\n");
			}
		} else {
			slot.operands.insert(
				slot.operands.begin()+ins,
				match.replace);
		}
		match.top.clear();

		setExpr(slot);
	} else {
		size_t matchTop = this->top.index;
		if (not match.expr.empty()) {
			matchTop = match.expr.back();
		}
		//cout << "top=" << matchTop << " expr=" << ::to_string(match.expr) << endl;
		if (match.replace.isExpr()
			and rules.getExpr(match.replace.index)->func != getExpr(matchTop)->func
			and match.top.size() < getExpr(matchTop)->operands.size()) {
			// We matched to a commutative operation and we need to collapse the
			// matched operands to the output of our expression.
			match.expr.back() = extract(matchTop, match.top).index;
			match.top.clear();
		}

		//cout << "top=" << matchTop << " expr=" << ::to_string(match.expr) << endl;
		//cout << "after insert: " << *this << endl;

		// Iterate over the replacement expression
		map<size_t, size_t> exprMap;
		for (auto curr = ConstDownIterator(rules, {match.replace.index}); not curr.done(); ++curr) {
			// Along the way, compute the exprIndex mapping
			auto pos = exprMap.insert({curr->exprIndex, 0});
			if (pos.second) {
				if (match.expr.empty()) {
					operations.push_back(Operation(-1, {}, nextExprIndex++));
					setExpr(operations.back().exprIndex, operations.size()-1);
					pos.first->second = operations.back().exprIndex;
				} else {
					pos.first->second = match.expr.back();
					match.expr.pop_back();
				}
			}
			auto slotPtr = getExpr(pos.first->second);
			if (slotPtr == nullptr) {
				printf("internal:%s:%d: expression %lu not found\n", __FILE__, __LINE__, pos.first->second);
				return;
			}
			Operation slot = *slotPtr;

			slot.func = curr->func;
			size_t ins = 0;
			if (match.top.empty()) {
				slot.operands.clear();
			} else {
				// TODO(edward.bingham) this is causing a out of bounds exception --
				//           may have been fixed by clearing match.top on line 845.

				// If this match doesn't cover all operands, we only want to replace
				// the ones that are covered.
				for (int i = (int)match.top.size()-1; i >= 0; i--) {
					slot.operands.erase(slot.operands.begin() + match.top[i]);
				}
				ins = match.top[0];
			}


			for (auto op = curr->operands.begin(); op != curr->operands.end(); op++) {
				if (op->isExpr()) {
					pos = exprMap.insert({op->index, 0});
					if (pos.second) {
						if (match.expr.empty()) {
							operations.push_back(Operation(-1, {}, nextExprIndex++));
							setExpr(operations.back().exprIndex, operations.size()-1);
							pos.first->second = operations.back().exprIndex;
						} else {
							pos.first->second = match.expr.back();
							match.expr.pop_back();
						}
					}
					
					slot.operands.insert(
						slot.operands.begin()+ins,
						Operand::exprOf(pos.first->second));
					++ins;
				} else if (op->isVar()) {
					auto v = match.vars.find(op->index);
					if (v != match.vars.end()) {
						slot.operands.insert(
							slot.operands.begin()+ins,
							v->second.begin(), v->second.end());
						ins += v->second.size();
					} else {
						printf("variable not mapped\n");
					}
				} else {
					slot.operands.insert(
						slot.operands.begin()+ins,
						*op);
					++ins;
				}
			}
			match.top.clear();
			setExpr(slot);
		}
	}
	//cout << "expr=" << ::to_string(match.expr) << endl;
	//cout << "after mapping: " << *this << endl;

	if (not match.expr.empty()) {
		vector<int> toErase;
		for (auto i = match.expr.begin(); i != match.expr.end(); i++) {
			int idx = lookup(*i);
			if (idx >= 0) {
				toErase.push_back(idx);
			}
		}
		sort(toErase.begin(), toErase.end());
		for (int i = (int)toErase.size()-1; i >= 0; i--) {
			breakMap();
			operations.erase(operations.begin() + toErase[i]);
		}
		// cout << "after erase: " << *this << endl;
	}
}

Expression &Expression::minimize(Expression directed) {
	static const Expression rules = rewriteBasic();
	if (directed.operations.empty()) {
		directed = rules;
	}

	// cout << "Rules: " << rules << endl;

	top.replace(tidy(*this, {top.index}));
	vector<Match> tokens = search(*this, directed, directed.top, 1u);
	while (not tokens.empty()) {
		//cout << "Expr: " << *this << endl;
		//cout << "Match: " << ::to_string(tokens) << endl;
		replace(directed, tokens.back());
		//cout << "Replace: " << *this << endl;
		top.replace(tidy(*this, {top.index}));
		//cout << "Canon: " << *this << endl << endl;
		tokens = search(*this, directed, directed.top, 1u);
	}

	// TODO(edward.bingham) Then I need to implement encoding
	// Use the unidirectional expression rewrite system?
	// propagate validity?
	
	return *this;
}

bool areSame(Expression e0, Expression e1) {
	if (e0.operations.size() != e1.operations.size()) {
		return false;
	}

	for (int i = 0; i < (int)e0.operations.size(); i++) {
		if (not areSame(e0.operations[i], e1.operations[i])) {
			return false;
		}
	}
	return true;
}

/*Expression espresso(Expression expr, vector<Type> vars, Expression directed, Expression undirected) {
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

	//Expression result;
	//Cost best(1e20, 1e20);
	expr.minimize(directed);

	/ *vector<pair<Cost, Expression> > stack;
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
	}* /

	return expr;
}*/

Expression &Expression::operator=(Operand e) {
	clear();
	top = e;
	return *this;
}

string Expression::to_string() const {
	ostringstream oss;
	oss << "top: " << top << "/" << nextExprIndex << endl;
	for (int i = (int)operations.size()-1; i >= 0; i--) {
		oss << i << ": " << operations[i] << endl;
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
Expression operator||(Expression e0, Expression e1) { return e0.push(Operation::BITWISE_NOT,   {e0.top, e0.append(e1)}); }
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
Expression operator|(Operand e0, Operand e1)  { return Expression(Operation::BOOLEAN_OR,    {e0, e1}); }
Expression operator& (Operand e0, Operand e1) { return Expression(Operation::BOOLEAN_AND,   {e0, e1}); }
Expression operator^ (Operand e0, Operand e1) { return Expression(Operation::BOOLEAN_XOR,   {e0, e1}); }
Expression bitwiseXor(Operand e0, Operand e1) { return Expression(Operation::BITWISE_XOR,   {e0, e1}); }
Expression operator==(Operand e0, Operand e1) { return Expression(Operation::EQUAL,         {e0, e1}); }
Expression operator!=(Operand e0, Operand e1) { return Expression(Operation::NOT_EQUAL,     {e0, e1}); }
Expression operator<(Operand e0, Operand e1)  { return Expression(Operation::LESS,          {e0, e1}); }
Expression operator>(Operand e0, Operand e1)  { return Expression(Operation::GREATER,       {e0, e1}); }
Expression operator<=(Operand e0, Operand e1) { return Expression(Operation::LESS_EQUAL,    {e0, e1}); }
Expression operator>=(Operand e0, Operand e1) { return Expression(Operation::GREATER_EQUAL, {e0, e1}); }
Expression operator<<(Operand e0, Operand e1) { return Expression(Operation::SHIFT_LEFT,    {e0, e1}); }
Expression operator>>(Operand e0, Operand e1) { return Expression(Operation::SHIFT_RIGHT,   {e0, e1}); }
Expression operator+ (Operand e0, Operand e1) { return Expression(Operation::ADD,           {e0, e1}); }
Expression operator- (Operand e0, Operand e1) { return Expression(Operation::SUBTRACT,      {e0, e1}); }
Expression operator* (Operand e0, Operand e1) { return Expression(Operation::MULTIPLY,      {e0, e1}); }
Expression operator/ (Operand e0, Operand e1) { return Expression(Operation::DIVIDE,        {e0, e1}); }
Expression operator% (Operand e0, Operand e1) { return Expression(Operation::MOD,           {e0, e1}); }
Expression operator&&(Operand e0, Operand e1) { return Expression(Operation::BITWISE_AND,   {e0, e1}); }
Expression operator||(Operand e0, Operand e1) { return Expression(Operation::BITWISE_OR,    {e0, e1}); }
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
	args.insert(args.begin(), Operand::typeOf(func));

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
