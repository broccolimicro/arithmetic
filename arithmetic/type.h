#pragma once

#include <vector>
#include <array>

namespace arithmetic {

// represents a fixed-point number
struct Type {
	Type();
	Type(double coeff, double width, double delay);
	~Type();

	// DESIGN(edward.bingham) serial vs parallel is an architectural decision,
	// and incurs cost complexities from bit/digit-level pipelining. So for this
	// function, we'll assume that variables represent the "digit" in a serial
	// architecture, or the whole number in a parallel architecture. This means
	// that the cost metrics being computed are only about bit-parallel
	// representations. It's up to the higher level synthesis system to
	// understand the differences between serial and parallel encodings.

	// DESIGN(edward.bingham) we're designing circuits. The design of the
	// whatever floating point operations there are in the system should not be
	// built into the language because it is highly dependent upon the system
	// requirements. However, we can support fixed-point operations readily.

	// DESIGN(edward.bingham) Encodings may not be base-2. They also may not even
	// be a fixed-base encoding. Therefore, we don't want to presuppose and
	// particular base. Our goal is just to be able to compute operator
	// complexity.

	// DESIGN(edward.bingham) Constants are represented by the smallest possible
	// fixed-point representation. This means that the width is always going to
	// be 0.0, and the coefficient is always equal to the value of the constant. 

	double coeff;
	double width;
	double delay;
	std::vector<int> bounds;

	// TODO(edward.bingham) I need to think about operations on arrays. Variables
	// may be multi-dimensional arrays, and Constants can already be
	// multi-dimensional arrays.
};

std::array<double, 2> overlap(Type t0, Type t1);

// Used to represent accumulated cost of an arithmetic expression for
// optimization, trading off area and energy vs operator latency
struct Cost {
	Cost();
	Cost(double complexity, double critical);
	~Cost();

	// the total time-space complexity of the operation
	double complexity;
	// the critical-path delay in gates
	double critical;

	Cost &operator+=(Cost c0);
};

Cost operator+(Cost c0, Cost c1);

}

