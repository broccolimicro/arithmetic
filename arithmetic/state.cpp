#include "expression.h"

#include <sstream>

namespace arithmetic
{

State::State() {
}

State::State(int uid, Value v) {
	set(uid, v);
}

State::~State() {
}

size_t State::size() const {
	return values.size();
}

void State::extendX(int num) {
	values.resize(values.size()+num, Value::X());
}

void State::extendN(int num) {
	values.resize(values.size()+num, false);
}

void State::extendU(int num) {
	values.resize(values.size()+num, Value::U());
}

void State::trunk(int size) {
	values.resize(size);
}

void State::clear() {
	values.clear();
}

void State::push_back(Value v) {
	values.push_back(v);
}

Value State::get(Reference lhs) const {
	if (lhs.isUndef()) {
		printf("error:%s:%d: variable not defined\n", __FILE__, __LINE__);
		return Value::X();
	} else if (lhs.uid >= values.size()) {
		printf("error:%s:%d: variable not defined 'v%zu'\n", __FILE__, __LINE__, lhs.uid);
		return Value::U();
	}
	return values[lhs.uid].get(lhs.slice);
}

void State::set(Reference lhs, Value rhs, bool define) {
	if (lhs.isUndef() and not define) {
		printf("error:%s:%d: variable not defined\n", __FILE__, __LINE__);
		return;
	}
	if (lhs.uid >= values.size()) {
		values.resize(lhs.uid+1);
	}
	values[lhs.uid].set(lhs.slice, rhs, define);
}

void State::svIntersect(Reference lhs, Value rhs) {
	if (lhs.isUndef()) {
		return;
	}
	if (lhs.uid >= values.size()) {
		values.resize(lhs.uid+1);
	}
	values[lhs.uid].set(lhs.slice, intersect(values[lhs.uid].get(lhs.slice), rhs));
}

bool State::isSubsetOf(const State &s) const {
	int m0 = (int)min(values.size(), s.values.size());
	for (int i = 0; i < m0; i++) {
		if (!values[i].isSubsetOf(s.values[i])) {
			return false;
		}
	}
	for (int i = m0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown()) {
			return false;
		}
	}
	return true;
}

State &State::operator&=(State s) {
	values.reserve(max(values.size(), s.values.size()));
	
	int m0 = (int)min(values.size(), s.values.size());
	for (int i = 0; i < m0; i++) {
		values[i] = intersect(values[i], s.values[i]);
	}
	for (int i = m0; i < (int)s.values.size(); i++) {
		values.push_back(s.values[i]);
	}

	return *this;
}

State &State::operator|=(State s) {
	if (s.values.size() < values.size()) {
		values.resize(s.values.size());
	}	
	for (int i = 0; i < (int)values.size(); i++) {
		values[i] = unionOf(values[i], s.values[i]);
	}
	return *this;
}

State &State::operator=(State s) {
	values = s.values;
	return *this;
}

Value State::operator[](Reference lhs) const {
	return get(lhs);
}

State State::remote(vector<vector<int> > groups)
{
	State result = *this;
	for (int i = 0; i < (int)result.values.size(); i++) {
		Value v = result.values[i];
		if (v.isUnstable()) {
			continue;
		}

		for (auto j = groups.begin(); j != groups.end(); j++) {
			if (find(j->begin(), j->end(), i) == j->end()) {
				continue;
			}

			for (auto k = j->begin(); k != j->end(); k++) {
				if (*k != i) {
					result.svIntersect(*k, v);
				}
			}
		}
	}

	return result;
}

bool State::isTautology() const {
	for (auto v = values.begin(); v != values.end(); v++) {
		if (not v->isUnknown()) {
			return false;
		}
	}
	return true;
}

State State::mask() const {
	State result;
	result.values.reserve(values.size());
	for (auto v = values.begin(); v != values.end(); v++) {
		if (v->isUnstable()) {
			result.values.push_back(Value::X());
		} else {
			result.values.push_back(Value::U());
		}
	}
	return result;
}

State State::mask(const State &m) const {
	State result;
	result.values.reserve(values.size());
	int m0 = min(values.size(), m.values.size());
	for (int i = 0; i < m0; i++) {
		if (m.values[i].isUnstable()) {
			result.values.push_back(Value::U());
		} else {
			result.values.push_back(values[i]);
		}
	}
	result.values.insert(result.values.end(), values.begin()+m0, values.end());
	return result;
}

State State::combineMask(const State &m) const {
	State result;
	int m0 = min(values.size(), m.values.size());
	result.values.reserve(m0);
	for (int i = 0; i < m0; i++) {
		result.values.push_back(
			m.values[i].isUnstable() or
			values[i].isUnstable() ? 
				Value::U() : Value::X());
	}
	return result;
}

void State::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	State result;
	for (int i = 0; i < (int)values.size() and i < (int)uidMap.size(); i++) {
		if (uidMap[i] >= 0) {
			if (uidMap[i] >= (int)result.values.size()) {
				result.values.resize(uidMap[i]+1, Value());
			}
			result.values[uidMap[i]] = values[i];
		}
	}
	values = result.values;
}

ostream &operator<<(ostream &os, const State &s) {
	os << "{";
	for (int i = 0; i < (int)s.values.size(); i++) {
		if (i != 0) {
			os << " ";
		}
		os << s.values[i];
	}
	os << "}";
	return os;
}

bool operator==(State s0, State s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (not areSame(s0.values[i], s1.values[i])) {
			return false;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (not s0.values[i].isUnknown()) {
			return false;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (not s1.values[i].isUnknown()) {
			return false;
		}
	}
	return true;
}

bool operator!=(State s0, State s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (not areSame(s0.values[i], s1.values[i])) {
			return true;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (not s0.values[i].isUnknown()) {
			return true;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (not s1.values[i].isUnknown()) {
			return true;
		}
	}
	return false;
}

bool operator<(State s0, State s1) {
	size_t m0 = max(s0.values.size(), s1.values.size());
	for (size_t i = 0; i < m0; i++) {
		int ord = order(i < s0.values.size() ? s0.values[i] : Value::U(),
		                i < s1.values.size() ? s1.values[i] : Value::U());
		if (ord != 0) {
			return ord < 0;
		}
	}
	return false;
}

bool operator>(State s0, State s1) {
	size_t m0 = max(s0.values.size(), s1.values.size());
	for (size_t i = 0; i < m0; i++) {
		int ord = order(i < s0.values.size() ? s0.values[i] : Value::U(),
		                i < s1.values.size() ? s1.values[i] : Value::U());
		if (ord != 0) {
			return ord > 0;
		}
	}
	return false;
}

bool operator<=(State s0, State s1) {
	size_t m0 = max(s0.values.size(), s1.values.size());
	for (size_t i = 0; i < m0; i++) {
		int ord = order(i < s0.values.size() ? s0.values[i] : Value::U(),
		                i < s1.values.size() ? s1.values[i] : Value::U());
		if (ord != 0) {
			return ord < 0;
		}
	}
	return true;
}

bool operator>=(State s0, State s1) {
	size_t m0 = max(s0.values.size(), s1.values.size());
	for (size_t i = 0; i < m0; i++) {
		int ord = order(i < s0.values.size() ? s0.values[i] : Value::U(),
		                i < s1.values.size() ? s1.values[i] : Value::U());
		if (ord != 0) {
			return ord > 0;
		}
	}
	return true;
}

State operator&(State s0, State s1) {
	State result;
	result.values.reserve(max(s0.values.size(), s1.values.size()));
	
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		result.values.push_back(intersect(s0.values[i], s1.values[i]));
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		result.values.push_back(s0.values[i]);
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		result.values.push_back(s1.values[i]);
	}

	return result;
}

State operator|(State s0, State s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	
	State result;
	result.values.reserve(m0);
	for (int i = 0; i < m0; i++) {
		result.values.push_back(unionOf(s0.values[i], s1.values[i]));
	}
	return result;
}

State localAssign(State s0, State s1, bool stable)
{
	if (s0.values.size() < s1.values.size()) {
		s0.values.resize(s1.values.size(), Value::U());
	}

	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (not s1.values[i].isUnknown()) {
			s0.values[i] = stable ? s1.values[i] : Value::X();
		}
	}
	return s0;
}

State remoteAssign(State s0, State s1, bool stable)
{
	if (s0.values.size() < s1.values.size()) {
		s0.values.resize(s1.values.size(), Value::U());
	}

	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (not s1.values[i].isUnknown() and not areSame(s0.values[i], s1.values[i])) {
			s0.values[i] = stable ? Value::U() : Value::X();
		}
	}
	return s0;
}

bool vacuousAssign(const State &s0, const State &s1, bool stable) {
	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (not s1.values[i].isUnknown() and not areSame(s0.values[i], s1.values[i]) and (not stable or not s0.values[i].isUnstable())) {
			return false;
		}
	}
	return true;
}

bool areInterfering(const State &s0, const State &s1) {
	int m0 = min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (not s0.values[i].isUnknown() and not s1.values[i].isUnknown() and not areSame(s0.values[i], s1.values[i])) {
			return true;
		}
	}
	return false;
}

State interfere(State s0, const State &s1) {
	int m0 = min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (not s0.values[i].isUnknown() and not s1.values[i].isUnknown() and not areSame(s0.values[i], s1.values[i])) {
			s0.values[i] = Value::X();
		}
	}
	return s0;
}

Region::Region() {
}

Region::~Region() {
}

Region Region::remote(vector<vector<int> > groups) {
	Region result;
	for (auto s = states.begin(); s != states.end(); s++) {
		result.states.push_back(s->remote(groups));
	}
	return result;
}

bool Region::isTautology() const {
	for (auto s = states.begin(); s != states.end(); s++) {
		if (s->isTautology()) {
			return true;
		}
	}
	return false;
}

State &Region::operator[](int idx) {
	return states[idx];
}

State Region::operator[](int idx) const {
	return states[idx];
}

void Region::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)states.size(); i++) {
		states[i].apply(uidMap);
	}
}

bool vacuousAssign(const State &s0, const Region &r1, bool stable) {
	for (auto s1 = r1.states.begin(); s1 != r1.states.end(); s1++) {
		if (vacuousAssign(s0, *s1, stable)) {
			return true;
		}
	}
	return false;
}

}
