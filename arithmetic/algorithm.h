#pragma once

#include "state.h"

namespace arithmetic {

template <typename T>
Value evaluate(T expr, State values) {
	vector<Value> result;
	Value prev;
	for (auto i = expr.begin(); i != expr.end(); ++i) {
		if (i->exprIndex >= result.size()) {
			result.resize(i->exprIndex+1, Value::X());
		}
		prev = i->evaluate(values, result);
		result[i->exprIndex] = prev;
	}

	return prev;
}

}

