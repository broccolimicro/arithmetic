#pragma once

#include <common/standard.h>
#include <common/index_vector.h>

#include "operation.h"

namespace arithmetic {

_CONST_INTERFACE_ARG(ConstOperationSet,
	(vector<Operand>, exprIndex, () const, ()),
	(const Operation *, getExpr, (size_t index) const, (index)));

_INTERFACE_ARG(OperationSet,
	(vector<Operand>, exprIndex, () const, ()),
	(const Operation *, getExpr, (size_t index) const, (index)),
	(bool, setExpr, (Operation o), (o)),
	(Operand, pushExpr, (Operation o), (o)),
	(bool, eraseExpr, (size_t index), (index)),
	(Mapping<size_t>, appendExpr, (ConstOperationSet expr), (expr)));

using Minimizer = std::function<Mapping<Operand>(OperationSet,vector<Operand>)>;

struct SimpleOperationSet {
	SimpleOperationSet();
	~SimpleOperationSet();

	index_vector<Operation> elems;

	vector<Operand> exprIndex() const;
	const Operation *getExpr(size_t index) const;
	bool setExpr(Operation o);
	Operand pushExpr(Operation o);
	bool eraseExpr(size_t index);

	Mapping<size_t> appendExpr(ConstOperationSet expr);

	void clear();
	size_t size() const;

	string to_string() const;
};

ostream &operator<<(ostream &os, ConstOperationSet e);

}
