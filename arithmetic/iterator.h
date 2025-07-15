#pragma once

#include <common/standard.h>

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

struct UpIterator {
	UpIterator(OperationSet root, vector<Operand> start=vector<Operand>());
	~UpIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	UpIterator &operator++();
	bool done() const;
};

bool operator==(const UpIterator &i0, const UpIterator &i1);
bool operator!=(const UpIterator &i0, const UpIterator &i1);

struct ConstUpIterator {
	ConstUpIterator(ConstOperationSet root, vector<Operand> start=vector<Operand>());
	~ConstUpIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstUpIterator &operator++();
	bool done() const;
};

bool operator==(const ConstUpIterator &i0, const ConstUpIterator &i1);
bool operator!=(const ConstUpIterator &i0, const ConstUpIterator &i1);

struct DownIterator {
	DownIterator(OperationSet root, vector<Operand> start=vector<Operand>());
	~DownIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	DownIterator &operator++();
	bool done() const;
};

bool operator==(const DownIterator &i0, const DownIterator &i1);
bool operator!=(const DownIterator &i0, const DownIterator &i1);

struct ConstDownIterator {
	ConstDownIterator(ConstOperationSet root, vector<Operand> start=vector<Operand>());
	~ConstDownIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstDownIterator &operator++();
	bool done() const;
};

bool operator==(const ConstDownIterator &i0, const ConstDownIterator &i1);
bool operator!=(const ConstDownIterator &i0, const ConstDownIterator &i1);

}
