#include "expression.h"

#include <sstream>

namespace arithmetic
{

Slice::Slice() {
	this->from = 0;
	this->to = std::numeric_limits<size_t>::max();
}

Slice::~Slice() {
}

bool Slice::isSlice() const {
	return to != std::numeric_limits<size_t>::max();
}

bool Slice::index(size_t index) {
	if (to < from + index) {
		printf("error: index %zu out of bounds for array of size %zu\n", index, to-from);
		return false;
	}
	idx.push_back(from + index);
	memb.push_back(false);
	from = 0;
	to = std::numeric_limits<size_t>::max();
	return true;
}

bool Slice::member(size_t index) {
	if (isSlice()) {
		printf("error: member operator not defined for arrays\n");
		return false;
	}
	idx.push_back(index);
	memb.push_back(true);
	return true;
}

bool Slice::slice(size_t from, size_t to) {
	if (this->to < this->from + to) {
		printf("error: range [%zu, %zu) out of bounds for array of size %zu\n", from, to, this->to-this->from);
		return false;
	}
	this->to = this->from + to;
	this->from = this->from + from;
	return true;
}

ostream &operator<<(ostream &os, Slice slice) {
	for (size_t i = 0; i < slice.idx.size(); i++) {
		if (slice.memb[i]) {
			os << ".m" << slice.idx[i];
		} else {
			os << "[" << slice.idx[i] << "]";
		}
	}
	if (slice.isSlice()) {
		os << "[" << slice.from << ":" << slice.to << "]";
	}
	return os;
}

Reference::Reference(size_t uid) {
	this->uid = uid;
}

Reference::~Reference() {
}

bool Reference::isUndef() const {
	return uid == std::numeric_limits<size_t>::max();
}

ostream &operator<<(ostream &os, Reference ref) {
	if (ref.isUndef()) {
		os << "undef";
	} else {
		os << "v" << ref.uid << ref.slice;
	}
	return os;
}

Value::Value() {
	type = ValType::UNDEF;
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

bool Value::isUndef() const {
	return type == ValType::UNDEF;
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
	if (type == ValType::UNDEF) {
		return "undef";
	} else if (type == ValType::WIRE) {
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

Value Value::undef() {
	return Value();
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

void Value::set(Slice slice, Value v, bool define) {
	//cout << "Value::set(): " << slice << " " << v << " " << define << endl;
	Value *dst = this;
	int i = 0;
	for (; i < (int)slice.idx.size() and dst->type != Value::INT; i++) {
		if (define and dst->isUndef()) {
			*dst = Value::U(slice.memb[i] ? Value::STRUCT : Value::ARRAY);
		}

		if (slice.memb[i] and dst->type != Value::STRUCT) {
			printf("internal:%s:%d: member operator expected struct type\n", __FILE__, __LINE__);
			return;
		} else if (not slice.memb[i] and dst->type != Value::ARRAY) {
			printf("internal:%s:%d: index operator expected array or int type\n", __FILE__, __LINE__);
			return;
		} else if (slice.idx[i] >= v.arr.size()) {
			if (define) {
				dst->arr.resize(slice.idx[i]+1);
				dst = &dst->arr[slice.idx[i]];
			} else {
				printf("error: index %zu out of bounds for array of size %zu\n", slice.idx[i], v.arr.size());
				return;
			}
		} else {
			dst = &dst->arr[slice.idx[i]];
		}
	}

	Slice accum;
	for (; i < (int)slice.idx.size() and dst->type == Value::INT; i++) {
		if (not accum.slice(slice.idx[i], slice.idx[i]+1)) {
			return;
		}
	}

	if (slice.isSlice() and not accum.slice(slice.from, slice.to)) {
		return;
	}

	if (accum.isSlice()) {
		if (define and dst->isUndef()) {
			dst->type = v.type;
		}

		if (dst->type == Value::ARRAY) {
			if (slice.to >= v.arr.size()) {
				if (define) {
					dst->arr.resize(slice.to+1);
				} else {
					printf("error: range [%zu, %zu) out of bounds for array of size %zu\n", slice.from, slice.to, v.arr.size());
					return;
				}
			}
			for (size_t j = slice.from; j != slice.to; j++) {
				dst->arr[j] = index(v, Value::intOf(j));
			}
		} else if (dst->type == Value::INT) {
			if (v.type != Value::INT) {
				printf("error: cannot assign a non-integer value to an integer bitslice\n");
				return;
			}
			int64_t mask = ((1l<<(slice.to-slice.from))-1l);
			dst->ival = (dst->ival & ~(mask << slice.from)) | ((v.ival & mask) << slice.from);
		}
		printf("internal:%s:%d: slice operator expected array or integer type\n", __FILE__, __LINE__);
		return;
	// TODO(edward.bingham) we should really do typechecking here
	} else {// if (dst->type == v.type or (dst->isUndef() and define)) {
		*dst = v;
	/*} else {
		printf("error:%s:%d: type mismatch between %s:%s and %s:%s\n", __FILE__, __LINE__, dst->ctypeName(), ::to_string(*dst).c_str(), v.ctypeName(), ::to_string(v).c_str());
		throw std::runtime_error("here");*/
	}

	//cout << "Value::set(): done " << *this << " " << *dst << endl;
}

Value Value::get(Slice slice) const {
	Value curr = *this;
	for (int i = 0; i < (int)slice.idx.size(); i++) {
		if (curr.isUndef()) {
			return Value::undef();
		} else if (slice.memb[i] and curr.type != Value::STRUCT) {
			printf("internal:%s:%d: member operator expected struct type\n", __FILE__, __LINE__);
			return Value::X();
		} else if (not slice.memb[i] and curr.type != Value::ARRAY and curr.type != Value::INT) {
			printf("internal:%s:%d: index operator expected array or int type\n", __FILE__, __LINE__);
			return Value::X();
		} else if (slice.idx[i] >= curr.arr.size()) {
			if (not curr.isUnknown()) {
				printf("error: index %zu out of bounds for array of size %zu\n", slice.idx[i], curr.arr.size());
			}
			return Value::undef();
		} else if (curr.type == Value::INT) {
			curr.ival = (curr.ival >> slice.idx[i]) & 1;
		} else {
			curr = curr.arr[slice.idx[i]];
		}
	}

	if (slice.isSlice()) {
		if (curr.isUndef()) {
			return Value::undef();
		} else if (curr.type == Value::ARRAY) {
			if (slice.from >= slice.to or slice.to >= curr.arr.size()) {
				if (not curr.isUnknown()) {
					printf("error: range [%zu, %zu) out of bounds for array of size %zu\n", slice.from, slice.to, curr.arr.size());
				}
				return Value::undef();
			}
			curr.arr = vector<Value>(curr.arr.begin()+slice.from, curr.arr.begin()+slice.to);
		} else if (curr.type == Value::INT) {
			curr.ival = (curr.ival >> slice.from) & ((1<<slice.to)-1);
		} else {
			printf("internal:%s:%d: expected array or integer type\n", __FILE__, __LINE__);
			return Value::X();
		}
	}

	return curr;
}

ValRef::ValRef(Value val, Reference ref) {
	this->val = val;
	this->ref = ref;
}

ValRef::~ValRef() {
}

ostream &operator<<(ostream &os, ValRef lval) {
	os << lval.val << ":" << lval.ref;
	return os;
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
	if (v.isUndef()) {
		os << "?";
	} else if (v.isUnstable()) {
		os << "X";
	} else if (v.isNeutral()) {
		os << "gnd";
	} else if (v.type == Value::WIRE and v.isValid()) {
		os << "vdd";
	} else if (v.isUnknown()) {
		os << "U";
		if (v.type == Value::ARRAY) {
			os << "[";
			for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
				if (i != v.arr.begin()) {
					os << ", ";
				}
				os << *i;
			}
			os << "]";
		}
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

Value isTrue(Value v) {
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

Value isValid(Value v) {
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
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(v0.type);
	} else if (v0.type == Value::ARRAY and v1.type == Value::ARRAY) {
		// concatination
		v0.arr.insert(v0.arr.end(), v1.arr.begin(), v1.arr.end());
		return v0;
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(v0.type);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(v0.type);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::stringOf(v0.sval + v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::intOf(v0.ival + v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::realOf(v0.rval + v1.rval);
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
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval == v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival == v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval == v1.rval);
		}
	}
	printf("error: 'operator==' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator!=(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval != v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival != v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval != v1.rval);
		}
	}
	printf("error: 'operator!=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator<(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval < v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival < v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval < v1.rval);
		}
	}
	printf("error: 'operator<' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator>(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval > v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival > v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval > v1.rval);
		}
	}
	printf("error: 'operator>' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator<=(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval <= v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
			return Value::boolOf(v0.ival <= v1.ival);
		} else if (v0.type == Value::REAL and v1.type == Value::REAL) {
			return Value::boolOf(v0.rval <= v1.rval);
		}
	}
	printf("error: 'operator<=' not defined for '%s' and '%s'\n", v0.ctypeName(), v1.ctypeName());
	return Value::X(Value::BOOL);
}

Value operator>=(Value v0, Value v1) {
	if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X(Value::BOOL);
	} else if (v0.isNeutral() or v1.isNeutral()) {
		return Value::gnd(Value::BOOL);
	} else if (v0.isUnknown() or v1.isUnknown()) {
		return Value::U(Value::BOOL);
	} else if (v0.isValid() and v1.isValid()) {
		if (v0.type == Value::STRING and v1.type == Value::STRING) {
			return Value::boolOf(v0.sval >= v1.sval);
		} else if (v0.type == Value::INT and v1.type == Value::INT) {
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
	} else if (v.isUnstable()) {
		return Value::X();
	} else if (v.type == Value::ARRAY
		or v.type == Value::STRUCT) {
		Value result = Value::vdd();
		for (auto i = v.arr.begin(); i != v.arr.end(); i++) {
			result = result & wireOf(*i);
		}
		return result;
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

ValRef index(ValRef v, Value i) {
	if (i.isUnstable()) {
		return Value::X();
	} else if (i.isNeutral()) {
		return Value::gnd();
	} else if (i.isUnknown()) {
		return Value::U();
	} else if (v.val.type == Value::ARRAY and i.type == Value::INT) {
		if (i.ival >= 0 and i.ival < (int)v.val.arr.size()) {
			v.val = v.val.arr[i.ival];
			if (not v.ref.isUndef()) {
				v.ref.slice.index((size_t)i.ival);
			}
			return v;
		}
		printf("error: index %ld out of bounds for array of size %zu\n", (long)i.ival, v.val.arr.size());
		return Value::X();
	}
	printf("error: 'operator[]' not defined for '%s' and '%s'\n", v.val.ctypeName(), i.ctypeName());
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

ValRef index(ValRef v, Value f, Value t) {
	if (f.isUnstable() or t.isUnstable()) {
		return Value::X(Value::ARRAY);
	} else if (f.isNeutral() or t.isNeutral()) {
		return Value::gnd(Value::ARRAY);
	} else if (f.isUnknown() or t.isUnknown()) {
		return Value::U(Value::ARRAY);
	} else if (v.val.type == Value::ARRAY and f.type == Value::INT and t.type == Value::INT) {
		if (f.ival >= 0 and f.ival < t.ival and t.ival <= (int)v.val.arr.size()) {
			v.val.arr = vector<Value>(v.val.arr.begin()+f.ival, v.val.arr.begin()+t.ival);
			if (not v.ref.isUndef()) {
				v.ref.slice.slice((size_t)f.ival, (size_t)t.ival);
			}
			return v;
		}
		printf("error: range [%ld, %ld) out of bounds for array of size %zu\n", (long)f.ival, (long)t.ival, v.val.arr.size());
		return Value::X(Value::ARRAY);
	}
	printf("error: 'operator[]' not defined for '%s', '%s', and '%s'\n", v.val.ctypeName(), f.ctypeName(), t.ctypeName());
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

ValRef member(ValRef v0, Value v1, TypeSet types) {
	if (v0.val.type == Value::STRUCT and v1.type == Value::STRING) {
		int idx = types.memberIndex(v0.val.sval, v1.sval);
		if (idx >= 0 and idx < (int)v0.val.arr.size()) {
			v0.val = v0.val.arr[idx];
			if (not v0.ref.isUndef()) {
				v0.ref.slice.member((size_t)idx);
			}
			return v0;
		}
		printf("internal: member %s(%d) out of bounds for structure '%s' of size %zu\n", v1.sval.c_str(), idx, v0.val.sval.c_str(), v0.val.arr.size());
		return Value::X();
	}
	printf("error: 'operator.' not defined for '%s' and '%s'\n", v0.val.ctypeName(), v1.ctypeName());
	return Value::X();
}

Value intersect(Value v0, Value v1) {
	if (v0.isUndef()) {
		return v1;
	} else if (v1.isUndef()) {
		return v0;
	} else if (v0.isUnstable() or v1.isUnstable()) {
		return Value::X();
	} else if ((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
		or (v0.type == Value::STRUCT and v1.type == Value::STRUCT and v0.sval == v1.sval)) {
		if (v0.arr.size() < v1.arr.size()) {
			v0.arr.resize(v1.arr.size());
		}
		for (size_t i = 0; i < v1.arr.size(); i++) {
			v0.arr[i] = intersect(v0.arr[i], v1.arr[i]);
		}
		return v0;
	} else if (v0.isUnknown()) {
		return v1;
	} else if (v1.isUnknown()
		or (v0.state == v1.state and v0.state != Value::VALID) // TODO(edward.bingham) remove this to re-enable type checking
		or (v0.type == Value::WIRE and v1.type == Value::WIRE and v0.state == v1.state)
		or (v0.type == Value::BOOL and v1.type == Value::BOOL and v0.state == v1.state and (v0.state != Value::VALID or v0.bval == v1.bval))
		or (v0.type == Value::INT and v1.type == Value::INT and v0.state == v1.state and (v0.state != Value::VALID or v0.ival == v1.ival))
		or (v0.type == Value::REAL and v1.type == Value::REAL and v0.state == v1.state and (v0.state != Value::VALID or v0.rval == v1.rval))) {
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

void localAssign(Value &s0, Value s1, bool stable) {
	if ((s0.type == Value::ARRAY and s1.isUnknown() and s1.type == Value::ARRAY)
		or (s0.type == Value::STRUCT and s1.isUnknown() and s1.type == Value::STRUCT)) {
		for (size_t i = 0; i < s0.arr.size() and i < s1.arr.size(); i++) {
			localAssign(s0.arr[i], s1.arr[i], stable);
		}
		// TODO(edward.bingham) bounds checking
		return;
	}

	if (not s1.isUndef() and not s1.isUnknown()) {
		s0 = stable ? s1 : Value::X();
	}
}

void remoteAssign(Value &s0, Value s1, bool stable) {
	if ((s0.type == Value::ARRAY and s1.isUnknown() and s1.type == Value::ARRAY)
		or (s0.type == Value::STRUCT and s1.isUnknown() and s1.type == Value::STRUCT)) {
		for (size_t i = 0; i < s0.arr.size() and i < s1.arr.size(); i++) {
			remoteAssign(s0.arr[i], s1.arr[i], stable);
		}
		// TODO(edward.bingham) bounds checking
		return;
	}

	if (not s1.isUndef()
		and not s1.isUnknown()
		and not areSame(s0, s1)) {
		s0 = stable ? Value::U() : Value::X();
	}
}

bool vacuousAssign(Value v0, Value v1, bool stable) {
	if (v1.isUndef()) {
		return true;
	}

	if (v1.isUnknown()) {
		if ((v0.type == Value::ARRAY and v1.type == Value::ARRAY)
			or (v0.type == Value::STRUCT and v1.type == Value::STRUCT)) {
			for (size_t i = 0; i < v1.arr.size() and i < v0.arr.size(); i++) {
				if (not vacuousAssign(v0.arr[i], v1.arr[i], stable)) {
					return false;
				}
			}
		}
		return true;
	}
	return areSame(v0, v1) or (stable and v0.isUnstable());
}

}
