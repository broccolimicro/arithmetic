#include "operation_set.h"

namespace arithmetic {

SimpleOperationSet::SimpleOperationSet() {
	Operation::loadOperators();
}

SimpleOperationSet::~SimpleOperationSet() {
}

vector<Operand> SimpleOperationSet::exprIndex() const {
	vector<Operand> result;
	result.reserve(elems.size());
	for (auto i = elems.begin(); i != elems.end(); i++) {
		if (not i->isUndef()) {
			result.push_back(i->op());
		}
	}
	return result;
}

const Operation *SimpleOperationSet::getExpr(size_t index) const {
	if (index < elems.size() and not elems[index].isUndef()) {
		return &elems[index];
	}
	return nullptr;
}

bool SimpleOperationSet::setExpr(Operation o) {
	if (o.exprIndex >= elems.size()) {
		for (size_t i = elems.size(); i < o.exprIndex; i++) {
			free.push_back(i);
		}
		elems.resize(o.exprIndex+1, Operation::undef());
	}
	elems[o.exprIndex] = o;
	return true;
}

Operand SimpleOperationSet::pushExpr(Operation o) {
	if (free.empty()) {
		o.exprIndex = elems.size();
		elems.push_back(o);
	} else {
		o.exprIndex = free.back();
		free.pop_back();
		elems[o.exprIndex] = o;
	}
	return o.op();
}

bool SimpleOperationSet::eraseExpr(size_t index) {
	if (index < elems.size() and not elems[index].isUndef()) {
		elems[index] = Operation::undef();
		free.push_back(index);
		return true;
	}
	return false;
}

void SimpleOperationSet::clear() {
	elems.clear();
	free.clear();
}

size_t SimpleOperationSet::size() const {
	return elems.size() - free.size();
}

string SimpleOperationSet::to_string() const {
	ostringstream oss;
	for (int i = (int)elems.size()-1; i >= 0; i--) {
		if (not elems[i].isUndef()) {
			oss << i << ": " << elems[i] << endl;
		}
	}
	return oss.str();
}

}

