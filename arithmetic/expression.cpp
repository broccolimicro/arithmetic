#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>
#include <common/combinatoric.h>

namespace arithmetic {

UpIterator Expression::exprUp(size_t exprIndex) {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return UpIterator(*this, exprIndex);
}

ConstUpIterator Expression::exprUp(size_t exprIndex) const {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return ConstUpIterator(*this, exprIndex);
}

DownIterator Expression::exprDown(size_t exprIndex) {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return DownIterator(*this, exprIndex);
}

ConstDownIterator Expression::exprDown(size_t exprIndex) const {
	if (exprIndex == std::numeric_limits<size_t>::max() and top.isExpr()) {
		exprIndex = top.index;
	}

	return ConstDownIterator(*this, exprIndex);
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

Operation *Expression::exprAt(size_t index) {
	int i = lookup(index);
	if (i < 0) {
		return nullptr;
	}
	return &operations[i];
}

const Operation *Expression::exprAt(size_t index) const {
	int i = lookup(index);
	if (i < 0) {
		return nullptr;
	}
	return &operations[i];
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
		arg.exprIndex = nextExprIndex++;
		pos = (int)operations.size();
		operations.push_back(arg);
		setExpr(operations.back().exprIndex, operations.size()-1);
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

	// cout << ::to_string(exprMap) << " " << exprMapIsDirty << endl;
	// cout << *this << endl;
	auto curr = exprUp();
	for (; not curr.done(); ++curr) {
		// cout << "start: " << curr.get() << endl;
		curr->replace(replace);
		curr->tidy();
		// cout << "replaced: " << curr.get() << endl;

		if (curr->operands.size() == 1u and curr->operands[0].isConst()) {
			Operand v = Operation::evaluate(curr->func, {curr->operands[0].get()});
			replace[curr->exprIndex] = v;
			// cout << "found const expr[" << curr->exprIndex << "] = " << v << endl;
		}	else if (curr->operands.size() == 1u and (curr->isReflexive()
			or (not rules and curr->isCommutative()))) {
			// replace reflexive expressions
			replace[curr->exprIndex] = curr->operands[0];
			// cout << "found reflex expr[" << curr->exprIndex << "] = " << curr->operands[0] << endl;
		} else {
			// replace identical operations
			for (auto k = operations.begin(); k != operations.end(); k++) {
				if (curr.expand[k->exprIndex] and k->exprIndex != curr->exprIndex and replace[k->exprIndex].isUndef() and areSame(curr.get(), *k)) {
					replace[curr->exprIndex] = Operand::exprOf(k->exprIndex);
					// cout << "found duplicate expr[" << curr->exprIndex << "] = " << Operand::exprOf(k->exprIndex) << endl;
					break;
				}
			}
		}

		// cout << ::to_string(curr.seen) << endl;
		// cout << *this << endl;
	}

	if (top.isExpr() and not replace[top.index].isUndef()) {
		top = replace[top.index];
	}

	for (int i = (int)operations.size()-1; i >= 0; i--) {
		if (not curr.seen[operations[i].exprIndex] or not replace[operations[i].exprIndex].isUndef()) {
			breakMap();
			operations.erase(operations.begin()+i);
		}
	}

	// cout << "done: " << *this << endl;

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

bool Expression::verifyRuleFormat(Operand i, bool msg) const {
	// ==, <, >
	if (not i.isExpr()) {
		if (msg) printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		return false;
	}
	auto j = exprAt(i.index);
	if (j == nullptr
		or j->operands.size() != 2u
		or (j->func != Operation::EQUAL
		and j->func != Operation::LESS
		and j->func != Operation::GREATER)) {
		if (msg) printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		return false;
	}
	return true;
}

bool Expression::verifyRulesFormat(bool msg) const {
	if (not top.isExpr() or operations.empty()) {
		if (msg) printf("error: no replacement rules found\n");
		return false;
	}
	auto ruleTop = exprAt(top.index);
	if (ruleTop->func != Operation::ARRAY) {
		if (msg) printf("error: invalid format for replacement rules\n");
		return false;
	}
	bool result = true;
	for (auto i = ruleTop->operands.begin(); i != ruleTop->operands.end(); i++) {
		result = verifyRuleFormat(*i, msg) and result;
	}
	return result;
}

Operand Expression::extract(size_t exprIndex, vector<size_t> idx) {
	operations.push_back(exprAt(exprIndex)->extract(idx, nextExprIndex++));
	setExpr(operations.back().exprIndex, operations.size()-1);
	return Operand::exprOf(operations.back().exprIndex);
}

Cost Expression::cost(vector<Type> vars) const {
	if (not top.isExpr()) {
		return Cost();
	}

	double complexity = 0.0;
	vector<Type> expr;

	for (auto curr = exprUp(); not curr.done(); ++curr) {
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

// TODO(edward.bingham) I need a way to canonicalize expressions and hash
// them so that I can do the state search algorithm.
//
// TODO(edward.bingham) rules aren't currently able to match with a variable
// number of operands. I need to create a comprehension functionality to
// support those more complex matches.
//
// TODO(edward.bingham) look into "tree automata" and "regular tree grammar"
// as a form of regex for trees instead of sequences.
vector<Expression::Match> Expression::search(const Expression &rules, size_t count, bool fwd, bool bwd) {
	if (not rules.verifyRulesFormat(true)) {
		return vector<Expression::Match>();
	}

	using Leaf = pair<Operand, Operand>;
	vector<pair<vector<Leaf>, Match> > stack;

	// initialize the initial matches
	auto ruleTop = rules.exprAt(rules.top.index);
	for (int i = 0; i < (int)operations.size(); i++) {
		Operand op = Operand::exprOf(operations[i].exprIndex);

		// search through the "rules" rules and add all of the matching starts
		for (auto j = ruleTop->operands.begin(); j != ruleTop->operands.end(); j++) {
			if (not rules.verifyRuleFormat(*j, false)) {
				continue;
			}

			auto rule = rules.exprAt(j->index);
			auto lhs = rule->operands.begin();
			auto rhs = std::next(lhs);
			// map left to right
			if (rule->func == Operation::GREATER or (fwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap({op}, *lhs, *this, rules, true, &match.vars)) {
					match.replace = *rhs;
					leaves.push_back({op, *lhs});
					stack.push_back({leaves, match});
				}
			}

			// map right to left
			if (rule->func == Operation::LESS or (bwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap({op}, *rhs, *this, rules, true, &match.vars)) {
					match.replace = *lhs;
					leaves.push_back({op, *rhs});
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
			auto fOp = exprAt(from.index);
			auto tOp = rules.exprAt(to.index);

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
			reverse(curr.expr.begin(), curr.expr.end());
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

/*size_t Expression::count(Operand start) const {
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
}*/

void Expression::replace(Operand from, Operand to) {
	for (auto i = operations.begin(); i != operations.end(); i++) {
		for (auto j = i->operands.begin(); j != i->operands.end(); j++) {
			if (areSame(*j, from)) {
				*j = to;
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

		auto slotOp = exprAt(slot);
		if (slotOp == nullptr) {
			printf("internal:%s:%d: expression %lu not found\n", __FILE__, __LINE__, slot);
			return;
		}

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
		size_t matchTop = this->top.index;
		if (not match.expr.empty()) {
			matchTop = match.expr.back();
		}
		//cout << "top=" << matchTop << " expr=" << ::to_string(match.expr) << endl;
		if (match.replace.isExpr()
			and rules.exprAt(match.replace.index)->func != exprAt(matchTop)->func
			and match.top.size() < exprAt(matchTop)->operands.size()) {
			// We matched to a commutative operation and we need to collapse the
			// matched operands to the output of our expression.
			match.expr.back() = extract(matchTop, match.top).index;
			match.top.clear();
		}

		//cout << "top=" << matchTop << " expr=" << ::to_string(match.expr) << endl;
		//cout << "after insert: " << *this << endl;

		// Iterate over the replacement expression
		map<size_t, size_t> exprMap;
		for (auto curr = ConstDownIterator(rules, match.replace.index); not curr.done(); ++curr) {
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
			auto slot = exprAt(pos.first->second);

			slot->func = curr->func;
			size_t ins = 0;
			if (match.top.empty()) {
				slot->operands.clear();
			} else {
				// TODO(edward.bingham) this is causing a out of bounds exception --
				//           may have been fixed by clearing match.top on line 845.

				// If this match doesn't cover all operands, we only want to replace
				// the ones that are covered.
				for (int i = (int)match.top.size()-1; i >= 0; i--) {
					slot->operands.erase(slot->operands.begin() + match.top[i]);
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
					
					slot->operands.insert(
						slot->operands.begin()+ins,
						Operand::exprOf(pos.first->second));
					++ins;
				} else if (op->isVar()) {
					auto v = match.vars.find(op->index);
					if (v != match.vars.end()) {
						slot->operands.insert(
							slot->operands.begin()+ins,
							v->second.begin(), v->second.end());
						ins += v->second.size();
					} else {
						printf("variable not mapped\n");
					}
				} else {
					slot->operands.insert(
						slot->operands.begin()+ins,
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

	tidy();
	vector<Match> tokens = search(directed, 1u);
	while (not tokens.empty()) {
		// cout << "Expr: " << *this << endl;
		// cout << "Match: " << ::to_string(tokens) << endl;
		replace(directed, tokens.back());
		// cout << "Replace: " << *this << endl;
		tidy();
		// cout << "Canon: " << *this << endl << endl;
		tokens = search(directed, 1u);
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

ostream &operator<<(ostream &os, Expression::Match m) {
	os << m.replace << " expr=" << ::to_string(m.expr) << " top=" << ::to_string(m.top) << " vars=" << ::to_string(m.vars);
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
