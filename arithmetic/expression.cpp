#include "expression.h"
#include "state.h"
#include "rewrite.h"

#include <sstream>
#include <array>
#include <common/standard.h>
#include <common/text.h>

namespace arithmetic
{

Expression::Expression() {
	Operation::loadOperators();
}

Expression::Expression(Operand arg0) {
	Operation::loadOperators();
	set(arg0);
}

Expression::Expression(int func, Operand arg0) {
	Operation::loadOperators();
	set(func, arg0);
}

Expression::Expression(int func, Expression arg0) {
	Operation::loadOperators();
	set(func, arg0);
}

Expression::Expression(int func, Operand arg0, Operand arg1) {
	Operation::loadOperators();
	set(func, arg0, arg1);
}

Expression::Expression(int func, Expression arg0, Operand arg1) {
	Operation::loadOperators();
	set(func, arg0, arg1);
}

Expression::Expression(int func, Operand arg0, Expression arg1) {
	Operation::loadOperators();
	set(func, arg0, arg1);
}

Expression::Expression(int func, Expression arg0, Expression arg1) {
	Operation::loadOperators();
	set(func, arg0, arg1);
}

Expression::Expression(int func, vector<Expression> args) {
	Operation::loadOperators();
	set(func, args);
}

Expression::~Expression() {
}

Operation *Expression::operationAt(size_t index) {
	return &operations[index];
}

const Operation *Expression::operationAt(size_t index) const {
	return &operations[index];
}

Operand Expression::popReflexive(size_t index) {
	if (index >= operations.size()) {
		index = operations.size()-1;
	}
	Operand op = Operand::exprOf(index);
	while (op.isExpr() and operations[op.index].isReflexive()) {
		size_t i = op.index;
		op = operations[i].operands[0];
		erase(i);
	}
	return op; 
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
		pos = operations.size();
		operations.push_back(arg);
	}
	return Operand::exprOf(pos);
}

Operand Expression::push(Expression arg) {
	size_t offset = operations.size();
	for (auto i = arg.operations.begin(); i != arg.operations.end(); i++) {
		operations.push_back(i->offsetExpr(offset));
	}
	return popReflexive();
}

Operand Expression::set(Operand arg0) {
	operations.clear();
	operations.push_back(Operation(Operation::IDENTITY, {arg0}));
	return Operand::exprOf(operations.size()-1);
}

Operand Expression::set(int func, Operand arg0) {
	operations.clear();
	operations.push_back(Operation(func, {arg0}));
	return Operand::exprOf(operations.size()-1);
}

Operand Expression::set(int func, Expression arg0) {
	operations.clear();
	Operand op0 = push(arg0);
	return push(Operation(func, {op0}));
}

Operand Expression::set(int func, Operand arg0, Operand arg1) {
	operations.clear();
	operations.push_back(Operation(func, {arg0, arg1}));
	return Operand::exprOf(operations.size()-1);
}

Operand Expression::set(int func, Expression arg0, Expression arg1) {
	operations.clear();
	Operand op0 = push(arg0);
	Operand op1 = push(arg1);
	return push(Operation(func, {op0, op1}));
}

Operand Expression::set(int func, Expression arg0, Operand arg1) {
	operations = arg0.operations;
	Operand op0 = popReflexive();
	return push(Operation(func, {op0, arg1}));
}

Operand Expression::set(int func, Operand arg0, Expression arg1) {
	operations = arg1.operations;
	Operand op1 = popReflexive();
	return push(Operation(func, {arg0, op1}));
}

Operand Expression::set(int func, vector<Expression> args) {
	operations.clear();
	vector<Operand> operands;
	for (auto i = args.begin(); i != args.end(); i++) {
		operands.push_back(push(*i));
	}
	return push(Operation(func, operands));
}

Operand Expression::push(int func) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	return push(Operation(func, {op0}));
}

Operand Expression::push(int func, Operand arg0) {
	Operand op0 = Operand::exprOf(operations.size()-1);
	return push(Operation(func, {op0, arg0}));
}

Operand Expression::push(int func, Expression arg0) {
	Operand op0 = popReflexive();
	Operand op1 = push(arg0);
	return push(Operation(func, {op0, op1}));
}

Value Expression::evaluate(State values) const {
	// I need a value per iterator combination.
	vector<Value> expressions;
	for (int i = 0; i < (int)operations.size(); i++) {
		expressions.push_back(operations[i].evaluate(values, expressions));
	}

	if (expressions.size() > 0)
		return expressions.back();
	else
		return Value();
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

	for (int i = 0; i < (int)operations.size(); i++) {
		operations[i].apply(uidMap);
	}
}

void Expression::apply(vector<Expression> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	vector<Operand> top;
	int offset = 0;

	vector<Operation> result;
	for (int i = 0; i < (int)uidMap.size(); i++) {
		if (uidMap[i].operations.empty()) {
			top.push_back(false);
		} else {
			for (int j = 0; j < (int)uidMap[i].operations.size(); j++) {
				Operation op = uidMap[i].operations[j];
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
	operations.resize(operations.size()+num);
	for (int i = (int)operations.size()-1; i >= (int)(index+num); i--) {
		std::swap(operations[i], operations[i-num]);
		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
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

Expression &Expression::eraseDangling() {
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

Expression &Expression::propagateConstants() {
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

Expression &Expression::canonicalize(bool rules) {
	vector<Operand> replace(operations.size(), Operand::undef());
	
	// propagate constants up through the precedence levels
	for (int i = 0; i < (int)operations.size(); i++) {
		if (operations[i].isCommutative()) {
			// Move constants to the left hand side for evaluation
			// Order variables by index
			sort(operations[i].operands.begin(), operations[i].operands.end(),
				[](const Operand &a, const Operand &b) {
					return a.type < b.type or (a.type == b.type
						and a.isVar() and a.index < b.index);
				});
		}

		// merge adjacent constants from left to right as a result of operator precedence
		auto j = operations[i].operands.begin();
		if (j->isExpr() and not replace[j->index].isUndef()) {
			*j = replace[j->index];
		}

		while (j != operations[i].operands.end() and next(j) != operations[i].operands.end()) {
			auto k = next(j);
			if (k->isExpr() and not replace[k->index].isUndef()) {
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
			// evaluate unary operations against constants
			replace[i] = Operation::evaluate(operations[i].func, {operations[i].operands[0].get()});
		} if (operations[i].operands.size() == 1u and ((not rules and operations[i].isCommutative()) or operations[i].isReflexive())) {
			// replace reflexive expressions
			replace[i] = operations[i].operands[0];
		} else {
			// look for identical operations
			for (int k = i-1; k >= 0; k--) {
				if (replace[k].isUndef() and areSame(operations[i], operations[k])) {
					replace[i] = Operand::exprOf(k);
					break;
				}
			}
		}
	}

	// eliminate dangling expressions
	if (replace.back().isConst() or replace.back().isVar()) {
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
		if (not replace[i].isUndef() and i != (int)operations.size()-1) {
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
		if (not replace[i].isUndef()) {
			erase(i);
		}
	}

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
		return indices.begin()+k;
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
	double complexity = 0.0;
	vector<Type> expr;
	for (int i = 0; i < (int)operations.size(); i++) {
		pair<Type, double> result(Type(0.0, 0.0, 0.0), 0.0);
		vector<Type> args;
		for (auto j = operations[i].operands.begin(); j != operations[i].operands.end(); j++) {
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
		result = operations[i].funcCost(operations[i].func, args);
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
			auto fOp = operations.begin() + from.index;
			auto tOp = rules.operations.begin() + to.index;

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
				CombinationIterator it((int)fOp->operands.size(), (int)tOp->operands.size());
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
					v->second.begin(), v->second.end());
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
		//cout << "top=" << top << " expr=" << ::to_string(match.expr) << endl;
		
		if (match.replace.isExpr() and rules.operations[match.replace.index].func != operations[top].func and match.top.size() < operations[top].operands.size()) {
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
							v->second.begin(), v->second.end());
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

	canonicalize();
	vector<Match> tokens = search(directed, 1u);
	while (not tokens.empty()) {
		//cout << "Expr: " << *this << endl;
		//cout << "Match: " << ::to_string(tokens) << endl;
		replace(directed, tokens.back());
		//cout << "Replace: " << *this << endl;
		canonicalize();
		//cout << "Canon: " << *this << endl << endl;
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

	//Expression result;
	//Cost best(1e20, 1e20);
	expr.minimize(directed);

	/*vector<pair<Cost, Expression> > stack;
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
	}*/

	return expr;
}

Expression &Expression::operator=(Operand e)
{
	set(Operation::IDENTITY, e);
	return *this;
}

string Expression::to_string() const {
	if (operations.empty()) {
		return "(gnd)";
	}

	ostringstream oss;
	for (int i = (int)operations.size()-1; i > 0; i--) {
		oss << i << ": " << operations[i] << endl;
	}
	oss << 0 << ": " << operations[0];
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

Expression operator~(Expression e)
{
	/*if (e.isValid()) {
		return Operand(false);
	} else if (e.isNeutral()) {
		return Operand(true);
	}*/
	Expression result;
	result.set(Operation::BOOLEAN_NOT, e);
	return result;
}

Expression operator-(Expression e)
{
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
	/*if (e.isValid()) {
		return Operand(true);
	} else if (e.isNeutral()) {
		return Operand(false);
	} else if (e.isWire()) {
		return e;
	}*/
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

Expression bitwiseXor(Expression e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
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
	/*if (e0.isNull() or e1.isNull()) {
		return Expression(Value::X());
	} else if (e0.isNeutral() or e1.isNeutral()) {
		return Operand(false);
	} else if (e0.isValid()) {
		return isValid(e1);
	} else if (e1.isValid()) {
		return isValid(e0);
	}*/
	Expression result;
	result.set(Operation::BOOLEAN_AND, e0, e1);
	return result;
}

Expression operator|(Expression e0, Expression e1)
{
	/*if (e0.isNull() or e1.isNull()) {
		return Expression(Value::X());
	} if (e0.isValid() or e1.isValid()) {
		return Operand(true);
	} else if (e0.isNeutral()) {
		return isValid(e1);
	} else if (e1.isNeutral()) {
		return isValid(e0);
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

Expression bitwiseXor(Expression e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
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

Expression bitwiseXor(Operand e0, Expression e1)
{
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
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

Expression bitwiseXor(Operand e0, Operand e1)
{
	Expression result;
	result.set(Operation::BITWISE_XOR, e0, e1);
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
