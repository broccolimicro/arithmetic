#include "algorithm.h"
#include "rewrite.h"

#include <common/text.h>
#include <common/combinatoric.h>

namespace arithmetic {

UpIterator::UpIterator(OperationSet root, vector<Operand> start) : root(root) {
	for (auto i = start.begin(); i != start.end(); i++) {
		if (i->isExpr()) {
			stack.push_back(i->index);
		}
	}
	++*this;
}

UpIterator::~UpIterator() {
}

const Operation &UpIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &UpIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *UpIterator::operator->() {
	return root.getExpr(stack.back());
}

// Depth first from leaves to root
UpIterator &UpIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= seen.size()) {
		seen.resize(stack.back()+1, false);
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root.getExpr(stack.back());
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}
				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	return *this;
}

bool UpIterator::done() const {
	return stack.empty();
}

bool operator==(const UpIterator &i0, const UpIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const UpIterator &i0, const UpIterator &i1) {
	return i0.stack != i1.stack;
}

ConstUpIterator::ConstUpIterator(ConstOperationSet root, vector<Operand> start) : root(root) {
	for (auto i = start.begin(); i != start.end(); i++) {
		if (i->isExpr()) {
			stack.push_back(i->index);
		}
	}
	++*this;
}

ConstUpIterator::~ConstUpIterator() {
}

const Operation &ConstUpIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &ConstUpIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *ConstUpIterator::operator->() {
	return root.getExpr(stack.back());
}

ConstUpIterator &ConstUpIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= seen.size()) {
		seen.resize(stack.back()+1, false);
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root.getExpr(stack.back());
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}
				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	return *this;
}

bool ConstUpIterator::done() const {
	return stack.empty();
}

bool operator==(const ConstUpIterator &i0, const ConstUpIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstUpIterator &i0, const ConstUpIterator &i1) {
	return i0.stack != i1.stack;
}

DownIterator::DownIterator(OperationSet root, vector<Operand> start) : root(root) {
	for (auto i = start.begin(); i != start.end(); i++) {
		if (i->isExpr()) {
			stack.push_back(i->index);
		}
	}
	++*this;
}

DownIterator::~DownIterator() {
}

const Operation &DownIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &DownIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *DownIterator::operator->() {
	return root.getExpr(stack.back());
}

DownIterator &DownIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.getExpr(stack.back());
		stack.pop_back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}

				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	if (not stack.empty()) {
		if (stack.back() >= seen.size()) {
			seen.resize(stack.back()+1, false);
		}

		seen[stack.back()] = true;

		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}

		expand[stack.back()] = true;
	}
	return *this;
}

bool DownIterator::done() const {
	return stack.empty();
}

bool operator==(const DownIterator &i0, const DownIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const DownIterator &i0, const DownIterator &i1) {
	return i0.stack != i1.stack;
}

ConstDownIterator::ConstDownIterator(ConstOperationSet root, vector<Operand> start) : root(root) {
	for (auto i = start.begin(); i != start.end(); i++) {
		if (i->isExpr()) {
			stack.push_back(i->index);
		}
	}
	++*this;
}

ConstDownIterator::~ConstDownIterator() {
}

const Operation &ConstDownIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &ConstDownIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *ConstDownIterator::operator->() {
	return root.getExpr(stack.back());
}

ConstDownIterator &ConstDownIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.getExpr(stack.back());
		stack.pop_back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}

				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	if (not stack.empty()) {
		if (stack.back() >= seen.size()) {
			seen.resize(stack.back()+1, false);
		}

		seen[stack.back()] = true;

		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}

		expand[stack.back()] = true;
	}
	return *this;
}

bool ConstDownIterator::done() const {
	return stack.empty();
}

bool operator==(const ConstDownIterator &i0, const ConstDownIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstDownIterator &i0, const ConstDownIterator &i1) {
	return i0.stack != i1.stack;
}

ostream &operator<<(ostream &os, Match m) {
	os << m.replace << " expr=" << ::to_string(m.expr) << " top=" << ::to_string(m.top) << " vars=" << ::to_string(m.vars);
	return os;
}

Value evaluate(ConstOperationSet ops, Operand top, State values) {
	if (not top.isExpr()) {
		return top.get(values, vector<Value>());
	}

	vector<Value> result;
	Value prev;
	for (auto i = ConstUpIterator(ops, {top}); not i.done(); ++i) {
		if (i->exprIndex >= result.size()) {
			result.resize(i->exprIndex+1, Value::X());
		}
		prev = i->evaluate(values, result);
		result[i->exprIndex] = prev;
	}

	return prev;
}

Cost cost(ConstOperationSet ops, Operand top, vector<Type> vars) {
	if (not top.isExpr()) {
		return Cost();
	}

	double complexity = 0.0;
	vector<Type> expr;

	for (auto curr = ConstUpIterator(ops, {top}); not curr.done(); ++curr) {
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

bool verifyRuleFormat(ConstOperationSet ops, Operand i, bool msg) {
	// ==, <, >
	if (not i.isExpr()) {
		if (msg) printf("internal:%s:%d: invalid format for rule\n", __FILE__, __LINE__);
		return false;
	}
	auto j = ops.getExpr(i.index);
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

bool verifyRulesFormat(ConstOperationSet ops, Operand top, bool msg) {
	auto ruleTop = ops.getExpr(top.index);
	if (not top.isExpr() or ruleTop == nullptr) {
		if (msg) printf("error: no replacement rules found\n");
		return false;
	}
	if (ruleTop->func != Operation::ARRAY) {
		if (msg) printf("error: invalid format for replacement rules\n");
		return false;
	}
	bool result = true;
	for (auto i = ruleTop->operands.begin(); i != ruleTop->operands.end(); i++) {
		result = verifyRuleFormat(ops, *i, msg) and result;
	}
	return result;
}

bool canMap(vector<Operand> o0, Operand o1, ConstOperationSet e0, ConstOperationSet e1, bool init, map<size_t, vector<Operand> > *vars) {
	if (o1.isConst()) {
		for (auto i = o0.begin(); i != o0.end(); i++) {
			if (not i->isConst() or not (areSame(i->cnst, o1.cnst)
				or (o1.cnst.type == Value::BOOL
					and ((o1.cnst.bval == Value::VALID and i->cnst.isValid())
						or (o1.cnst.bval == Value::NEUTRAL and i->cnst.isNeutral()))))) {
				return false;
			}
		}
		return true;
	} else if (o1.isVar()) {
		if (vars != nullptr) {
			auto ins = vars->insert({o1.index, o0});
			if (ins.second) {
				return true;
			} else if (o0.size() != ins.first->second.size()) {
				return false;
			}
			for (int i = 0; i < (int)o0.size(); i++) {
				if (o0[i] != ins.first->second[i]) {
					return false;
				}
			}
			return true;
		}
		return true;
	} else if (o1.isExpr()) {
		for (auto i = o0.begin(); i != o0.end(); i++) {
			if (not i->isExpr()) {
				return false;
			}
			auto op0 = e0.getExpr(i->index);
			auto op1 = e1.getExpr(o1.index);
			if (not (op0->func == op1->func
				and (op0->operands.size() == op1->operands.size()
					or ((op1->isCommutative() or init)
						and op0->operands.size() > op1->operands.size())))) {
				return false;
			}
		}
		return true;
	}
	return false;
}

Operand extract(OperationSet expr, size_t from, vector<size_t> operands) {
	Operation o0 = *expr.getExpr(from);
	Operand result = expr.pushExpr(Operation());
	expr.setExpr(o0.extract(operands, result.index));
	expr.setExpr(o0);
	return result;
}

Expression subExpr(ConstOperationSet e0, Operand top) {
	Expression result;
	Mapping m;
	for (ConstUpIterator i(e0, {top}); not i.done(); ++i) {
		m.set(i->op(), result.pushExpr(Operation(*i).apply(m)));
	}
	result.top = m.map(top);
	return result;
}

// tidy() does a few things:
// 1. propagate constants
// 2. remove reflexive operations
// 3. remove unitary commutative operations (if not rules)
// 4. remove dangling operations
// 5. merge successive commutative operations
// 6. sort operands into a canonical order for commutative operations
Mapping tidy(OperationSet expr, vector<Operand> top, bool rules) {
	// Start from the top and do depth first search. That zips up the graph for
	// us. First, we need a mapping of index in operations to exprIndex so we
	// don't have to search for the exprIndex each time.

	// DESIGN(edward.bingham) This algorithm assumes that there are no cycles in
	// the graph. So, a directed acyclic graph. If there are cycles, this
	// function will fail silently and produce a result that doesn't sufficiently
	// propagate all constants. The end result should still be functionally
	// equivalent.

	Mapping result;
	vector<size_t> keep;
	vector<size_t> refcount;

	// cout << ::to_string(exprMap) << " " << exprMapIsDirty << endl;
	// cout << *this << endl;
	auto currIter = UpIterator(expr, top);
	for (; not currIter.done(); ++currIter) {
		Operation curr = *currIter;
		// cout << "start: " << curr << endl;
		curr.apply(result);

		bool squish = false;
		for (int i = (int)curr.operands.size()-1; i >= 0; i--) {
			Operand op = curr.operands[i];
			if (op.isExpr()) {
				auto opExpr = expr.getExpr(op.index);
				if (curr.isCommutative() and opExpr->func == curr.func) {
					curr.operands.erase(curr.operands.begin() + i);
					curr.operands.insert(curr.operands.begin() + i, opExpr->operands.begin(), opExpr->operands.end());
					squish = true;
				} else {
					if (op.index >= refcount.size()) {
						refcount.resize(op.index+1, 0u);
					}
					refcount[op.index]++;
				}
			}
		}
		// cout << "squish: " << curr << endl;

		curr.tidy();
		// cout << "tidy: " << curr << endl;
		if (squish) {
			// cout << "setting expr" << endl;
			expr.setExpr(curr);
		}
		// cout << "replaced: " << curr << endl;

		if (curr.operands.size() == 1u and curr.operands[0].isConst()) {
			// cout << "found const " << curr.op() << " = " << curr << endl;
			result.set(curr.op(), Operation::evaluate(curr.func, {curr.operands[0].get()}));
		}	else if (curr.operands.size() == 1u and (curr.isReflexive()
			or (not rules and curr.isCommutative()))) {
			// replace reflexive expressions
			// cout << "found reflex " << curr.op() << " = " << curr.operands[0] << endl;
			result.set(curr.op(), curr.operands[0]);
		} else {
			// replace identical operations
			for (auto k = keep.begin(); k != keep.end(); k++) {
				if (curr == *expr.getExpr(*k)) {
					// cout << "found duplicate " << curr.op() << " = " << Operand::exprOf(*k) << endl;
					result.set(curr.op(), Operand::exprOf(*k));
					break;
				}
			}
		}

		if (not result.has(curr.op())) {
			expr.setExpr(curr);
			keep.push_back(curr.exprIndex);
		}
	}

	top = result.map(top);
	for (auto i = top.begin(); i != top.end(); i++) {
		if (i->isExpr()) {
			if (i->index >= refcount.size()) {
				refcount.resize(i->index+1, 0u);
			}
			refcount[i->index]++;
		}
	}

	// TODO(edward.bingham) This ends up being an N^2 algorithm because each
	// erase invalidates the expr mapping... This needs to be a bit more elegant.
	vector<Operand> index = expr.exprIndex();
	for (auto i = index.begin(); i != index.end(); i++) {
		if (result.has(*i)) {
			expr.eraseExpr(i->index);
		} else if (i->index >= currIter.seen.size() or not currIter.seen[i->index] or i->index >= refcount.size() or refcount[i->index] == 0u) {
			expr.eraseExpr(i->index);
			result.set(*i, Operand::undef());
		}
	}

	// cout << "done: " << *this << endl;

	return result;

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

// TODO(edward.bingham) I need a way to canonicalize expressions and hash
// them so that I can do the state search algorithm.
//
// TODO(edward.bingham) rules aren't currently able to match with a variable
// number of operands. I need to create a comprehension functionality to
// support those more complex matches.
//
// TODO(edward.bingham) look into "tree automata" and "regular tree grammar"
// as a form of regex for trees instead of sequences.

// pin - these expression IDs cannot be contained in a match except at the very
// top of the match. These must be preserved through a replace.
vector<Match> search(ConstOperationSet ops, vector<Operand> pin, const Expression &rules, size_t count, bool fwd, bool bwd) {
	if (not verifyRulesFormat(rules, rules.top, true)) {
		//cout << rules << endl;
		return vector<Match>();
	}

	using Leaf = pair<Operand, Operand>;
	vector<pair<vector<Leaf>, Match> > stack;

	// initialize the initial matches
	auto ruleTop = rules.getExpr(rules.top.index);
	vector<Operand> indices = ops.exprIndex();
	for (auto i = indices.begin(); i != indices.end(); i++) {
		// search through the "rules" rules and add all of the matching starts
		for (auto j = ruleTop->operands.begin(); j != ruleTop->operands.end(); j++) {
			if (not verifyRuleFormat(rules, *j, false)) {
				continue;
			}

			auto rule = rules.getExpr(j->index);
			auto lhs = rule->operands.begin();
			auto rhs = std::next(lhs);
			// map left to right
			if (rule->func == Operation::GREATER or (fwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap({*i}, *lhs, ops, rules, true, &match.vars)) {
					match.replace = *rhs;
					leaves.push_back({*i, *lhs});
					stack.push_back({leaves, match});
				}
			}

			// map right to left
			if (rule->func == Operation::LESS or (bwd and rule->func == Operation::EQUAL)) {
				Match match;
				vector<Leaf> leaves;
				if (canMap({*i}, *rhs, ops, rules, true, &match.vars)) {
					match.replace = *lhs;
					leaves.push_back({*i, *rhs});
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
			auto fOp = ops.getExpr(from.index);
			auto tOp = rules.getExpr(to.index);

			bool foundPin = false;
			for (auto i = fOp->operands.begin(); i != fOp->operands.end(); i++) {
				if (i->isExpr() and find(pin.begin(), pin.end(), *i) != pin.end()) {
					foundPin = true;
					break;
				}
			}
			if (foundPin) {
				continue;
			}

			bool commute = tOp->isCommutative();
			if (commute and tOp->operands.size() == 1u) {
				//cout << "Elastic Commutative" << endl;
				Match nextMatch = curr;
				vector<Leaf> nextLeaves = leaves;
				if (canMap(fOp->operands, tOp->operands[0], ops, rules, false, &nextMatch.vars)) {
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
						found = canMap({fOp->operands[*i]}, tOp->operands[i-it.begin()], ops, rules, false, &nextMatch.vars);
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

Mapping replace(OperationSet expr, const Expression &rules, Match match) {
	Mapping result;
	for (int i = 0; i < ((int)match.expr.size())-1; i++) {
		result.set(Operand::exprOf(match.expr[i]), Operand::undef());
	}

	if (not match.replace.isExpr()) {
		size_t ins = 0;
		size_t slotIndex = match.expr.back();
		match.expr.pop_back();

		auto slotPtr = expr.getExpr(slotIndex);
		if (slotPtr == nullptr) {
			printf("internal:%s:%d: expression %lu not found\n", __FILE__, __LINE__, slotIndex);
			return result;
		}
		Operation slot = *slotPtr;

		if (match.top.empty()) {
			slot.operands.clear();
			slot.func = Operation::OpType::TYPE_UNDEF;
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

		expr.setExpr(slot);
	} else if (not match.expr.empty()) {
		//cout << "top=" << match.expr.back() << " expr=" << ::to_string(match.expr) << endl;
		if (match.replace.isExpr()
			and rules.getExpr(match.replace.index)->func != expr.getExpr(match.expr.back())->func
			and match.top.size() < expr.getExpr(match.expr.back())->operands.size()) {
			// We matched to a commutative operation and we need to collapse the
			// matched operands to the output of our expression.
			match.expr.back() = extract(expr, match.expr.back(), match.top).index;
			match.top.clear();
		}

		//cout << "top=" << match.expr.back() << " expr=" << ::to_string(match.expr) << endl;
		//cout << "after insert: " << *this << endl;

		// Iterate over the replacement expression
		map<size_t, size_t> exprMap;
		for (auto curr = ConstDownIterator(rules, {match.replace}); not curr.done(); ++curr) {
			// Along the way, compute the exprIndex mapping
			auto pos = exprMap.insert({curr->exprIndex, 0});
			if (pos.second) {
				if (match.expr.empty()) {
					pos.first->second = expr.pushExpr(Operation()).index;
				} else {
					pos.first->second = match.expr.back();
					match.expr.pop_back();
				}
			}
			auto slotPtr = expr.getExpr(pos.first->second);
			if (slotPtr == nullptr) {
				printf("internal:%s:%d: expression %lu not found\n", __FILE__, __LINE__, pos.first->second);
				return result;
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
							pos.first->second = expr.pushExpr(Operation()).index;
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
			expr.setExpr(slot);
		}
	}
	//cout << "expr=" << ::to_string(match.expr) << endl;
	//cout << "after mapping: " << *this << endl;

	for (auto i = match.expr.begin(); i != match.expr.end(); i++) {
		expr.eraseExpr(*i);
	}

	return result;
	// cout << "after erase: " << *this << endl;
}

Mapping minimize(OperationSet expr, vector<Operand> top, Expression rules) {
	static const Expression defaultRules = rewriteBasic();
	if (rules.top.isUndef()) {
		rules = defaultRules;
	}

	// cout << "Rules: " << rules << endl;

	Mapping result;
	result.apply(tidy(expr, top));
	top = result.map(top);
	vector<Match> tokens = search(expr, top, rules, 1u);
	while (not tokens.empty()) {
		// cout << "Expr: " << ::to_string(top) << " " << expr.cast<Expression>() << endl;
		// cout << "Match: " << ::to_string(tokens) << endl;
		Mapping sub = replace(expr, rules, tokens.back());
		// cout << "Replace: " << expr.cast<Expression>() << endl;
		sub.apply(tidy(expr, top));
		top = sub.map(top);
		result.apply(sub);
		// cout << "Canon: " << ::to_string(top) << " " << expr.cast<Expression>() << endl << endl;
		tokens = search(expr, top, rules, 1u);
	}

	// TODO(edward.bingham) Then I need to implement encoding
	// Use the unidirectional expression rewrite system?
	// propagate validity?
	
	return result;
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

}

