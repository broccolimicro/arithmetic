#pragma once

#include <common/standard.h>

#include "operation.h"

namespace arithmetic {

_INTERFACE_ARG(OperationSet,
	(Operation *, exprAt, (size_t index)));

_CONST_INTERFACE_ARG(ConstOperationSet,
	(const Operation *, exprAt, (size_t index) const));

struct Iterator {
	Iterator(OperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~Iterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	Operation &get();
	Operation &operator*();
	Operation *operator->();
	Iterator &operator++();
};

bool operator==(const Iterator &i0, const Iterator &i1);
bool operator!=(const Iterator &i0, const Iterator &i1);

struct ConstIterator {
	ConstIterator(ConstOperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~ConstIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstIterator &operator++();
};

bool operator==(const ConstIterator &i0, const ConstIterator &i1);
bool operator!=(const ConstIterator &i0, const ConstIterator &i1);

struct ReverseIterator {
	ReverseIterator(OperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~ReverseIterator();

	OperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	Operation &get();
	Operation &operator*();
	Operation *operator->();
	ReverseIterator &operator++();
};

bool operator==(const ReverseIterator &i0, const ReverseIterator &i1);
bool operator!=(const ReverseIterator &i0, const ReverseIterator &i1);

struct ConstReverseIterator {
	ConstReverseIterator(ConstOperationSet root, size_t exprIndex = std::numeric_limits<size_t>::max());
	~ConstReverseIterator();

	ConstOperationSet root;
	// prefer multiple vector<bool> instead of vector<pair<bool, bool>
	// > because vector<bool> is a bitset in a c++
	vector<bool> expand;
	vector<bool> seen;
	vector<size_t> stack;

	const Operation &get();
	const Operation &operator*();
	const Operation *operator->();
	ConstReverseIterator &operator++();
};

bool operator==(const ConstReverseIterator &i0, const ConstReverseIterator &i1);
bool operator!=(const ConstReverseIterator &i0, const ConstReverseIterator &i1);

}
