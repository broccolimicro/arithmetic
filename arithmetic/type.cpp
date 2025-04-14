#include "type.h"

#include <cmath>

using namespace std;

namespace arithmetic {

// default to wire
Type::Type() {
	coeff = 0.0;
	width = 0.0;
	delay = 0.0;
}

Type::Type(double coeff, double width, double delay) {
	this->coeff = coeff;
	this->width = width;
	this->delay = delay;
}

Type::~Type() {
}

array<double, 2> overlap(Type t0, Type t1) {
	double off0 = log2(t0.coeff);
	double off1 = log2(t1.coeff);
	return {max(0.0, min(t0.width+off0, t1.width+off1)-max(off0, off1)), max(t0.width+off0, t1.width+off1)-min(off0, off1)};
}

Cost::Cost() {
	complexity = 0.0;
	critical = 0.0;
}

Cost::Cost(double complexity, double critical) {
	this->complexity = complexity;
	this->critical = critical;
}

Cost::~Cost() {
}

Cost &Cost::operator+=(Cost c0) {
	this->complexity += c0.complexity;
	this->critical += c0.critical;
	return *this;
}

Cost operator+(Cost c0, Cost c1) {
	return (c0 += c1);
}

}
