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
		result.push_back(i->op());
	}
	return result;
}

const Operation *SimpleOperationSet::getExpr(size_t index) const {
	if (not elems.is_valid(index)) {
		return nullptr;
	}
	return &elems[index];
}

bool SimpleOperationSet::setExpr(Operation o) {
	elems.emplace_at(o.exprIndex, o);
	return true;
}

Operand SimpleOperationSet::pushExpr(Operation o) {
	o.exprIndex = elems.next_index();
	return Operand::exprOf(elems.insert(o));
}

bool SimpleOperationSet::eraseExpr(size_t index) {
	return elems.erase(index);
}

void SimpleOperationSet::clear() {
	elems.clear();
}

size_t SimpleOperationSet::size() const {
	return elems.size();
}

string SimpleOperationSet::to_string() const {
	ostringstream oss;
	for (auto i = elems.rbegin(); i != elems.rend(); i++) {
		oss << i.index << ": " << *i << endl;
	}
	return oss.str();
}

ostream &operator<<(ostream &os, SimpleOperationSet e) {
	os << e.to_string();
	return os;
}

}

