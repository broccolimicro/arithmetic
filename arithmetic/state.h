/*
 * expression.h
 *
 *  Created on: Jun 25, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

namespace arithmetic
{

struct value
{
	value();
	value(int data);
	~value();

	int data;

	const static int invalid  = 0x80000000;
	const static int unstable = 0x80000001;
	const static int unknown  = 0x80000002;
	const static int valid	  = 0x80000003;
};

ostream &operator<<(ostream &os, value v);

value operator~(value v);
value operator-(value v);
value operator!(value v);
value operator|(value v0, value v1);
value operator&(value v0, value v1);
value operator^(value v0, value v1);
value operator<<(value v0, value v1);
value operator>>(value v0, value v1);
value operator+(value v0, value v1);
value operator-(value v0, value v1);
value operator*(value v0, value v1);
value operator/(value v0, value v1);
value operator%(value v0, value v1);

value operator==(value v0, value v1);
value operator!=(value v0, value v1);
value operator<(value v0, value v1);
value operator>(value v0, value v1);
value operator<=(value v0, value v1);
value operator>=(value v0, value v1);
value operator&&(value v0, value v1);
value operator||(value v0, value v1);

struct state
{
	state();
	~state();

	vector<value> values;

	int size() const;
	void extendX(int num);
	void extendN(int num);
	void extendU(int num);
	void trunk(int size);

	value get(int uid) const;
	void set(int uid, value v);

	state &operator=(state s);

	value &operator[](int uid);
};

ostream &operator<<(ostream &os, const state &s);

bool operator==(state s0, state s1);
bool operator!=(state s0, state s1);
bool operator<(state s0, state s1);
bool operator>(state s0, state s1);
bool operator<=(state s0, state s1);
bool operator>=(state s0, state s1);

}
