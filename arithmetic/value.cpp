#include "expression.h"

#include <sstream>

namespace arithmetic
{

value::value() {
	type = BOOL;
	bval = UNSTABLE;
}

value::value(bool bval) {
	this->type = BOOL;
	this->bval = (bval ? VALID : NEUTRAL);
}

value::value(int64_t ival) {
	this->type = INT;
	this->ival = ival;
}

value::value(int ival) {
	this->type = INT;
	this->ival = (int64_t)ival;
}

value::value(double rval) {
	this->type = REAL;
	this->rval = rval;
}

value::~value() {
}

bool value::isValid() const {
	return ((type == value::BOOL and bval == value::VALID)
		or type == value::INT
		or type == value::REAL);
}

bool value::isNeutral() const {
	return type == value::BOOL and bval == value::NEUTRAL;
}

bool value::isUnstable() const {
	return type == value::BOOL and bval == value::UNSTABLE;
}

bool value::isUnknown() const {
	return type == value::BOOL and bval == value::UNKNOWN;
}

const char *value::ctypeName() const {
	if (type == BOOL) {
		return "bool";
	} else if (type == INT) {
		return "int";
	} else if (type == REAL) {
		return "real";
	} else if (type == ARRAY) {
		return "array";
	} else if (type >= STRUCT) {
		return "struct";
	}
	return "void";
}

string value::typeName() const {
	return string(ctypeName());
}

value value::X() {
	value v;
	v.type = BOOL;
	v.bval = UNSTABLE;
	return v;
}

value value::U() {
	value v;
	v.type = BOOL;
	v.bval = UNKNOWN;
	return v;
}

value value::boolOf(bool bval) {
	value v;
	v.type = BOOL;
	v.bval = (bval ? VALID : NEUTRAL);
	return v;
}

value value::intOf(int64_t ival) {
	value v;
	v.type = INT;
	v.ival = ival;
	return v;
}

value value::realOf(double rval) {
	value v;
	v.type = REAL;
	v.rval = rval;
	return v;
}

value value::arrOf(vector<value> arr) {
	value v;
	v.type = ARRAY;
	v.arr = arr;
	return v;
}

value value::structOf(vector<value> arr) {
	value v;
	v.type = STRUCT;
	v.arr = arr;
	return v;
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
bool value::isSubsetOf(value v) const {
	if ((v.type == BOOL and v.bval == UNKNOWN)
		or (type == INT and v.type == INT and ival == v.ival)
		or (type == REAL and v.type == REAL and rval == v.rval)
		or (type == BOOL and v.type == BOOL and bval == v.bval)
		or (type == BOOL and bval == UNSTABLE
			and (v.type == INT
				or v.type == REAL
				or (v.type == BOOL
					and (v.bval == VALID
						or v.bval == NEUTRAL))))) {
		return true;
	}

	if (((type == ARRAY and v.type == ARRAY)
			or (type == STRUCT and v.type == STRUCT))
		and arr.size() == v.arr.size()) {
		for (int i = 0; i < (int)arr.size(); i++) {
			if (not arr[i].isSubsetOf(v.arr[i])) {
				return false;
			}
		}
		return true;
	}

	return false;
}

bool areSame(value v0, value v1) {
	if ((v0.type == value::BOOL and v1.type == value::BOOL and v0.bval == v1.bval)
		or (v0.type == value::INT and v1.type == value::INT and v0.ival == v1.ival)
		or (v0.type == value::REAL and v1.type == value::REAL and v0.rval == v1.rval)) {
		return true;
	}

	if (((v0.type == value::ARRAY and v1.type == value::ARRAY)
			or (v0.type == value::STRUCT and v1.type == value::STRUCT))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			if (not areSame(v0.arr[i], v1.arr[i])) {
				return false;
			}
		}
		return true;
	}

	return false;
}

int order(value v0, value v1) {
	if (v0.type < v1.type) {
		return -1;
	} else if (v0.type > v1.type) {
		return 1;
	}

	if ((v0.type == value::BOOL and v1.type == value::BOOL and v0.bval < v1.bval)
		or (v0.type == value::INT and v1.type == value::INT and v0.ival < v1.ival)
		or (v0.type == value::REAL and v1.type == value::REAL and v0.rval < v1.rval)) {
		return -1;
	} else if ((v0.type == value::BOOL and v1.type == value::BOOL and v0.bval > v1.bval)
		or (v0.type == value::INT and v1.type == value::INT and v0.ival > v1.ival)
		or (v0.type == value::REAL and v1.type == value::REAL and v0.rval > v1.rval)) {
		return 1;
	}

	if ((v0.type == value::ARRAY and v1.type == value::ARRAY)
			or (v0.type == value::STRUCT and v1.type == value::STRUCT)) {
		if (v0.arr.size() < v1.arr.size()) {
			return -1;
		} else if (v0.arr.size() > v1.arr.size()) {
			return 1;
		}

		for (int i = 0; i < (int)v0.arr.size(); i++) {
			int ord = order(v0.arr[i], v1.arr[i]);
			if (ord != 0) {
				return ord;
			}
		}
	}

	return 0;
}

ostream &operator<<(ostream &os, value v) {
	if (v.type == value::BOOL) {
		if (v.bval == value::UNSTABLE) {
			os << "X";
		} else if (v.bval == value::NEUTRAL) {
			os << "gnd";
		} else if (v.bval == value::VALID) {
			os << "vdd";
		} else if (v.bval == value::UNKNOWN) {
			os << "-";
		}
	} else if (v.type == value::INT) {
		os << v.ival;
	} else if (v.type == value::REAL) {
		os << v.rval;
	} else if (v.type == value::ARRAY) {
		os << "[";
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			if (i != v.arr.begin()) {
				os << ", ";
			}
			os << *i;
		}
		os << "]";
	} else if (v.type >= value::STRUCT) {
		os << "{";
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			if (i != v.arr.begin()) {
				os << ", ";
			}
			os << *i;
		}
		os << "}";
	}
	return os;
}

value::operator bool() const {
	return isValid();
}

value operator~(value v) {
	if (v.type == value::value::ARRAY or v.type >= value::value::STRUCT) {
		value result = value::boolOf(false);
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result | ~(*i);
		}
		return result;
	} else if (v.isValid()) {
		return value::boolOf(false);
	} else if (v.isNeutral()) {
		return value::boolOf(true);
	}
	return v;
}

value operator-(value v) {
	if (v.type == value::value::BOOL) {
		return v;
	} else if (v.type == value::value::INT) {
		v.ival = -v.ival;
		return v;
	} else if (v.type == value::value::REAL) {
		v.rval = -v.rval;
		return v;
	}
	printf("error: 'operator-' not defined for '%s'\n", v.ctypeName());
	return value::X();
}

value valid(value v) {
	if (v.type == value::ARRAY or v.type >= value::STRUCT) {
		value result = value::boolOf(true);
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result & valid(*i);
		}
		return result;
	} else if (v.isValid()) {
		return value::boolOf(true);
	} else if (v.isNeutral()) {
		return value::boolOf(false);
	}
	return v;
}

// Bitwise NOT
value operator!(value v) {
	if (v.type == value::value::BOOL) {
		return v;
	} else if (v.type == value::value::INT) {
		v.ival = ~v.ival;
		return v;
	}
	printf("error: 'operator-' not defined for '%s'\n", v.ctypeName());
	return value::X();
}

// Bitwise OR
value operator||(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival | v1.ival);
	}
	printf("error: 'operator||' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator&&(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival & v1.ival);
	}
	printf("error: 'operator&&' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator^(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival ^ v1.ival);
	}
	printf("error: 'operator^' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator<<(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival << v1.ival);
	}
	printf("error: 'operator<<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator>>(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival >> v1.ival);
	}
	printf("error: 'operator>>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator+(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival + v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::realOf(v0.rval + v1.rval);
	} else if (v0.type == value::value::ARRAY and v1.type == value::value::ARRAY) {
		// concatination
		v0.arr.insert(v0.arr.end(), v1.arr.begin(), v1.arr.end());
		return v0;
	}
	printf("error: 'operator+' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator-(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival - v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::realOf(v0.rval - v1.rval);
	}
	printf("error: 'operator-' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator*(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival * v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::realOf(v0.rval * v1.rval);
	}
	printf("error: 'operator*' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator/(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival / v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::realOf(v0.rval / v1.rval);
	}
	printf("error: 'operator/' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator%(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::intOf(v0.ival % v1.ival);
	}
	printf("error: 'operator%%' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator==(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival == v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval == v1.rval);
	}
	printf("error: 'operator==' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator!=(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival != v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval != v1.rval);
	}
	printf("error: 'operator!=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator<(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival < v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval < v1.rval);
	}
	printf("error: 'operator<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator>(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival > v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval > v1.rval);
	}
	printf("error: 'operator>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator<=(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival <= v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval <= v1.rval);
	}
	printf("error: 'operator<=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator>=(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.type == value::value::INT and v1.type == value::value::INT) {
		return value::boolOf(v0.ival >= v1.ival);
	} else if (v0.type == value::value::REAL and v1.type == value::value::REAL) {
		return value::boolOf(v0.rval >= v1.rval);
	}
	printf("error: 'operator>=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator&(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.isValid() and v1.isValid()) {
		return value::boolOf(true);
	}
	printf("error: 'operator&' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value operator|(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isValid() or v1.isValid()) {
		return value::boolOf(true);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return value::U();
	} else if (v0.isNeutral() and v1.isNeutral()) {
		return value::boolOf(false);
	}
	printf("error: 'operator|' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return value::X();
}

value boolOf(value v) {
	return valid(v);
}

value realOf(value v) {
	if (v.isUnstable() or v.isUnknown() or v.isNeutral() or v.type == value::value::REAL) {
		return v;
	} else if (v.type == value::value::BOOL) {
		return value::realOf(0.0);
	} else if (v.type == value::value::INT) {
		return value::realOf((double)v.ival);
	}
	printf("error: cast to 'real' not defined for '%s'\n", v.ctypeName());
	return value::X();
}

value intOf(value v) {
	if (v.isUnstable() or v.isUnknown() or v.isNeutral() or v.type == value::value::INT) {
		return v;
	} else if (v.type == value::value::BOOL) {
		return value::intOf(0);
	} else if (v.type == value::value::REAL) {
		return value::intOf((int64_t)v.rval);
	}
	printf("error: cast to 'int' not defined for '%s'\n", v.ctypeName());
	return value::X();
}

value index(value v, value i) {
	if (v.isUnstable() or i.isUnstable()) {
		return value::X();
	} else if (v.isNeutral() or i.isNeutral()) {
		return value::boolOf(false);
	} else if (v.isUnknown() or i.isUnknown()) {
		return value::U();
	} else if (v.type == value::value::ARRAY and i.type == value::value::INT) {
		if (i.ival >= 0 and i.ival < (int)v.arr.size()) {
			return v.arr[i.ival];
		}
		printf("error: index %d out of bounds for array of size %ld\n", (int)v.arr.size(), i.ival);
		return value::X();
	}
	printf("error: 'operator[]' not defined for '%s' and '%s'\n", v.ctypeName(), i.ctypeName());
	return value::X();
}

value intersect(value v0, value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return value::X();
	} else if (v0.isUnknown()) {
		return v1;
	} else if (v1.isUnknown()
		or (v0.type == value::BOOL and v1.type == value::BOOL and v0.bval == v1.bval)
		or (v0.type == value::INT and v1.type == value::INT and v0.ival == v1.ival)
		or (v0.type == value::REAL and v1.type == value::REAL and v0.rval == v1.rval)) {
		return v0;
	} else if (((v0.type == value::ARRAY and v1.type == value::ARRAY)
		or (v0.type == value::STRUCT and v1.type == value::STRUCT))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	}
	return value::X();
}

value union_of(value v0, value v1) {
	if (v0.isUnstable()) {
		return v1;
	} else if (v1.isUnstable()) {
		return v0;
	} else if ((v0.type == value::BOOL and v1.type == value::BOOL and v0.bval == v1.bval)
		or (v0.type == value::INT and v1.type == value::INT and v0.ival == v1.ival)
		or (v0.type == value::REAL and v1.type == value::REAL and v0.rval == v1.rval)) {
		return v0;
	} else if (((v0.type == value::ARRAY and v1.type == value::ARRAY)
		or (v0.type == value::STRUCT and v1.type == value::STRUCT))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	}
	return value::U();
}

}
