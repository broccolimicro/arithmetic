#include "rewrite.h"

namespace arithmetic {

Expression basic_rewrite() {
	// matches all variables and Expressions
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);

	Operand X = Operand::X();
	Operand U = Operand::U(); // matches all valid things including constants

	// true matches valid
	// false matches neutral

	// just create a function that tells whether an operator is commutative, and add logic to check both paths and reorder for canonicalization

	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
 
	// I need to distinguish between canonicalization rules and simplification rules?

	Expression rules = arithmetic::array(vector<Expression>({
		(~~a) == (is_valid(a)),
		(is_valid(~a)) == (~a),
		(~is_valid(a)) == (~a),
		(is_valid(is_valid(a))) == (is_valid(a)),

		(a&a) == (is_valid(a)),
		(a&~a) == (false),
		(true&a) == (is_valid(a)),
		(U&a) == (is_valid(a)),
		(false&a) == (false),
		(X&a) == (X),
		(is_valid(a)&b) == (a&b),
		(is_valid(a&b)) == (a&b),

		(a|a) == (is_valid(a)),
		(a|~a) == (true),
		(true|a) == (true),
		(false|a) == (is_valid(a)),
		(is_valid(a)|b) == (a|b),
		(is_valid(a|b)) == (a|b),
		
		(a & (b & c)) == (Expression("&", {a, b, c})),
		(a | (b | c)) == (Expression("|", {a, b, c})),

		(a-b) == (a+(-b)),
		(a+a) == (2*a),
		(a+(-a)) == (0),
		(false+a) == (false),
		(X+a) == (X),
		(0+a) == (a),
		(true+a) == (a),
		(is_valid(a+b)) == (a&b),
		(~(a+b)) == (~a|~b),
		(a + (b + c)) == (Expression("+", {a, b, c})),

		(a/a) == (1),
		(a/b) == (a*(1/b)),
		(false*a) == (false),
		(X*a) == (X),
		(0*a) == (0),
		(1*a) == (a),
		(-1*a) == (-a),
		(true+a) == (a),
		(is_valid(a*b)) == (a&b),
		(~(a*b)) == (~a|~b),
		(a * (b * c)) == (Expression("*", {a, b, c})),
	
		(a < b) == (a-b < 0),
		(a > b) == (b < a),
		(a <= b) == (a-b-1 < 0),
		(a >= b) == (b <= a),
		(a == b) == (a-b == 0),
		(a != b) == (a-b != 0),

		(a ^ a) == (0),
		(a && a) == (a),
		(0 && a) == (0),
		(0 || a) == (a),
		(-1 && a) == (a),
		(-1 || a) == (-1),
		(a && (b && c)) == (Expression("&&", {a, b, c})),
		(a || a) == (a),
		(a || (b || c)) == (Expression("||", {a, b, c})),
		(!!a) == (a)
	}));

	//	match( ~(a & b), ~a | ~b ),
	//	match( ~(a | b), ~a & ~b ),
	//	match( a & (b | c), (a & b) | (a & c) ),
	//	match( -(a + b), (-a)+(-b) ),
	//	match( -(a * b), (-a)*b ),
	//	match( a * (b + c), (a * b) + (a * c) ),
	//	match( a ^ b, (a && (!b)) || ((!a) && b) ),
	//	match( !(a && b), (!a) || (!b) ),
	//	match( !(a || b), (!a) && (!b) )

	return rules;
}

/*void valrdy_rewrite(vector<Expression> &rewrite) {

}*/

}
