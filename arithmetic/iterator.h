#pragma once

#include <common/standard.h>

#include "operation.h"

namespace arithmetic {

_INTERFACE_ARG(OperationSet,
	(vector<Operand>, exprIndex, () const),
	(Operation *, exprAt, (size_t index)));

_CONST_INTERFACE_ARG(ConstOperationSet,
	(vector<Operand>, exprIndex, () const),
	(const Operation *, exprAt, (size_t index) const));

struct UpIterator {
	UpIterator(OperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~UpIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	Operation &get();
	Operation &operator*();
	Operation *operator->();
	UpIterator &operator++();
	bool done() const;
};

bool operator==(const UpIterator &i0, const UpIterator &i1);
bool operator!=(const UpIterator &i0, const UpIterator &i1);

struct ConstUpIterator {
	ConstUpIterator(ConstOperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
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
	DownIterator(OperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~DownIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	Operation &get();
	Operation &operator*();
	Operation *operator->();
	DownIterator &operator++();
	bool done() const;
};

bool operator==(const DownIterator &i0, const DownIterator &i1);
bool operator!=(const DownIterator &i0, const DownIterator &i1);

struct ConstDownIterator {
	ConstDownIterator(ConstOperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
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
