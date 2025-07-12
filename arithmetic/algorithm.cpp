#include "algorithm.h"

namespace arithmetic {

Value evaluate(ConstOperationSet expr, Operand top, State values) {
	if (not top.isExpr()) {
		return top.get(values, vector<Value>());
	}

	vector<Value> result;
	Value prev;
	for (auto i = ConstUpIterator(expr, top.index); not i.done(); ++i) {
		if (i->exprIndex >= result.size()) {
			result.resize(i->exprIndex+1, Value::X());
		}
		prev = i->evaluate(values, result);
		result[i->exprIndex] = prev;
	}

	return prev;
}

}

