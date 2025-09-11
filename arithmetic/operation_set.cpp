#include "operation_set.h"
#include "algorithm.h"
#include <common/text.h>

namespace arithmetic {

SimpleOperationSet::SimpleOperationSet() {
	Operation::loadOperators();
}

SimpleOperationSet::~SimpleOperationSet() {
}

vector<Operand> SimpleOperationSet::exprIndex() const {
	vector<Operand> result;
	result.reserve(elems.count());
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

Mapping SimpleOperationSet::append(ConstOperationSet arg, vector<Operand> top) {
	Mapping m;
	for (ConstUpIterator i(arg, top); not i.done(); ++i) {
		m.set(i->op(), pushExpr(Operation(*i).apply(m)));
	}
	return m;
}

void SimpleOperationSet::clear() {
	elems.clear();
}

size_t SimpleOperationSet::size() const {
	return elems.count();
}

string SimpleOperationSet::to_string() const {
	ostringstream oss;
	for (auto i = elems.rbegin(); i != elems.rend(); i++) {
		oss << *i << endl;
	}
	return oss.str();
}

ostream &operator<<(ostream &os, ConstOperationSet e) {
	vector<Operand> operands = e.exprIndex();
	for (auto i = operands.begin(); i != operands.end(); i++) {
		os << *e.getExpr(i->index) << endl;
	}
	return os;
}

}

