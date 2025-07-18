#pragma once

#include <common/standard.h>
#include <common/index_vector.h>

#include "operation.h"

namespace arithmetic {

_INTERFACE_ARG(OperationSet,
	(vector<Operand>, exprIndex, () const, ()),
	(const Operation *, getExpr, (size_t index) const, (index)),
	(bool, setExpr, (Operation o), (o)),
	(Operand, pushExpr, (Operation o), (o)),
	(bool, eraseExpr, (size_t index), (index)));

_CONST_INTERFACE_ARG(ConstOperationSet,
	(vector<Operand>, exprIndex, () const, ()),
	(const Operation *, getExpr, (size_t index) const, (index)));

struct SimpleOperationSet {
	SimpleOperationSet();
	~SimpleOperationSet();

	index_vector<Operation> elems;

	vector<Operand> exprIndex() const;
	const Operation *getExpr(size_t index) const;
	bool setExpr(Operation o);
	Operand pushExpr(Operation o);
	bool eraseExpr(size_t index);

	void clear();
	size_t size() const;

	string to_string() const;
};

ostream &operator<<(ostream &os, SimpleOperationSet e);

}
