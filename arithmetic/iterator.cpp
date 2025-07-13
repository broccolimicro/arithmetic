#include "iterator.h"

namespace arithmetic {

UpIterator::UpIterator(OperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

UpIterator::~UpIterator() {
}

const Operation &UpIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &UpIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *UpIterator::operator->() {
	return root.getExpr(stack.back());
}

// Depth first from leaves to root
UpIterator &UpIterator::operator++() {
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
		auto curr = root.getExpr(stack.back());
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

bool UpIterator::done() const {
	return stack.empty();
}

bool operator==(const UpIterator &i0, const UpIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const UpIterator &i0, const UpIterator &i1) {
	return i0.stack != i1.stack;
}

ConstUpIterator::ConstUpIterator(ConstOperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

ConstUpIterator::~ConstUpIterator() {
}

const Operation &ConstUpIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &ConstUpIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *ConstUpIterator::operator->() {
	return root.getExpr(stack.back());
}

ConstUpIterator &ConstUpIterator::operator++() {
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
		auto curr = root.getExpr(stack.back());
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

bool ConstUpIterator::done() const {
	return stack.empty();
}

bool operator==(const ConstUpIterator &i0, const ConstUpIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstUpIterator &i0, const ConstUpIterator &i1) {
	return i0.stack != i1.stack;
}

DownIterator::DownIterator(OperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

DownIterator::~DownIterator() {
}

const Operation &DownIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &DownIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *DownIterator::operator->() {
	return root.getExpr(stack.back());
}

DownIterator &DownIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.getExpr(stack.back());
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

bool DownIterator::done() const {
	return stack.empty();
}

bool operator==(const DownIterator &i0, const DownIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const DownIterator &i0, const DownIterator &i1) {
	return i0.stack != i1.stack;
}

ConstDownIterator::ConstDownIterator(ConstOperationSet root, size_t exprIndex) : root(root) {
	if (exprIndex != std::numeric_limits<size_t>::max()) {
		stack.push_back(exprIndex);
		++*this;
	}
}

ConstDownIterator::~ConstDownIterator() {
}

const Operation &ConstDownIterator::get() {
	return *root.getExpr(stack.back());
}

const Operation &ConstDownIterator::operator*() {
	return *root.getExpr(stack.back());
}

const Operation *ConstDownIterator::operator->() {
	return root.getExpr(stack.back());
}

ConstDownIterator &ConstDownIterator::operator++() {
	if (stack.empty()) {
		return *this;
	}

	if (stack.back() >= expand.size()) {
		expand.resize(stack.back()+1, false);
	}

	if (expand[stack.back()]) {
		auto curr = root.getExpr(stack.back());
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

bool ConstDownIterator::done() const {
	return stack.empty();
}

bool operator==(const ConstDownIterator &i0, const ConstDownIterator &i1) {
	return i0.stack == i1.stack;
}

bool operator!=(const ConstDownIterator &i0, const ConstDownIterator &i1) {
	return i0.stack != i1.stack;
}

}

