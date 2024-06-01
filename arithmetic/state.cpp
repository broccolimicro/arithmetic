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

ostream &operator<<(ostream &os, value v)
{
	if (v.data == value::invalid) {
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
		v.data = value::invalid;
	else if (v.data == value::invalid)
		v.data = 0;
	return v;
}

value operator|(value v0, value v1)
{
	value result;

	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data | v1.data;

	return result;
}

value operator&(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data & v1.data;

	return result;
}

value operator^(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = v0.data ^ v1.data;

	return result;
}

value operator<<(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data << v1.data);

	return result;
}

value operator>>(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data >> v1.data);

	return result;
}

value operator+(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data + v1.data);

	return result;
}

value operator-(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data - v1.data);

	return result;
}

value operator*(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data * v1.data);

	return result;
}

value operator/(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data / v1.data);

	return result;
}

value operator%(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = (v0.data % v1.data);

	return result;
}

value operator==(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data == v1.data)
		result.data = 0;
	else
		result.data = value::invalid;

	return result;
}

value operator!=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data != v1.data)
		result.data = 0;
	else
		result.data = value::invalid;
	return result;
}

value operator<(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data < v1.data)
		result.data = 0;
	else
		result.data = value::invalid;
	return result;
}

value operator>(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data > v1.data)
		result.data = 0;
	else
		result.data = value::invalid;
	return result;
}

value operator<=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data <= v1.data)
		result.data = 0;
	else
		result.data = value::invalid;
	return result;
}

value operator>=(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else if (v0.data >= v1.data)
		result.data = 0;
	else
		result.data = value::invalid;
	return result;
}

value operator&&(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data == value::invalid || v1.data == value::invalid)
		result.data = value::invalid;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = 0;
	return result;
}

value operator||(value v0, value v1)
{
	value result;
	if (v0.data == value::unstable || v1.data == value::unstable)
		result.data = value::unstable;
	else if (v0.data >= value::valid || v1.data >= value::valid)
		result.data = 0;
	else if (v0.data == value::unknown || v1.data == value::unknown)
		result.data = value::unknown;
	else
		result.data = value::invalid;
	return result;
}

state::state() {
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
	values.resize(values.size()+num, value(value::invalid));
}

void state::extendU(int num) {
	values.resize(values.size()+num, value(value::unknown));
}

void state::trunk(int size) {
	values.resize(size);
}

value state::get(int uid) const {
	return values[uid];
}

void state::set(int uid, value v) {
	values[uid] = v;
}

state &state::operator=(state s) {
	values = s.values;
	return *this;
}

value &state::operator[](int uid) {
	return values[uid];
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


}
