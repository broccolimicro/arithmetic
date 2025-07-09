#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>
#include <common/combinatoric.h>

namespace arithmetic {

Expression::iterator::iterator(Expression *root) {
	this->root = root;
	if (root != nullptr) {
		expand = vector<bool>(root->operations.size(), false);
		seen = vector<bool>(root->operations.size(), false);
	}
}

Expression::iterator::~iterator() {
}

Operation &Expression::iterator::get() {
	return root->operations[stack.back()];
}

vector<Operation>::iterator Expression::iterator::at() {
	return root->operations.begin() + stack.back();
}

Operation &Expression::iterator::operator*() {
	return root->operations[stack.back()];
}

vector<Operation>::iterator Expression::iterator::operator->() {
	return root->operations.begin() + stack.back();
}

Expression::iterator &Expression::iterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root->operations.begin() + stack.back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				int idx = root->lookup(i->index);
				if (idx >= 0 and not seen[idx]) {
					stack.push_back(idx);
					seen[idx] = true;
				}
			}
		}
	}

	return *this;
}

bool operator==(const Expression::iterator &i0, const Expression::iterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const Expression::iterator &i0, const Expression::iterator &i1) {
	return i0.stack != i1.stack;
}

Expression::const_iterator::const_iterator(const Expression *root) {
	this->root = root;
	if (root != nullptr) {
		expand = vector<bool>(root->operations.size(), false);
		seen = vector<bool>(root->operations.size(), false);
	}
}

Expression::const_iterator::~const_iterator() {
}

const Operation &Expression::const_iterator::get() {
	return root->operations[stack.back()];
}

vector<Operation>::const_iterator Expression::const_iterator::at() {
	return root->operations.begin() + stack.back();
}

const Operation &Expression::const_iterator::operator*() {
	return root->operations[stack.back()];
}

vector<Operation>::const_iterator Expression::const_iterator::operator->() {
	return root->operations.begin() + stack.back();
}

Expression::const_iterator &Expression::const_iterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root->operations.begin() + stack.back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				int idx = root->lookup(i->index);
				if (idx >= 0 and not seen[idx]) {
					stack.push_back(idx);
					seen[idx] = true;
				}
			}
		}
	}

	return *this;
}

bool operator==(const Expression::const_iterator &i0, const Expression::const_iterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const Expression::const_iterator &i0, const Expression::const_iterator &i1) {
	return i0.stack != i1.stack;
}

Expression::iterator Expression::at(Operand op) {
	iterator result(this);
	if (op.isExpr()) {
		int idx = lookup(op.index);
		if (idx >= 0) {
			result.stack.push_back(idx);
		}
	}
	++result;
	return result;
}

Expression::iterator Expression::begin() {
	return at(top);
}

Expression::iterator Expression::end() {
	return iterator(this);
}

Expression::const_iterator Expression::at(Operand op) const {
	const_iterator result(this);
	if (op.isExpr()) {
		int idx = lookup(op.index);
		if (idx >= 0) {
			result.stack.push_back(idx);
		}
	}
	++result;
	return result;
}

Expression::const_iterator Expression::begin() const {
	return at(top);
}

Expression::const_iterator Expression::end() const {
	return const_iterator(this);
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
		printf("internal:%s:%d: expression index not found in mapping\n", __FILE__, __LINE__);
		return -1;
	}
	int idx = exprMap[exprIndex];
	if (idx < 0 or idx >= (int)operations.size()) {
		printf("internal:%s:%d: operation not found in expression\n", __FILE__, __LINE__);
		return -1;
	}
	return idx;
}

Expression::Expression() {
	Operation::loadOperators();
	exprMapIsDirty = false;
	nextExprIndex = 0;
	top = Operand::undef();
}

Expression::Expression(Operand arg0) {
	Operation::loadOperators();
	exprMapIsDirty = false;
	nextExprIndex = 0;
	set(arg0);
}

Expression::~Expression() {
}

vector<Operation>::iterator Expression::at(size_t index) {
	int i = lookup(index);
	if (i < 0) {
		return operations.end();
	}
	return operations.begin() + i;
}

vector<Operation>::const_iterator Expression::at(size_t index) const {
	int i = lookup(index);
	if (i < 0) {
		return operations.end();
	}
	return operations.begin() + i;
}

void Expression::clear() {
	nextExprIndex = 0;
	top = Operand::undef();
	exprMap.clear();
	exprMapIsDirty = false;
}

Operand Expression::push(Operation arg) {
	// add to operations list if doesn't exist
	int pos = -1;
	for (int i = 0; i < (int)operations.size() and pos < 0; i++) {
		if (areSame(operations[i], arg)) {
			pos = i;
		}
	}
	if (pos < 0) {
		arg.exprIndex = nextExprIndex++;
		pos = (int)operations.size();
		operations.push_back(arg);
		setExpr(operations.back().exprIndex, operations.size()-1);
	}
	top = Operand::exprOf(operations[pos].exprIndex);
	return top;
}

Operand Expression::push(Expression arg) {
	for (auto i = arg.operations.begin(); i != arg.operations.end(); i++) {
		operations.push_back(i->offsetExpr(nextExprIndex));
		setExpr(operations.back().exprIndex, operations.size()-1);
	}
	top = arg.top.offsetExpr(nextExprIndex);
	nextExprIndex += arg.nextExprIndex;
	return top;
}

Operand Expression::set(Operand arg0) {
	clear();
	top = arg0;
	return top;
}

Operand Expression::set(int func, Operand arg0) {
	clear();
	top = push(Operation(func, {arg0}));
	return top;
}

Operand Expression::set(int func, Expression arg0) {
	operations = arg0.operations;
	top = arg0.top;
	nextExprIndex = arg0.nextExprIndex;
	exprMap = arg0.exprMap;
	exprMapIsDirty = arg0.exprMapIsDirty;
	return push(Operation(func, {top}));
}

Operand Expression::set(int func, Operand arg0, Operand arg1) {
	clear();
	top = push(Operation(func, {arg0, arg1}));
	return top;
}

Operand Expression::set(int func, Expression arg0, Expression arg1) {
	exprMap.clear();
	exprMapIsDirty = true;
	operations = arg0.operations;
	Operand op0 = arg0.top;
	nextExprIndex = arg0.nextExprIndex;
	Operand op1 = push(arg1);
	top = push(Operation(func, {op0, op1}));
	return top;
}

Operand Expression::set(int func, Expression arg0, Operand arg1) {
	operations = arg0.operations;
	Operand op0 = arg0.top;
	nextExprIndex = arg0.nextExprIndex;
	exprMap = arg0.exprMap;
	exprMapIsDirty = arg0.exprMapIsDirty;
	return push(Operation(func, {op0, arg1}));
}

Operand Expression::set(int func, Operand arg0, Expression arg1) {
	operations = arg1.operations;
	Operand op1 = arg1.top;
	nextExprIndex = arg1.nextExprIndex;
	exprMap = arg1.exprMap;
	exprMapIsDirty = arg1.exprMapIsDirty;
	return push(Operation(func, {arg0, op1}));
}

Operand Expression::set(int func, vector<Expression> args) {
	clear();
	exprMapIsDirty = true;
	vector<Operand> operands;
	for (auto i = args.begin(); i != args.end(); i++) {
		operands.push_back(push(*i));
	}
	return push(Operation(func, operands));
}

Value Expression::evaluate(State values) const {
	// I need a value per iterator combination.
	vector<Value> result;
	for (auto i = operations.begin(); i != operations.end(); i++) {
		result.push_back(i->evaluate(values, result));
	}

	return result.empty() ? Value() : result.back();
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

// tidy() does a few things:
// 1. propagate constants
// 2. remove reflexive operations
// 3. remove unitary commutative operations (if not rules)
// 4. remove dangling operations
// ------------ 5. merge successive commutative operations
// 6. sort operands into a canonical order for commutative operations
Expression &Expression::tidy(bool rules) {
	// Start from the top and do depth first search. That zips up the graph for
	// us. First, we need a mapping of index in operations to exprIndex so we
	// don't have to search for the exprIndex each time.

	// DESIGN(edward.bingham) This algorithm assumes that there are no cycles in
	// the graph. So, a directed acyclic graph. If there are cycles, this
	// function will fail silently and produce a result that doesn't sufficiently
	// propagate all constants. The end result should still be functionally
	// equivalent.

	vector<Operand> replace(nextExprIndex, Operand::undef());

	auto curr = begin();
	for (; curr != end(); ++curr) {
		//cout << "start: " << curr.get() << endl;
		curr->replace(replace);
		curr->tidy();
		//cout << "replaced: " << curr.get() << endl;

		if (curr->operands.size() == 1u and curr->operands[0].isConst()) {
			Operand v = Operation::evaluate(curr->func, {curr->operands[0].get()});
			replace[curr->exprIndex] = v;
			//cout << "found const expr[" << curr->exprIndex << "] = " << v << endl;
		}	else if (curr->operands.size() == 1u and (curr->isReflexive()
			or (not rules and curr->isCommutative()))) {
			// replace reflexive expressions
			replace[curr->exprIndex] = curr->operands[0];
			//cout << "found reflex expr[" << curr->exprIndex << "] = " << curr->operands[0] << endl;
		} else {
			// replace identical operations
			for (auto k = operations.begin(); k != operations.end(); k++) {
				if (curr.expand[k->exprIndex] and k != curr.at() and replace[k->exprIndex].isUndef() and areSame(curr.get(), *k)) {
					replace[curr->exprIndex] = Operand::exprOf(k->exprIndex);
					//cout << "found duplicate expr[" << curr->exprIndex << "] = " << Operand::exprOf(k->exprIndex) << endl;
					break;
				}
			}
		}

		//cout << *this << endl;
	}

	if (top.isExpr() and not replace[top.index].isUndef()) {
		top = replace[top.index];
	}

	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (not curr.seen[i] or not replace[operations[i].exprIndex].isUndef()) {
			operations.erase(operations.begin()+i);
		}
	}

	//cout << "done: " << *this << endl;

	return *this;

	// TODO(edward.bingham) reorder operations into a canonical order

	// for each expression, we need to know if it depends on another
	// expression. If so, then the dependency must come first.
	/*vector<vector<size_t> > depend(operations.size());
	for (int i = 0; i < (int)operations.size(); i++) {
		for (auto j = operations[i].begin(); j != operations[i].end(); j++) {
			if (j->isExpr()) {
				depend[i].insert(depend[i].end(), depend[j->index].begin(), depend[j->index].end());
				depend[i].push_back(j->index);
			}
		}
		sort(depend[i].begin(); depend[i].end());
		depend[i].erase(unique(depend[i].begin(), depend[i].end()), depend[i].end());
	}

	// Remapping is an expensive operation, so lets figure out the
	// mapping first before we start swapping indices in the actual
	// expression.
	vector<size_t> mapping(operations.size());
	for (size_t i = 0; i < mapping.size(); i++) {
		mapping[i] = i;
	}

	// One expression A is less than another B if
	// 1. B depends on A
	// 2. A's operation is less than B's
	// 3. Their operations are not commutative 

	for (size_t i = 1; i < mapping.size(); i++) {
		size_t mi = mapping[i];
		size_t j = i;
		for (; j > 0; j--) {
			size_t mj = mapping[j-1];
			// If operation i depends on operation j-1, then it must be greater
			if (find(depend[mi].begin(), depend[mi].end(), mj) != depend[mi].end()
				or operations[mi].func > operations[mj].func) {
				break;
			} else if (operations[mi].func < operations[mj].func) {
				continue;
			}

			
		}
	}*/

	// 1. identify a mapping from expression index -> expression index
	//   a. determine recursively if one expression should be placed before another
	//   b. use that to order expressons from left to right in the operands in a way that is unaffected by expression index
	//   c. order the expressions for post-order traversal
	// 2. apply that mapping and update all indices

	// TODO(edward.bingham) create hash function on expression for insertion into hash map
}

Cost Expression::cost(vector<Type> vars) const {
	if (not top.isExpr()) {
		return Cost();
	}

	double complexity = 0.0;
	vector<Type> expr;

	for (auto curr = begin(); curr != end(); ++curr) {
		pair<Type, double> result(Type(0.0, 0.0, 0.0), 0.0);
		vector<Type> args;
		for (auto j = curr->operands.begin(); j != curr->operands.end(); j++) {
			if (j->isConst()) {
				args.push_back(j->cnst.typeOf());
			} else if (j->isVar() and j->index < vars.size()) {
				args.push_back(vars[j->index]);
			} else if (j->isExpr() and j->index < expr.size()) {
				args.push_back(expr[j->index]);
			} else {
				printf("error: variable not defined for expression\n");
				args.push_back(Type());
			}
		}
		result = curr->funcCost(curr->func, args);
		if (curr->exprIndex >= expr.size()) {
			expr.resize(curr->exprIndex+1);
		}
		expr[curr->exprIndex] = result.first;
		complexity += result.second;
	}

	double delay = 0.0;
	if (top.index < expr.size()) {
		delay = expr[top.index].delay;
	}
	return Cost(complexity, delay);
}

/*vector<Expression::Match> Expression::search(const Expression &rules, size_t count, bool fwd, bool bwd) {
	using Leaf = pair<Operand, Operand>;
	vector<pair<vector<Leaf>, Match> > stack;

	// TODO(edward.bingham) I need a way to canonicalize expressions and hash
	// them so that I can do the state search algorithm.

	// TODO(edward.bingham) rules aren't currently able to match with a variable
	// number of operands. I need to create a comprehension functionality to
	// support those more complex matches.

	// TODO(edward.bingham) look into "tree automata" and "regular tree grammar"
	// as a form of regex for trees instead of sequences.

	if (not rules.top.isExpr() or rules.operations.empty()) {
		printf("error: no replacement rules found\n");
		return vector<Expression::Match>();
	}

	auto ruleTop = rules.operations.begin() + ruleMap[rules.top.index];
	if (ruleTop->func != Operation::ARRAY) {
		printf("error: invalid format for replacement rules\n");
		return vector<Expression::Match>();
	}
	auto rulesBegin = ruleTop->operands.begin();
	auto rulesEnd = ruleTop->operands.end();
	for (auto i = rulesBegin; i != rulesEnd; i++) {
		// ==, <, >
		if (not i->isExpr()) {
			printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
			continue;
		}
		auto j = rules.operations.begin() + ruleMap[i->index];
		if (j->func != Operation::EQUAL
			and j->func != Operation::LESS
			and j->func != Operation::GREATER) {
			printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		}
	}

	// initialize the initial matches
	for (int i = 0; i < (int)operations.size(); i++) {
		// search through the "rules" rules and add all of the matching starts
		for (auto j = rulesBegin; j != rulesEnd; j++) {
			if (not j->isExpr()) {
				continue;
			}
			auto rule = rules.operations.begin() + ruleMap[j->index];
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
				if (canMap({Operand::exprOf(i)}, *lhs, *this, rules, true, &match.vars)) {
					match.replace = *rhs;
					leaves.push_back({Operand::exprOf(i), *lhs});
					stack.push_back({leaves, match});
				}
			}

			// map right to left
			if (rule->func == Operation::LESS or (bwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap({Operand::exprOf(i)}, *rhs, *this, rules, true, &match.vars)) {
					match.replace = *lhs;
					leaves.push_back({Operand::exprOf(i), *rhs});
					stack.push_back({leaves, match});
				}
			}
		}
	}

	//cout << "Search:" << endl;
	//for (int i = 0; i < (int)stack.size(); i++) {
	//	cout << "Stack " << i << ": " << ::to_string(stack[i].first) << " " << stack[i].second << endl;
	//}
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

		//cout << "Curr: " << curr << " from=" << from << " to=" << to << endl;
		//cout << "Leaves: " << ::to_string(leaves) << endl;

		if (to.isExpr()) {
			auto fOp = operations.begin() + mapping[from.index];
			auto tOp = rules.operations.begin() + ruleMap[to.index];

			bool commute = tOp->isCommutative();
			if (commute and tOp->operands.size() == 1u) {
				//cout << "Elastic Commutative" << endl;
				Match nextMatch = curr;
				vector<Leaf> nextLeaves = leaves;
				if (canMap(fOp->operands, tOp->operands[0], *this, rules, false, &nextMatch.vars)) {
					for (size_t i = 0; i < fOp->operands.size(); i++) {
						nextLeaves.push_back({fOp->operands[i], tOp->operands[0]});
						if (nextMatch.top.empty()) {
							nextMatch.top.push_back(i);
						}
					}
					stack.push_back({nextLeaves, nextMatch});
				}
			} else {
				//cout << "Looking for Partial Permutations" << endl;
				CombinatoricIterator it((int)fOp->operands.size(), (int)tOp->operands.size());
				do {
					Match nextMatch = curr;
					vector<Leaf> nextLeaves = leaves;
					vector<size_t> operands;
					bool found = true;
					//cout << "Looking at [";
					for (auto i = it.begin(); i != it.end() and found; i++) {
						//cout << *i << "(" << fOp->operands[*i] << "==" << tOp->operands[i-it.begin()] << ") ";
						nextLeaves.push_back({fOp->operands[*i], tOp->operands[i-it.begin()]});
						operands.push_back(*i);
						found = canMap({fOp->operands[*i]}, tOp->operands[i-it.begin()], *this, rules, false, &nextMatch.vars);
						//if (not found) {
						//	cout << "XX";
						//}
					}
					//cout << "]" << endl;

					if (found) {
						//cout << "found" << endl;
						if (nextMatch.top.empty()) {
							sort(operands.begin(), operands.end());
							nextMatch.top = operands;
						}
						stack.push_back({nextLeaves, nextMatch});
					}
				} while (commute ? it.nextPerm() : it.nextShift());
			}
		} else if (leaves.empty()) {
			//cout << "Found " << curr << endl;
			sort(curr.expr.begin(), curr.expr.end());
			result.push_back(curr);
			if (count != 0 and result.size() >= count) {
				return result;
			}
		} else {
			stack.push_back({leaves, curr});
		}
		//cout << endl;
	}
	//cout << "Done" << endl;
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

		auto currOp = operations.begin() + mapping[curr];

		++result;
		for (auto op = currOp->operands.begin(); op != currOp->operands.end(); op++) {
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

// TODO(edward.bingham) I am currently decoupling the expression ids from the index in the operations vector. This is the next function I have to do.

void Expression::replace(const Expression &rules, Match match) {
	if (not match.replace.isExpr()) {
		size_t ins = 0;
		size_t slot = match.expr.back();
		match.expr.pop_back();

		auto slotOp = operations.begin() + mapping[slot];

		if (match.top.empty()) {
			slotOp->operands.clear();
			slotOp->func = -1;
		} else {
			// If this match doesn't cover all operands, we only want to replace
			// the ones that are covered.
			for (int i = (int)match.top.size()-1; i >= 0; i--) {
				slotOp->operands.erase(slotOp->operands.begin() + match.top[i]);
			}
			ins = match.top[0];
		}

		if (match.replace.isVar()) {
			auto v = match.vars.find(match.replace.index);
			if (v != match.vars.end()) {
				slotOp->operands.insert(
					slotOp->operands.begin()+ins,
					v->second.begin(), v->second.end());
			} else {
				printf("variable not mapped\n");
			}
		} else {
			slotOp->operands.insert(
				slotOp->operands.begin()+ins,
				match.replace);
		}

		match.top.clear();
	} else {
		size_t top = this->top.index;
		if (not match.expr.empty()) {
			top = match.expr.back();
		}
		top = mapping[top];
		//cout << "top=" << top << " expr=" << ::to_string(match.expr) << endl;
		
		// We matched to a commutative operation and we need to collapse the
		// matched operands to the output of our expression.
		if (match.replace.isExpr()
			and rules.operations[match.replace.index].func != operations[top].func
			and match.top.size() < operations[top].operands.size()) {
			insert(top, 1);
			operations[top].func = operations[top+1].func;
			for (int i = (int)match.top.size()-1; i >= 0; i--) {
				operations[top].operands.push_back(operations[top+1].operands[match.top[i]]);
				if (i != 0) {
					operations[top+1].operands.erase(operations[top+1].operands.begin()+match.top[i]);
				}
			}
			operations[top+1].operands[match.top[0]] = Operand::exprOf(top);
		}

		// Allocating space in the operation stack for the new set of operations
		// TODO(edward.bingham) this should no longer be necessary
		size_t num = rules.count(match.replace);
		if (num > match.expr.size()) {
			num -= match.expr.size();
			insert(top, num);
			for (size_t i = 0; i < num; i++) {
				match.expr.push_back(i+top+1);
			}
			top = match.expr.back();
		}

		//cout << "top=" << top << " num=" << num << " expr=" << ::to_string(match.expr) << endl;
		//cout << "after insert: " << *this << endl;

		// Iterate over the replacement expression
		vector<size_t> stack(1, match.replace.index);
		map<size_t, size_t> exprMap;
		while (not stack.empty()) {
			size_t curr = stack.back();
			stack.pop_back();

			// Along the way, compute the exprIndex mapping
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
				// TODO(edward.bingham) this is causing a out of bounds exception
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
							v->second.begin(), v->second.end());
						ins += v->second.size();
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
	//cout << "expr=" << ::to_string(match.expr) << endl;
	//cout << "after mapping: " << *this << endl;

	if (not match.expr.empty()) {
		erase(match.expr);
		//cout << "after erase: " << *this << endl;
	}
}

Expression &Expression::minimize(Expression directed) {
	static const Expression rules = rewriteBasic();
	if (directed.operations.empty()) {
		directed = rules;
	}

	//cout << "Rules: " << rules << endl;

	tidy();
	vector<Match> tokens = search(directed, 1u);
	while (not tokens.empty()) {
		//cout << "Expr: " << *this << endl;
		//cout << "Match: " << ::to_string(tokens) << endl;
		replace(directed, tokens.back());
		//cout << "Replace: " << *this << endl;
		tidy();
		//cout << "Canon: " << *this << endl << endl;
		tokens = search(directed, 1u);
	}

	// TODO(edward.bingham) Then I need to implement encoding
	// Use the unidirectional expression rewrite system?
	// propagate validity?
	
	return *this;
}*/

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

ostream &operator<<(ostream &os, Expression::Match m) {
	os << m.replace << " expr=" << ::to_string(m.expr) << " top=" << ::to_string(m.top) << " vars=" << ::to_string(m.vars);
	return os;
}

Expression operator~(Expression e) {
	Expression result;
	result.set(Operation::BOOLEAN_NOT, e);
	return result;
}

Expression operator-(Expression e) {
	Expression result;
	result.set(Operation::NEGATION, e);
	return result;
}

Expression ident(Expression e) {
	Expression result;
	result.set(Operation::IDENTITY, e);
	return result;
}

Expression isValid(Expression e) {
	Expression result;
	result.set(Operation::VALIDITY, e);
	return result;
}

Expression isNegative(Expression e) {
	Expression result;
	result.set(Operation::NEGATIVE, e);
	return result;
}

Expression operator!(Expression e) {
	Expression result;
	result.set(Operation::BITWISE_NOT, e);
	return result;
}

Expression inv(Expression e) {
	Expression result;
	result.set(Operation::INVERSE, e);
	return result;
}

Expression operator||(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_NOT, e0, e1);
	return result;
}

Expression operator&&(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator^(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression bitwiseXor(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
	return result;
}

Expression operator==(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator|(Expression e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator|(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression bitwiseXor(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
	return result;
}

Expression operator==(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Expression e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression operator|(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression bitwiseXor(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
	return result;
}

Expression operator==(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Operand e0, Expression e1) {
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression operator|(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0, e1);
	return result;
}

Expression operator&(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator^(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0, e1);
	return result;
}

Expression bitwiseXor(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
	return result;
}

Expression operator==(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::EQUAL, e0, e1);
	return result;
}

Expression operator!=(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::NOT_EQUAL, e0, e1);
	return result;
}

Expression operator<(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::LESS, e0, e1);
	return result;
}

Expression operator>(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::GREATER, e0, e1);
	return result;
}

Expression operator<=(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::LESS_EQUAL, e0, e1);
	return result;
}

Expression operator>=(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::GREATER_EQUAL, e0, e1);
	return result;
}

Expression operator<<(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::SHIFT_LEFT, e0, e1);
	return result;
}

Expression operator>>(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::SHIFT_RIGHT, e0, e1);
	return result;
}

Expression operator+(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::ADD, e0, e1);
	return result;
}

Expression operator-(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::SUBTRACT, e0, e1);
	return result;
}

Expression operator*(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::MULTIPLY, e0, e1);
	return result;
}

Expression operator/(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::DIVIDE, e0, e1);
	return result;
}

Expression operator%(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::MOD, e0, e1);
	return result;
}

Expression operator&&(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0, e1);
	return result;
}

Expression operator||(Operand e0, Operand e1) {
	Expression result;
	result.set(Operation::BITWISE_OR, e0, e1);
	return result;
}

Expression array(vector<Expression> e) {
	Expression result;
	result.set(Operation::ARRAY, e);
	return result;
}

Expression booleanOr(Expression e0) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0);
	return result;
}

Expression booleanAnd(Expression e0) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0);
	return result;
}

Expression booleanXor(Expression e0) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0);
	return result;
}

Expression bitwiseOr(Expression e0) {
	Expression result;
	result.set(Operation::BITWISE_OR, e0);
	return result;
}

Expression bitwiseAnd(Expression e0) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0);
	return result;
}

Expression bitwiseXor(Expression e0) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0);
	return result;
}

Expression add(Expression e0) {
	Expression result;
	result.set(Operation::ADD, e0);
	return result;
}

Expression mult(Expression e0) {
	Expression result;
	result.set(Operation::MULTIPLY, e0);
	return result;
}

Expression booleanOr(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BOOLEAN_OR, e0);
	return result;
}

Expression booleanAnd(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0);
	return result;
}

Expression booleanXor(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BOOLEAN_XOR, e0);
	return result;
}

Expression bitwiseOr(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BITWISE_OR, e0);
	return result;
}

Expression bitwiseAnd(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BITWISE_AND, e0);
	return result;
}

Expression bitwiseXor(vector<Expression> e0) {
	Expression result;
	result.set(Operation::BITWISE_XOR, e0);
	return result;
}

Expression add(vector<Expression> e0) {
	Expression result;
	result.set(Operation::ADD, e0);
	return result;
}

Expression mult(vector<Expression> e0) {
	Expression result;
	result.set(Operation::MULTIPLY, e0);
	return result;
}

Expression call(int func, vector<Expression> args) {
	Expression result;
	args.insert(args.begin(), Operand::typeOf(func));
	result.set(Operation::CALL, args);
	return result;
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
