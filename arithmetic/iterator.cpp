#include "iterator.h"

namespace arithmetic {

Iterator::Iterator(OperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

Iterator::~Iterator() {
}

Operation &Iterator::get() {
	return *root.exprAt(stack.back());
}

Operation &Iterator::operator*() {
	return *root.exprAt(stack.back());
}

Operation *Iterator::operator->() {
	return root.exprAt(stack.back());
}

// Depth first from leaves to root
Iterator &Iterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= seen.size()) {
		seen.resize(stack.back()+1, false);
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root.exprAt(stack.back());
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}
				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	return *this;
}

bool operator==(const Iterator &i0, const Iterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const Iterator &i0, const Iterator &i1) {
	return i0.stack != i1.stack;
}

ConstIterator::ConstIterator(ConstOperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

ConstIterator::~ConstIterator() {
}

const Operation &ConstIterator::get() {
	return *root.exprAt(stack.back());
}

const Operation &ConstIterator::operator*() {
	return *root.exprAt(stack.back());
}

const Operation *ConstIterator::operator->() {
	return root.exprAt(stack.back());
}

ConstIterator &ConstIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= seen.size()) {
		seen.resize(stack.back()+1, false);
	}

	if (seen[stack.back()]) {
		stack.pop_back();
	} else {
		seen[stack.back()] = true;
	}
	while (not stack.empty()) {
		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}
		if (expand[stack.back()]) {
			return *this;
		}

		expand[stack.back()] = true;
		auto curr = root.exprAt(stack.back());
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}
				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	return *this;
}

bool operator==(const ConstIterator &i0, const ConstIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstIterator &i0, const ConstIterator &i1) {
	return i0.stack != i1.stack;
}

ReverseIterator::ReverseIterator(OperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

ReverseIterator::~ReverseIterator() {
}

Operation &ReverseIterator::get() {
	return *root.exprAt(stack.back());
}

Operation &ReverseIterator::operator*() {
	return *root.exprAt(stack.back());
}

Operation *ReverseIterator::operator->() {
	return root.exprAt(stack.back());
}

ReverseIterator &ReverseIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.exprAt(stack.back());
		stack.pop_back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}

				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	if (not stack.empty()) {
		if (stack.back() >= seen.size()) {
			seen.resize(stack.back()+1, false);
		}

		seen[stack.back()] = true;

		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}

		expand[stack.back()] = true;
	}
	return *this;
}

bool operator==(const ReverseIterator &i0, const ReverseIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ReverseIterator &i0, const ReverseIterator &i1) {
	return i0.stack != i1.stack;
}

ConstReverseIterator::ConstReverseIterator(ConstOperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

ConstReverseIterator::~ConstReverseIterator() {
}

const Operation &ConstReverseIterator::get() {
	return *root.exprAt(stack.back());
}

const Operation &ConstReverseIterator::operator*() {
	return *root.exprAt(stack.back());
}

const Operation *ConstReverseIterator::operator->() {
	return root.exprAt(stack.back());
}

ConstReverseIterator &ConstReverseIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.exprAt(stack.back());
		stack.pop_back();
		for (auto i = curr->operands.begin(); i != curr->operands.end(); i++) {
			if (i->isExpr()) {
				if (i->index >= seen.size()) {
					seen.resize(i->index+1, false);
				}

				if (i->index >= 0 and not seen[i->index]) {
					stack.push_back(i->index);
					seen[i->index] = true;
				}
			}
		}
	}

	if (not stack.empty()) {
		if (stack.back() >= seen.size()) {
			seen.resize(stack.back()+1, false);
		}

		seen[stack.back()] = true;

		if (stack.back() >= expand.size()) {
			expand.resize(stack.back()+1, false);
		}

		expand[stack.back()] = true;
	}
	return *this;
}

bool operator==(const ConstReverseIterator &i0, const ConstReverseIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstReverseIterator &i0, const ConstReverseIterator &i1) {
	return i0.stack != i1.stack;
}

}

