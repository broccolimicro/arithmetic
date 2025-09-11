#include "expression.h"

#include <sstream>

namespace arithmetic
{

Value::Value() {
	type = ValType::WIRE;
	state = StateType::UNSTABLE;
}

Value::Value(bool bval) {
	this->type = ValType::BOOL;
	this->state = StateType::VALID;
	this->bval = bval;
}

Value::Value(int64_t ival) {
	this->type = ValType::INT;
	this->state = StateType::VALID;
	this->ival = ival;
}

Value::Value(int ival) {
	this->type = ValType::INT;
	this->state = StateType::VALID;
	this->ival = (int64_t)ival;
}

Value::Value(double rval) {
	this->type = ValType::REAL;
	this->state = StateType::VALID;
	this->rval = rval;
}

Value::Value(string sval) {
	this->type = ValType::STRING;
	this->state = StateType::VALID;
	this->sval = sval;
}

Value::~Value() {
}

bool Value::isValid() const {
	return state == StateType::VALID;
}

bool Value::isNeutral() const {
	return state == StateType::NEUTRAL;
}

bool Value::isUnstable() const {
	return state == StateType::UNSTABLE;
}

bool Value::isUnknown() const {
	return state == StateType::UNKNOWN;
}

bool Value::isTrue() const {
	if (type == WIRE) {
		return isValid();
	}
	Value v = boolOf(*this);
	return v.isValid() and v.bval;
}

const char *Value::ctypeName() const {
	if (type == ValType::WIRE) {
		return "wire";
	} else if (type == ValType::STRING) {
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
	return "??";
}

string Value::typeName() const {
	return string(ctypeName());
}

Value Value::X(Value::ValType type) {
	Value v;
	v.state = StateType::UNSTABLE;
	v.type = type;
	return v;
}

Value Value::U(Value::ValType type) {
	Value v;
	v.state = StateType::UNKNOWN;
	v.type = type;
	return v;
}

Value Value::gnd(Value::ValType type) {
	Value v;
	v.state = StateType::NEUTRAL;
	v.type = type;
	return v;
}

Value Value::vdd() {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::WIRE;
	return v;
}

Value Value::stringOf(string sval) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::STRING;
	v.sval = sval;
	return v;
}

Value Value::boolOf(bool bval) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::BOOL;
	v.bval = bval;
	return v;
}

Value Value::intOf(int64_t ival) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::INT;
	v.ival = ival;
	return v;
}

Value Value::realOf(double rval) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::REAL;
	v.rval = rval;
	return v;
}

Value Value::arrOf(vector<Value> arr) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::ARRAY;
	v.arr = arr;
	return v;
}

Value Value::structOf(string name, vector<Value> arr) {
	Value v;
	v.state = StateType::VALID;
	v.type = ValType::STRUCT;
	v.sval = name;
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
	if (v.isUnknown() or (isNeutral() and v.isNeutral())
		or (isValid() and v.isValid()
		and ((type == ValType::INT and v.type == ValType::INT and ival == v.ival)
		or (type == ValType::REAL and v.type == ValType::REAL and rval == v.rval)
		or (type == ValType::BOOL and v.type == ValType::BOOL and bval == v.bval)))
		or (isUnstable() and not v.isUnstable())) {
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
	if (v0.state == v1.state and (not v0.isValid()
		or (v0.type == Value::WIRE and v1.type == Value::WIRE)
		or (v0.type == Value::BOOL and v1.type == Value::BOOL and v0.bval == v1.bval)
		or (v0.type == Value::INT and v1.type == Value::INT and v0.ival == v1.ival)
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.rval == v1.rval)
		or (v0.type == Value::STRING and v1.type == Value::STRING and v0.sval == v1.sval)
	)) {
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

	if (v0.state < v1.state) {
		return -1;
	} else if (v0.state > v1.state) {
		return 1;
	}

	if (v0.isValid() and v1.isValid()) {
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
	if (v.isUnstable()) {
		os << "X";
	} else if (v.isNeutral()) {
		os << "gnd";
	} else if (v.type == Value::WIRE and v.isValid()) {
		os << "vdd";
	} else if (v.isUnknown()) {
		os << "U";
	} else if (v.type == Value::STRING) {
		os << "\"" << v.sval << "\"";
	} else if (v.type == Value::BOOL) {
		if (not v.bval) {
			os << "false";
		} else if (v.bval) {
			os << "true";
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
	} else if (v.type == Value::STRUCT) {
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
	return isTrue();
}

Type Value::typeOf() const {
	if (type == Value::WIRE
		or type == Value::STRING) {
		return Type(0.0, 0.0, 0.0);
	} else if (type == Value::BOOL) {
		return Type((double)bval, 0.0, 0.0);
	} else if (type == Value::INT) {
		return Type((double)ival, 0.0, 0.0);
	} else if (type == Value::REAL) {
		return Type(rval, 0.0, 0.0);
	}
	// TODO(edward.bingham) figure out how to handle arrays and structures
	printf("error: not yet implemented\n");
	return Type();
}

Value wtrue(Value v) {
	v = boolOf(v);
	if (v.isUnstable()) {
		return Value::X();
	} else if (v.isUnknown()) {
		return Value::U();
	} else if (v.isNeutral()) {
		return Value::gnd();
	}
	return v.bval ? Value::vdd() : Value::gnd();
}

Value bvalid(Value v) {
	v = wireOf(v);
	if (v.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v.isUnknown()) {
		return Value::U(Value::BOOL);
	}
	return Value::boolOf(v.isValid());
}

Value valid(Value v) {
	return wireOf(v);
}

// Wire NOT
Value operator~(Value v) {
	v = wireOf(v);
	if (v.isValid()) {
		return Value::gnd();
	} else if (v.isNeutral()) {
		return Value::vdd();
	}
	return v;
}

// Boolean NOT
Value operator!(Value v) {
	v = boolOf(v);
	v.bval = not v.bval;
	return v;
}

Value operator-(Value v) {
	if (v.type == Value::WIRE) {
		return v;
	} else if (v.type == Value::INT) {
		v.ival = -v.ival;
		return v;
	} else if (v.type == Value::REAL) {
		v.rval = -v.rval;
		return v;
	}
	printf("error: 'operator-' not defined for '%s'\n", v.ctypeName());
	return Value::X(v.type);
}

// Inverse
Value inv(Value v) {
	v = realOf(v);
	v.rval = 1.0/v.rval;
	return v;
}

// Boolean OR
Value operator||(Value v0, Value v1) {
	v0 = boolOf(v0);
	v1 = boolOf(v1);
	if ((v0.isValid() and v0.bval) or (v1.isValid() and v1.bval)) {
		return Value::boolOf(true);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		return Value::boolOf(false);
	}
	printf("error: 'operator||' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator&&(Value v0, Value v1) {
	v0 = boolOf(v0);
	v1 = boolOf(v1);
	if ((v0.isValid() and not v0.bval) or (v1.isValid() and not v1.bval)) {
		return Value::boolOf(false);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		return Value::boolOf(true);
	}
	printf("error: 'operator&&' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value operator^(Value v0, Value v1) {
	v0 = wireOf(v0);
	v1 = wireOf(v1);
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if ((v0.isValid() and v1.isNeutral())
		or (v0.isNeutral() and v1.isValid())) {
		return Value::vdd();
	} else if ((v0.isValid() and v1.isValid())
		or (v0.isNeutral() and v1.isNeutral())) {
		return Value::gnd();
	}
	return Value::X();
}

Value operator<<(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival << v1.ival);
		}
	}
	printf("error: 'operator<<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator>>(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival >> v1.ival);
		}
	}
	printf("error: 'operator>>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator+(Value v0, Value v1) {
	if (v0.type == Value::WIRE
		or v0.type == Value::BOOL
		or v0.type == Value::INT
		or v0.type == Value::REAL) {
		v1 = cast(v0.type, v1);
	}
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::stringOf(v0.sval + v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival + v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::realOf(v0.rval + v1.rval);
		} else if (v0.type == Value::ARRAY and v1.type == Value::ARRAY) {
			// concatination
			v0.arr.insert(v0.arr.end(), v1.arr.begin(), v1.arr.end());
			return v0;
		}
	}
	printf("error: 'operator+' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator-(Value v0, Value v1) {
	if (v0.type == Value::WIRE
		or v0.type == Value::BOOL
		or v0.type == Value::INT
		or v0.type == Value::REAL) {
		v1 = cast(v0.type, v1);
	}
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival - v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::realOf(v0.rval - v1.rval);
		}
	}
	printf("error: 'operator-' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator*(Value v0, Value v1) {
	if (v0.type == Value::WIRE
		or v0.type == Value::BOOL
		or v0.type == Value::INT
		or v0.type == Value::REAL) {
		v1 = cast(v0.type, v1);
	}
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival * v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::realOf(v0.rval * v1.rval);
		}
	}
	printf("error: 'operator*' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator/(Value v0, Value v1) {
	if (v0.type == Value::WIRE
		or v0.type == Value::BOOL
		or v0.type == Value::INT
		or v0.type == Value::REAL) {
		v1 = cast(v0.type, v1);
	}
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			if (v1.ival == 0) { 
				throw std::runtime_error("error: attempted to divide by 0\n"); 
			} 
			return Value::intOf(v0.ival / v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			if (v1.rval == 0.0) { 
				throw std::runtime_error("error: attempted to divide by 0.0\n"); 
			} 
			return Value::realOf(v0.rval / v1.rval);
		}
	}
	printf("error: 'operator/' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator%(Value v0, Value v1) {
	if (v0.type == Value::WIRE
		or v0.type == Value::BOOL
		or v0.type == Value::INT
		or v0.type == Value::REAL) {
		v1 = cast(v0.type, v1);
	}
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			if (v1.ival == 0) { 
				throw std::runtime_error("error: attempted to mod by 0\n"); 
			} 
			return Value::intOf(v0.ival % v1.ival);
		}
	}
	printf("error: 'operator%%' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(v0.type);
}

Value operator==(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval == v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival == v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval == v1.rval);
		}
	}
	printf("error: 'operator==' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator!=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval != v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival != v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval != v1.rval);
		}
	}
	printf("error: 'operator!=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator<(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval < v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival < v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval < v1.rval);
		}
	}
	printf("error: 'operator<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator>(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval > v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival > v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval > v1.rval);
		}
	}
	printf("error: 'operator>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator<=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval <= v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival <= v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval <= v1.rval);
		}
	}
	printf("error: 'operator<=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator>=(Value v0, Value v1) {
	if (v0.type == Value::STRING and v1.type == Value::STRING) {
		return Value::boolOf(v0.sval >= v1.sval);
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival >= v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval >= v1.rval);
		}
	}
	printf("error: 'operator>=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator&(Value v0, Value v1) {
	v0 = wireOf(v0);
	v1 = wireOf(v1);
	if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd();
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isValid() or v1.isValid()) {
		return Value::vdd();
	} else if (v0.isUnknown() and v1.isUnknown()) {
		return Value::U();
	}
	return Value::X();
}

Value operator|(Value v0, Value v1) {
	v0 = wireOf(v0);
	v1 = wireOf(v1);
	if (v0.isValid() or v1.isValid()) {
		return Value::vdd();
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U();
	} else if (v0.isNeutral() and v1.isNeutral()) {
		return Value::gnd();
	}
	return Value::X();
}

Value stringOf(Value v) {
	stringstream os;
	os << v;
	return Value::stringOf(os.str());
}

Value wireOf(Value v) {
	if (v.type == Value::WIRE) {
		return v;
	} else if (v.type == Value::ARRAY
		or v.type == Value::STRUCT) {
		Value result = Value::vdd();
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result & wireOf(*i);
		}
		return result;
	} else if (v.isUnstable()) {
		return Value::X();
	} else if (v.isUnknown()) {
		return Value::U();
	} else if (v.isNeutral()) {
		return Value::gnd();
	} else if (v.isValid()) {
		return Value::vdd();
	}
	printf("error: 'cast<wire>' not defined for '%s'\n", v.ctypeName());
	return Value::X(Value::WIRE);
}

Value boolOf(Value v) {
	if (v.type == Value::BOOL) {
		return v;
	} else if (v.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v.type == Value::INT) {
		return Value::boolOf((bool)v.ival);
	} else if (v.type == Value::REAL) {
		return Value::boolOf((bool)v.rval);
	} else if (v.type == Value::WIRE) {
		return Value::boolOf(true);
	}
	printf("error: 'cast<bool>' not defined for '%s'\n", v.ctypeName());
	return Value::X(Value::BOOL);
}

Value realOf(Value v) {
	if (v.type == Value::REAL) {
		return v;
	} else if (v.isUnstable()) {
		return Value::X(Value::REAL);
	} else if (v.isUnknown()) {
		return Value::U(Value::REAL);
	} else if (v.isNeutral()) {
		return Value::gnd(Value::REAL);
	} else if (v.type == Value::WIRE) {
		return Value::realOf(0.0);
	} else if (v.type == Value::INT) {
		return Value::realOf((double)v.ival);
	} else if (v.type == Value::BOOL) {
		return Value::realOf((double)v.bval);
	}
	printf("error: cast to 'real' not defined for '%s'\n", v.ctypeName());
	return Value::X();
}

Value intOf(Value v) {
	if (v.type == Value::INT) {
		return v;
	} else if (v.isUnstable()) {
		return Value::X(Value::INT);
	} else if (v.isUnknown()) {
		return Value::U(Value::INT);
	} else if (v.isNeutral()) {
		return Value::gnd(Value::INT);
	} else if (v.type == Value::WIRE) {
		return Value::intOf(0);
	} else if (v.type == Value::REAL) {
		return Value::intOf((int64_t)v.rval);
	} else if (v.type == Value::BOOL) {
		return Value::intOf((int64_t)v.ival);
	}
	printf("error: cast to 'int' not defined for '%s'\n", v.ctypeName());
	return Value::X(Value::INT);
}

Value cast(Value::ValType type, Value v) {
	if (v.type == type) {
		return v;
	} else if (type == Value::WIRE) {
		return wireOf(v);
	} else if (type == Value::BOOL) {
		return boolOf(v);
	} else if (type == Value::INT) {
		return intOf(v);
	} else if (type == Value::REAL) {
		return realOf(v);
	}
 	printf("error: cast to 'int' not defined for '%s'\n", v.ctypeName());
	return Value::X(type);
}

Value index(Value v, Value i) {
	if (i.isUnstable()) {
		return Value::X();
	} else if (i.isNeutral()) {
		return Value::gnd();
	} else if (i.isUnknown()) {
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
	if (f.isUnstable() or t.isUnstable()) {
		return Value::X(Value::ARRAY);
	} else if (f.isNeutral() or t.isNeutral()) {
		return Value::gnd(Value::ARRAY);
	} else if (f.isUnknown() or t.isUnknown()) {
		return Value::U(Value::ARRAY);
	} else if (v.type == Value::ARRAY and f.type == Value::INT and t.type == Value::INT) {
		if (f.ival >= 0 and f.ival < t.ival and t.ival <= (int)v.arr.size()) {
			v.arr = vector<Value>(v.arr.begin()+f.ival, v.arr.begin()+t.ival);
			return v;
		}
		printf("error: range [%ld, %ld) out of bounds for array of size %zu\n", (long)f.ival, (long)t.ival, v.arr.size());
		return Value::X(Value::ARRAY);
	}
	printf("error: 'operator[]' not defined for '%s', '%s', and '%s'\n", v.ctypeName(), f.ctypeName(), t.ctypeName());
	return Value::X(Value::ARRAY);
}

Value member(Value v0, Value v1, TypeSet types) {
	if (v0.type == Value::STRUCT and v1.type == Value::STRING) {
		int idx = types.memberIndex(v0.sval, v1.sval);
		if (idx >= 0 and idx < (int)v0.arr.size()) {
			return v0.arr[idx];
		}
		printf("internal: member %s(%d) out of bounds for structure '%s' of size %zu\n", v1.sval.c_str(), idx, v0.sval.c_str(), v0.arr.size());
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
		or (v0.type == Value::STRUCT and v1.type == Value::STRUCT and v0.sval == v1.sval))
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
		or (v0.type == Value::STRUCT and v1.type == Value::STRUCT and v0.sval == v1.sval))
		and v0.arr.size() == v1.arr.size()) {
		for (int i = 0; i < (int)v0.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	}
	return Value::U();
}

}
