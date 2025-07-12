#include "algorithm.h"
#include <common/text.h>
#include <common/combinatoric.h>

namespace arithmetic {

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
	for (auto i = ConstUpIterator(ops, top.index); not i.done(); ++i) {
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

	for (auto curr = ConstUpIterator(ops, top.index); not curr.done(); ++curr) {
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
	auto j = ops.exprAt(i.index);
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
	auto ruleTop = ops.exprAt(top.index);
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
				if (not areSame(o0[i], ins.first->second[i])) {
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
			auto op0 = e0.exprAt(i->index);
			auto op1 = e1.exprAt(o1.index);
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


// TODO(edward.bingham) I need a way to canonicalize expressions and hash
// them so that I can do the state search algorithm.
//
// TODO(edward.bingham) rules aren't currently able to match with a variable
// number of operands. I need to create a comprehension functionality to
// support those more complex matches.
//
// TODO(edward.bingham) look into "tree automata" and "regular tree grammar"
// as a form of regex for trees instead of sequences.
vector<Match> search(ConstOperationSet ops, ConstOperationSet rules, Operand top, size_t count, bool fwd, bool bwd) {
	if (not verifyRulesFormat(rules, top, true)) {
		return vector<Match>();
	}

	using Leaf = pair<Operand, Operand>;
	vector<pair<vector<Leaf>, Match> > stack;

	// initialize the initial matches
	auto ruleTop = rules.exprAt(top.index);
	vector<Operand> indices = ops.exprIndex();
	for (auto i = indices.begin(); i != indices.end(); i++) {
		// search through the "rules" rules and add all of the matching starts
		for (auto j = ruleTop->operands.begin(); j != ruleTop->operands.end(); j++) {
			if (not verifyRuleFormat(rules, *j, false)) {
				continue;
			}

			auto rule = rules.exprAt(j->index);
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
			auto fOp = ops.exprAt(from.index);
			auto tOp = rules.exprAt(to.index);

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


}

