#include "rewrite.h"

namespace arithmetic {

Expression rewriteBasic() {
	// matches all variables and Expressions
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);

	Operand X = Operand::X();
	Operand U = Operand::U();

	// true matches valid
	// false matches neutral
	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
 
	Expression rules = arithmetic::array(vector<Expression>({
		(~(~a)) > (is_valid(a)),
		(is_valid(~a)) > (~a),
		(~is_valid(a)) > (~a),
		(is_valid(is_valid(a))) > (is_valid(a)),

		(a&a) > (is_valid(a)),
		(a&~a) > (false),
		(true&a) > (is_valid(a)),
		(U&a) > (is_valid(a)),
		(false&a) > (false),
		(X&a) > (X),
		(is_valid(a)&b) > (a&b),
		(is_valid(a&b)) > (a&b),

		(a|a) > (is_valid(a)),
		(a|~a) > (true),
		(true|a) > (true),
		(false|a) > (is_valid(a)),
		(is_valid(a)|b) > (a|b),
		(is_valid(a|b)) > (a|b),
		
		(a & booleanAnd(b)) > (a & b),
		(a | booleanOr(b)) > (a | b),

		(a-b) > (a+(-b)),
		(a+a) > (2*a),
		(a+(-a)) > (0),
		(false+a) > (false),
		(X+a) > (X),
		(0+a) > (a),
		(true+a) > (a),
		(is_valid(a+b)) > (a&b),
		(~(a+b)) > (~a|~b),
		(a + add(b)) > (a + b),

		(a*inv(a)) > (1),
		(a/b) > (a*inv(b)),
		(false*a) > (false),
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),
		(true+a) > (a),
		(is_valid(a*b)) > (a&b),
		(~(a*b)) > (~a|~b),
		(a * mult(b)) > (a * b),
	
		(a < b) > (a-b < 0),
		(a > b) > (b < a),
		(a <= b) > (a-b-1 < 0),
		(a >= b) > (b <= a),
		(a > b) > (a-b > 0),
		(a != b) > (a-b != 0),

		(a ^ a) > (0),
		(a && a) > (a),
		(0 && a) > (0),
		(0 || a) > (a),
		(-1 && a) > (a),
		(-1 || a) > (-1),
		(a && bitwiseAnd(b)) > (a && b),
		(a || a) > (a),
		(a || bitwiseOr(b)) > (a || b),
		(!!a) > (a)
	}));

	return rules.canonicalize(true);
}

Expression rewriteUndirected() {
	// matches all variables and Expressions
	Operand a = Operand::varOf(0);
	Operand b = Operand::varOf(1);
	Operand c = Operand::varOf(2);

	Operand X = Operand::X();
	Operand U = Operand::U();

	Expression rules = arithmetic::array(vector<Expression>({
		(~booleanAnd(a)) == booleanOr(~a),
		(~booleanOr(a)) == booleanAnd(~a),
		(a & booleanOr(b)) == booleanOr(a & b),
		(!bitwiseAnd(a)) == bitwiseOr(!a),
		(!bitwiseOr(a)) == bitwiseAnd(!a),
		(a && bitwiseOr(b)) == bitwiseOr(a && b),
		(-add(a)) == add(-a),
		(-(a * b)) == ((-a)*b),
		(a * add(b)) == add(a * b),
		(a ^ b) == ((a & (~b)) | ((~a) & b)),
		bitwiseXor(a, b) == ((a && (!b)) || ((!a) && b)),
	}));

	rules.canonicalize(true);
	
	return rules;
}

/*void valrdy_rewrite(vector<Expression> &rewrite) {

}*/

}
