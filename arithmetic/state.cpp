/*
 * expression.cpp
 *
 *  Created on: Jul 1, 2015
 *      Author: nbingham
 */

#include "expression.h"

#include <sstream>

namespace arithmetic
{

value::value()
{
	data = unstable;
}

value::value(int data)
{
	this->data = data;
}

value::~value()
{

}

// Unstable is a subset of neutral and valid which are
// both subsets of unknown. 
//
//      unstable   .
//      /     \    .
//     v       v   .
//  neutral  valid .
//      \     /    .
//       v   v     .
//      unknown    .
bool value::is_subset_of(value v) const {
	return (data == v.data or
	        v.data == value::unknown or
	       (data == value::unstable and
	         (v.data >= value::valid or
	          v.data == value::neutral)));
}

ostream &operator<<(ostream &os, value v)
{
	if (v.data == value::neutral) {
		os << "N";
	} else if (v.data == value::unstable) {
		os << "X";
	} else if (v.data == value::unknown) {
		os << "-";
	} else {
		os << v.data;
	}
	return os;
}

value::operator bool() const {
	return data >= value::valid;
}

value operator~(value v)
{
	if (v.data >= value::valid)
		v.data = ~v.data;
	return v;
}

value operator-(value v)
{
	if (v.data >= value::valid)
		v.data = -v.data;
	return v;
}

value operator!(value v)
{
	if (v.data >= value::valid)
		v.data = value::neutral;
	else if (v.data == value::neutral)
		v.data = 0;
	return v;
}

value operator|(value v0, value v1)
{
	value result;

	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data | v1.data;

	return result;
}

value operator&(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data & v1.data;

	return result;
}

value operator^(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data ^ v1.data;

	return result;
}

value operator<<(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data << v1.data);

	return result;
}

value operator>>(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data >> v1.data);

	return result;
}

value operator+(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data + v1.data);

	return result;
}

value operator-(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data - v1.data);

	return result;
}

value operator*(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data * v1.data);

	return result;
}

value operator/(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data / v1.data);

	return result;
}

value operator%(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data % v1.data);

	return result;
}

value operator==(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data == v1.data)
		result.data = 0;
	else
		result.data = value::neutral;

	return result;
}

value operator!=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data != v1.data)
		result.data = 0;
	else
		result.data = value::neutral;
	return result;
}

value operator<(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data < v1.data)
		result.data = 0;
	else
		result.data = value::neutral;
	return result;
}

value operator>(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data > v1.data)
		result.data = 0;
	else
		result.data = value::neutral;
	return result;
}

value operator<=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data <= v1.data)
		result.data = 0;
	else
		result.data = value::neutral;
	return result;
}

value operator>=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data >= v1.data)
		result.data = 0;
	else
		result.data = value::neutral;
	return result;
}

value operator&&(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::neutral or v1.data == value::neutral)
		result.data = value::neutral;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = 0;
	return result;
}

value operator||(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable or v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data >= value::valid or v1.data >= value::valid)
		result.data = 0;
	else if (v0.data == value::unknown or v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = value::neutral;
	return result;
}

value intersect(value v0, value v1) {
	if (v0.data == value::unstable or v1.data == value::unstable) {
		return value(value::unstable);
	} else if (v0.data == value::unknown) {
		return v1;
	} else if (v1.data == value::unknown or v0.data == v1.data) {
		return v0;
	}
	return value(value::unstable);
}

value union_of(value v0, value v1) {
	if (v0.data == value::unstable) {
		return v1;
	} else if (v1.data == value::unstable) {
		return v0;
	} else if (v0.data != v1.data) {
		return value(value::unknown);
	}
	return v0;
}

state::state() {
}

state::state(int uid, value v) {
	set(uid, v);
}

state::~state() {
}

int state::size() const {
	return (int)values.size();
}

void state::extendX(int num) {
	values.resize(values.size()+num, value(value::unstable));
}

void state::extendN(int num) {
	values.resize(values.size()+num, value(value::neutral));
}

void state::extendU(int num) {
	values.resize(values.size()+num, value(value::unknown));
}

void state::trunk(int size) {
	values.resize(size);
}

void state::clear() {
	values.clear();
}

void state::push_back(value v) {
	values.push_back(v);
}

value state::get(int uid) const {
	if (uid < (int)values.size()) {
		return values[uid];
	} else {
		return value(value::unknown);
	}
}

void state::set(int uid, value v) {
	if (uid >= (int)values.size()) {
		values.resize(uid+1, value(value::unknown));
	}
	values[uid] = v;
}

void state::sv_intersect(int uid, value v) {
	if (uid >= (int)values.size()) {
		values.resize(uid+1, value(value::unknown));
	}
	values[uid] = intersect(values[uid], v);
}

bool state::is_subset_of(const state &s) const {
	int m0 = (int)min(values.size(), s.values.size());
	for (int i = 0; i < m0; i++) {
		if (!values[i].is_subset_of(s.values[i])) {
			return false;
		}
	}
	for (int i = m0; i < (int)s.values.size(); i++) {
		if (s.values[i].data != value::unknown) {
			return false;
		}
	}
	return true;
}

state &state::operator&=(state s) {
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

state &state::operator|=(state s) {
	if (s.values.size() < values.size()) {
		values.resize(s.values.size());
	}	
	for (int i = 0; i < (int)values.size(); i++) {
		values[i] = union_of(values[i], s.values[i]);
	}
	return *this;
}

state &state::operator=(state s) {
	values = s.values;
	return *this;
}

value &state::operator[](int uid) {
	if (uid >= (int)values.size()) {
		values.resize(uid+1, value(value::unknown));
	}
	return values[uid];
}

value state::operator[](int uid) const {
	if (uid < (int)values.size()) {
		return values[uid];
	} else {
		return value(value::unknown);
	}
}

state state::remote(vector<vector<int> > groups)
{
	state result = *this;
	for (int i = 0; i < (int)result.values.size(); i++) {
		value v = result.values[i];
		if (v.data == value::unknown) {
			continue;
		}

		for (auto j = groups.begin(); j != groups.end(); j++) {
			if (find(j->begin(), j->end(), i) == j->end()) {
				continue;
			}

			for (auto k = j->begin(); k != j->end(); k++) {
				if (*k != i) {
					result.sv_intersect(*k, v);
				}
			}
		}
	}

	return result;
}

bool state::is_tautology() const {
	for (auto v = values.begin(); v != values.end(); v++) {
		if (v->data != value::unknown) {
			return false;
		}
	}
	return true;
}

state state::mask() const {
	state result;
	result.values.reserve(values.size());
	for (auto v = values.begin(); v != values.end(); v++) {
		if (v->data == value::unknown) {
			result.values.push_back(value::unstable);
		} else {
			result.values.push_back(value::unknown);
		}
	}
	return result;
}

state state::mask(const state &m) const {
	state result;
	result.values.reserve(values.size());
	int m0 = min(values.size(), m.values.size());
	for (int i = 0; i < m0; i++) {
		if (m.values[i].data == value::unknown) {
			result.values.push_back(value::unknown);
		} else {
			result.values.push_back(values[i]);
		}
	}
	result.values.insert(result.values.end(), values.begin()+m0, values.end());
	return result;
}

state state::combine_mask(const state &m) const {
	state result;
	int m0 = min(values.size(), m.values.size());
	result.values.reserve(m0);
	for (int i = 0; i < m0; i++) {
		result.values.push_back(
			m.values[i].data == value::unknown or
			values[i].data == value::unknown ? 
				value::unknown : value::unstable);
	}
	return result;
}

ostream &operator<<(ostream &os, const state &s) {
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

bool operator==(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data != s1.values[i].data) {
			return false;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data != value::unknown) {
			return false;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return false;
		}
	}
	return true;
}

bool operator!=(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data != s1.values[i].data) {
			return true;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data != value::unknown) {
			return true;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return true;
		}
	}
	return false;
}

bool operator<(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data < s1.values[i].data) {
			return true;
		} else if (s0.values[i].data > s1.values[i].data) {
			return false;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data < value::unknown) {
			return true;
		} else if (s0.values[i].data > value::unknown) {
			return false;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return true;
		} else if (s1.values[i].data > value::unknown) {
			return false;
		}
	}
	return false;
}

bool operator>(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data < s1.values[i].data) {
			return false;
		} else if (s0.values[i].data > s1.values[i].data) {
			return true;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data < value::unknown) {
			return false;
		} else if (s0.values[i].data > value::unknown) {
			return true;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return false;
		} else if (s1.values[i].data > value::unknown) {
			return true;
		}
	}
	return false;
}

bool operator<=(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data < s1.values[i].data) {
			return true;
		} else if (s0.values[i].data > s1.values[i].data) {
			return false;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data < value::unknown) {
			return true;
		} else if (s0.values[i].data > value::unknown) {
			return false;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return true;
		} else if (s1.values[i].data > value::unknown) {
			return false;
		}
	}
	return true;
}

bool operator>=(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data < s1.values[i].data) {
			return false;
		} else if (s0.values[i].data > s1.values[i].data) {
			return true;
		}
	}
	for (int i = m0; i < (int)s0.values.size(); i++) {
		if (s0.values[i].data < value::unknown) {
			return false;
		} else if (s0.values[i].data > value::unknown) {
			return true;
		}
	}
	for (int i = m0; i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			return false;
		} else if (s1.values[i].data > value::unknown) {
			return true;
		}
	}
	return true;
}

state operator&(state s0, state s1) {
	state result;
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

state operator|(state s0, state s1) {
	int m0 = (int)min(s0.values.size(), s1.values.size());
	
	state result;
	result.values.reserve(m0);
	for (int i = 0; i < m0; i++) {
		result.values.push_back(union_of(s0.values[i], s1.values[i]));
	}
	return result;
}

state local_assign(state s0, state s1, bool stable)
{
	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown) {
			s0.values[i].data = s1.values[i].data;
		}
	}
	return s0;
}

state remote_assign(state s0, state s1, bool stable)
{
	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown and s0.values[i].data != s1.values[i].data) {
			s0.values[i].data = stable ? value::unknown : value::unstable;
		}
	}
	return s0;
}

bool vacuous_assign(const state &s0, const state &s1, bool stable) {
	for (int i = 0; i < (int)s0.values.size() and i < (int)s1.values.size(); i++) {
		if (s1.values[i].data != value::unknown and s0.values[i].data != s1.values[i].data and (not stable or s0.values[i].data != value::unstable)) {
			return false;
		}
	}
	return true;
}

bool are_interfering(const state &s0, const state &s1) {
	int m0 = min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data != value::unknown and s1.values[i].data != value::unknown and s0.values[i].data != s1.values[i].data) {
			return true;
		}
	}
	return false;
}

state interfere(state s0, const state &s1) {
	int m0 = min(s0.values.size(), s1.values.size());
	for (int i = 0; i < m0; i++) {
		if (s0.values[i].data != value::unknown and s1.values[i].data != value::unknown and s0.values[i].data != s1.values[i].data) {
			s0.values[i].data = value::unstable;
		}
	}
	return s0;
}

region::region() {
}

region::~region() {
}

region region::remote(vector<vector<int> > groups) {
	region result;
	for (auto s = states.begin(); s != states.end(); s++) {
		result.states.push_back(s->remote(groups));
	}
	return result;
}

bool region::is_tautology() const {
	for (auto s = states.begin(); s != states.end(); s++) {
		if (s->is_tautology()) {
			return true;
		}
	}
	return false;
}

state &region::operator[](int idx) {
	return states[idx];
}

state region::operator[](int idx) const {
	return states[idx];
}

bool vacuous_assign(const state &s0, const region &r1, bool stable) {
	for (auto s1 = r1.states.begin(); s1 != r1.states.end(); s1++) {
		if (vacuous_assign(s0, *s1, stable)) {
			return true;
		}
	}
	return false;
}

}
