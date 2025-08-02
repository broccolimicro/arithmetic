#include "expression.h"

#include <sstream>

namespace arithmetic
{

Value::Value() {
	type = ValType::BOOL;
	bval = Bval::UNSTABLE;
}

Value::Value(bool bval) {
	this->type = ValType::BOOL;
	this->bval = (bval ? Bval::VALID : Bval::NEUTRAL);
}

Value::Value(int64_t ival) {
	this->type = ValType::INT;
	this->ival = ival;
}

Value::Value(int ival) {
	this->type = ValType::INT;
	this->ival = (int64_t)ival;
}

Value::Value(double rval) {
	this->type = ValType::REAL;
	this->rval = rval;
}

Value::Value(string sval) {
	this->type = ValType::STRING;
	this->sval = sval;
}

Value::~Value() {
}

bool Value::isValid() const {
	return ((type == ValType::BOOL and bval == Bval::VALID)
		or type == ValType::INT
		or type == ValType::REAL);
}

bool Value::isNeutral() const {
	return type == ValType::BOOL and bval == Bval::NEUTRAL;
}

bool Value::isUnstable() const {
	return (type == ValType::BOOL and bval == Bval::UNSTABLE) or type == ValType::STRING;
}

bool Value::isUnknown() const {
	return type == ValType::BOOL and bval == Bval::UNKNOWN;
}

const char *Value::ctypeName() const {
	if (type == ValType::STRING) {
		return "string";
	} else if (type == ValType::BOOL) {
		return "bool";
	} else if (type == ValType::INT) {
		return "int";
	} else if (type == ValType::REAL) {
		return "real";
	} else if (type == ValType::ARRAY) {
		return "array";
	} else if (type >= ValType::STRUCT) {
		return "struct";
	}
	return "void";
}

string Value::typeName() const {
	return string(ctypeName());
}

Value Value::X() {
	Value v;
	v.type = ValType::BOOL;
	v.bval = Bval::UNSTABLE;
	return v;
}

Value Value::U() {
	Value v;
	v.type = ValType::BOOL;
	v.bval = Bval::UNKNOWN;
	return v;
}

Value Value::stringOf(string sval) {
	Value v;
	v.type = ValType::STRING;
	v.sval = sval;
	return v;
}

Value Value::boolOf(bool bval) {
	Value v;
	v.type = ValType::BOOL;
	v.bval = (bval ? Bval::VALID : Bval::NEUTRAL);
	return v;
}

Value Value::intOf(int64_t ival) {
	Value v;
	v.type = ValType::INT;
	v.ival = ival;
	return v;
}

Value Value::realOf(double rval) {
	Value v;
	v.type = ValType::REAL;
	v.rval = rval;
	return v;
}

Value Value::arrOf(vector<Value> arr) {
	Value v;
	v.type = ValType::ARRAY;
	v.arr = arr;
	return v;
}

Value Value::structOf(vector<Value> arr) {
	Value v;
	v.type = ValType::STRUCT;
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
bool Value::isSubsetOf(Value v) const {
	if ((v.type == ValType::BOOL and v.bval == Bval::UNKNOWN)
		or (type == ValType::INT and v.type == ValType::INT and ival == v.ival)
		or (type == ValType::REAL and v.type == ValType::REAL and rval == v.rval)
		or (type == ValType::BOOL and v.type == ValType::BOOL and bval == v.bval)
		or (type == ValType::BOOL and bval == Bval::UNSTABLE
			and (v.type == ValType::INT
				or v.type == ValType::REAL
				or (v.type == ValType::BOOL
					and (v.bval == Bval::VALID
						or v.bval == Bval::NEUTRAL))))) {
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

bool areSame(Value v0, Value v1) {
	if ((v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval == v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival == v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval == v1.rval)
		or (v0.type == Value::STRING and v1.type == Value::STRING and v0.sval == v1.sval)
	) {
		return true;
	}

	if (((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
			or (v0.type == Value::STRUCT and v1.type == Value::STRUCT))
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

int order(Value v0, Value v1) {
	if (v0.type < v1.type) {
		return -1;
	} else if (v0.type > v1.type) {
		return 1;
	}

	if ((v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval < v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival < v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval < v1.rval)
		or (v0.type == Value::STRING and v1.type == Value::STRING and v0.sval < v1.sval)
	) {
		return -1;
	} else if ((v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval > v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival > v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval > v1.rval)
		or (v0.type == Value::STRING and v1.type == Value::STRING and v0.sval > v1.sval)
	) {
		return 1;
	}

	if ((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
			or (v0.type == Value::STRUCT and v1.type == Value::STRUCT)) {
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

ostream &operator<<(ostream &os, Value v) {
	if (v.type == Value::STRING) {
		os << "\"" << v.sval << "\"";
	} else if (v.type == Value::BOOL) {
		if (v.bval == Value::UNSTABLE) {
			os << "X";
		} else if (v.bval == Value::NEUTRAL) {
			os << "false";
		} else if (v.bval == Value::VALID) {
			os << "true";
		} else if (v.bval == Value::UNKNOWN) {
			os << "U";
		} else {
			os << "berror(" << v.bval << ")";
		}
	} else if (v.type == Value::INT) {
		os << v.ival;
	} else if (v.type == Value::REAL) {
		os << v.rval;
	} else if (v.type == Value::ARRAY) {
		os << "[";
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			if (i != v.arr.begin()) {
				os << ", ";
			}
			os << *i;
		}
		os << "]";
	} else if (v.type >= Value::STRUCT) {
		os << "{";
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			if (i != v.arr.begin()) {
				os << ", ";
			}
			os << *i;
		}
		os << "}";
	} else {
		os << "cerror(" << v.type << ")";
	}
	return os;
}

Value::operator bool() const {
	return isValid();
}

Type Value::typeOf() const {
	if (type == Value::BOOL or type == Value::STRING) {
		return Type(0.0, 0.0, 0.0);
	} else if (type == Value::INT) {
		return Type((double)ival, 0.0, 0.0);
	} else if (type == Value::REAL) {
		return Type(rval, 0.0, 0.0);
	}
	// TODO(edward.bingham) figure out how to handle arrays and structures
	printf("error: not yet implemented\n");
	return Type();
}

Value operator~(Value v) {
	if (v.type == Value::ARRAY or v.type >= Value::STRUCT) {
		Value result = Value::boolOf(false);
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result | ~(*i);
		}
		return result;
	} else if (v.isValid()) {
		return Value::boolOf(false);
	} else if (v.isNeutral()) {
		return Value::boolOf(true);
	}
	return v;
}

Value operator-(Value v) {
	if (v.type == Value::BOOL) {
		return v;
	} else if (v.type == Value::INT) {
		v.ival = -v.ival;
		return v;
	} else if (v.type == Value::REAL) {
		v.rval = -v.rval;
		return v;
	}
	printf("error: 'operator-' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}

Value valid(Value v) {
	if (v.type == Value::ARRAY or v.type >= Value::STRUCT) {
		Value result = Value::boolOf(true);
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result & valid(*i);
		}
		return result;
	} else if (v.isValid()) {
		return Value::boolOf(true);
	} else if (v.isNeutral()) {
		return Value::boolOf(false);
	}
	return v;
}

// Bitwise NOT
Value operator!(Value v) {
	if (v.type == Value::BOOL) {
		return v;
	} else if (v.type == Value::INT) {
		v.ival = ~v.ival;
		return v;
	}
	printf("error: 'operator!' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}

// Inverse
Value inv(Value v) {
	if (v.type == Value::BOOL) {
		return v;
	} else if (v.type == Value::INT) {
		v.ival = 0;
		return v;
	} else if (v.type == Value::REAL) {
		v.rval = 1.0/v.rval;
		return v;
	}
	printf("error: 'inverse' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}


// Bitwise OR
Value operator||(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival | v1.ival);
	}
	printf("error: 'operator||' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator&&(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival & v1.ival);
	}
	printf("error: 'operator&&' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator^(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival ^ v1.ival);
	}
	printf("error: 'operator^' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator<<(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival << v1.ival);
	}
	printf("error: 'operator<<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator>>(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival >> v1.ival);
	}
	printf("error: 'operator>>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator+(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::stringOf(v0.sval + v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival + v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::realOf(v0.rval + v1.rval);
	} else if (v0.type == Value::ARRAY and v1.type == Value::ARRAY) {
		// concatination
		v0.arr.insert(v0.arr.end(), v1.arr.begin(), v1.arr.end());
		return v0;
	}
	printf("error: 'operator+' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator-(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival - v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::realOf(v0.rval - v1.rval);
	}
	printf("error: 'operator-' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator*(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival * v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::realOf(v0.rval * v1.rval);
	}
	printf("error: 'operator*' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator/(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival / v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::realOf(v0.rval / v1.rval);
	}
	printf("error: 'operator/' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator%(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::intOf(v0.ival % v1.ival);
	}
	printf("error: 'operator%%' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator==(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval == v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival == v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval == v1.rval);
	}
	printf("error: 'operator==' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator!=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval != v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival != v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval != v1.rval);
	}
	printf("error: 'operator!=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator<(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval < v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival < v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval < v1.rval);
	}
	printf("error: 'operator<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator>(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval > v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival > v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval > v1.rval);
	}
	printf("error: 'operator>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator<=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval <= v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival <= v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval <= v1.rval);
	}
	printf("error: 'operator<=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator>=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval >= v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.type == Value::INT and v1.type == Value::INT) {
		return Value::boolOf(v0.ival >= v1.ival);
	} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
		return Value::boolOf(v0.rval >= v1.rval);
	}
	printf("error: 'operator>=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator&(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::boolOf(false);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.isValid() and v1.isValid()) {
		return Value::boolOf(true);
	}
	printf("error: 'operator&' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator|(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isValid() or v1.isValid()) {
		return Value::boolOf(true);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.isNeutral() and v1.isNeutral()) {
		return Value::boolOf(false);
	}
	printf("error: 'operator|' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value stringOf(Value v) {
	stringstream os;
	os << v;
	return Value::stringOf(os.str());
}

Value boolOf(Value v) {
	return valid(v);
}

Value realOf(Value v) {
	if (v.isUnstable() or v.isUnknown() or v.isNeutral() or v.type == Value::REAL) {
		return v;
	} else if (v.type == Value::BOOL) {
		return Value::realOf(0.0);
	} else if (v.type == Value::INT) {
		return Value::realOf((double)v.ival);
	}
	printf("error: cast to 'real' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}

Value intOf(Value v) {
	if (v.isUnstable() or v.isUnknown() or v.isNeutral() or v.type == Value::INT) {
		return v;
	} else if (v.type == Value::BOOL) {
		return Value::intOf(0);
	} else if (v.type == Value::REAL) {
		return Value::intOf((int64_t)v.rval);
	}
	printf("error: cast to 'int' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}

Value index(Value v, Value i) {
	if (v.isUnstable() or i.isUnstable()) {
		return Value::X();
	} else if (v.isNeutral() or i.isNeutral()) {
		return Value::boolOf(false);
	} else if (v.isUnknown() or i.isUnknown()) {
		return Value::U();
	} else if (v.type == Value::ARRAY and i.type == Value::INT) {
		if (i.ival >= 0 and i.ival < (int)v.arr.size()) {
			return v.arr[i.ival];
		}
		printf("error: index %ld out of bounds for array of size %zu\n", (long)i.ival, v.arr.size());
		return Value::X();
	}
	printf("error: 'operator[]' not defined for '%s' and '%s'\n", v.ctypeName(), i.ctypeName());
	return Value::X();
}

Value index(Value v, Value f, Value t) {
	if (v.isUnstable() or f.isUnstable() or t.isUnstable()) {
		return Value::X();
	} else if (v.isNeutral() or f.isNeutral() or t.isNeutral()) {
		return Value::boolOf(false);
	} else if (v.isUnknown() or f.isUnknown() or t.isUnknown()) {
		return Value::U();
	} else if (v.type == Value::ARRAY and f.type == Value::INT and t.type == Value::INT) {
		if (f.ival >= 0 and f.ival < (int)v.arr.size() and t.ival >= 0 and t.ival <= (int)v.arr.size()) {
			v.arr = vector<Value>(v.arr.begin()+f.ival, v.arr.begin()+t.ival);
			return v;
		}
		printf("error: range [%ld, %ld) out of bounds for array of size %zu\n", (long)f.ival, (long)t.ival, v.arr.size());
		return Value::X();
	}
	printf("error: 'operator[]' not defined for '%s', '%s', and '%s'\n", v.ctypeName(), f.ctypeName(), t.ctypeName());
	return Value::X();
}

Value member(Value v0, Value v1, TypeSet types) {
	if (v0.type >= Value::STRUCT and v1.type == Value::STRING) {
		int idx = types.memberIndex(v0.type, v1.sval);
		if (idx >= 0 and idx < (int)v0.arr.size()) {
			return v0.arr[idx];
		}
		printf("internal: member %s(%d) out of bounds for structure of size %zu\n", v1.sval.c_str(), idx, v0.arr.size());
		return Value::X();
	}
	printf("error: 'operator.' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value intersect(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isUnknown()) {
		return v1;
	} else if (v1.isUnknown()
		or (v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval == v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival == v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval == v1.rval)) {
		return v0;
	} else if (((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
		or (v0.type == Value::STRUCT and v1.type == Value::STRUCT))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	}
	return Value::X();
}

Value unionOf(Value v0, Value v1) {
	if (v0.isUnstable()) {
		return v1;
	} else if (v1.isUnstable()) {
		return v0;
	} else if ((v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval == v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival == v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval == v1.rval)) {
		return v0;
	} else if (((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
		or (v0.type == Value::STRUCT and v1.type == Value::STRUCT))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	}
	return Value::U();
}

}
