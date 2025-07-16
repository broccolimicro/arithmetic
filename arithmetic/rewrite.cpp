#include "rewrite.h"
#include "algorithm.h"

namespace arithmetic {

Expression rewriteBasic() {
	// matches all variables and Expressions
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();

	// true matches valid
	// false matches neutral
	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
 
	Expression rules = arithmetic::array(vector<Expression>({
		(~(~a)) > (isValid(a)),
		(isValid(~a)) > (~a),
		(~isValid(a)) > (~a),
		(isValid(isValid(a))) > (isValid(a)),

		(a&a) > (isValid(a)),
		(a&~a) > (false),
		(true&a) > (isValid(a)),
		(U&a) > (isValid(a)),
		(false&a) > (false),
		(X&a) > (X),
		(isValid(a)&b) > (a&b),
		(isValid(a&b)) > (a&b),
		(a & (-b)) > (a&b),
		(a | (-b)) > (a|b),
		(~(-b)) > (~b),
		(a & (b+c)) > (a & b & c),
		(a | (b+c)) > (a | (b & c)),
		(~(a+b)) > (~a | ~b),
		(a & (b*c)) > (a & b & c),
		(a | (b*c)) > (a | (b & c)),
		(~(a*b)) > (~a | ~b),
		(a & inv(b)) > (a & b),
		(a | inv(b)) > (a | b),
		(~inv(a)) > (~a),
		(a & (!b)) > (a & b),
		(a | (!b)) > (a | b),
		(~(!b)) > (~b),
		(a & (b && c)) > (a & b & c),
		(a | (b && c)) > (a | (b & c)),
		(~(b && c)) > (~b | ~c),
		(a & (b || c)) > (a & b & c),
		(a | (b || c)) > (a | (b & c)),
		(~(b || c)) > (~b | ~c),

		(a|a) > (isValid(a)),
		(a|~a) > (true),
		(true|a) > (true),
		(false|a) > (isValid(a)),
		(isValid(a)|b) > (a|b),
		(isValid(a|b)) > (a|b),
		
		(a & booleanAnd(b)) > (a & b),
		(a | booleanOr(b)) > (a | b),

		(a-b) > (a+(-b)),
		(a+a) > (2*a),
		(a+(-a)) > (0),
		(false+a) > (false),
		(X+a) > (X),
		(0+a) > (a),
		(true+a) > (a),
		(isValid(a+b)) > (a&b),
		(~(a+b)) > (~a|~b),
		(a + add(b)) > (a + b),
		(isValid(-a)) > (isValid(a)),

		(a*inv(a)) > (1),
		(a/b) > (a*inv(b)),
		(false*a) > (false),
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),
		(true+a) > (a),
		(isValid(a*b)) > (a&b),
		(~(a*b)) > (~a|~b),
		(a * mult(b)) > (a * b),
		(isValid(inv(a))) > (isValid(a)),
	
		(a > b) > (b < a),
		(a >= b) > (b <= a),
		(a < b) > isNegative(a-b),
		(a <= b) > ~isNegative(b-a),

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

	rules.top = tidy(rules, {rules.top}, true).map(rules.top);
	return rules;
}

Expression rewriteHuman() {
	// matches all variables and Expressions
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();

	// true matches valid
	// false matches neutral
	// All rewrites on only constants handled explicitly (without rewrite rules, using value)
 
	Expression rules = arithmetic::array(vector<Expression>({
		(a + (-b)) > (a - b),
		(isNegative(a)) > (a < 0),
		(a-b < c) > (a < b+c),
		(-b+a) > (a-b),
		(inv(a)) > (1/a),
		(a/(b/c)) > (a*c/b),

		(~(~a)) > (isValid(a)),
		(isValid(~a)) > (~a),
		(~isValid(a)) > (~a),
		(isValid(isValid(a))) > (isValid(a)),
		(a & (-b)) > (a&b),
		(a | (-b)) > (a|b),
		(~(-b)) > (~b),
		(a & (b+c)) > (a & b & c),
		(a | (b+c)) > (a | (b & c)),
		(~(a+b)) > (~a | ~b),
		(a & (b*c)) > (a & b & c),
		(a | (b*c)) > (a | (b & c)),
		(~(a*b)) > (~a | ~b),
		(a & inv(b)) > (a & b),
		(a | inv(b)) > (a | b),
		(~inv(a)) > (~a),
		(a & (!b)) > (a & b),
		(a | (!b)) > (a | b),
		(~(!b)) > (~b),
		(a & (b && c)) > (a & b & c),
		(a | (b && c)) > (a | (b & c)),
		(~(b && c)) > (~b | ~c),
		(a & (b || c)) > (a & b & c),
		(a | (b || c)) > (a | (b & c)),
		(~(b || c)) > (~b | ~c),


		(a&a) > (isValid(a)),
		(a&~a) > (false),
		(true&a) > (isValid(a)),
		(U&a) > (isValid(a)),
		(false&a) > (false),
		(X&a) > (X),
		(isValid(a)&b) > (a&b),
		(isValid(a&b)) > (a&b),

		(a|a) > (isValid(a)),
		(a|~a) > (true),
		(true|a) > (true),
		(false|a) > (isValid(a)),
		(isValid(a)|b) > (a|b),
		(isValid(a|b)) > (a|b),
		
		(a & booleanAnd(b)) > (a & b),
		(a | booleanOr(b)) > (a | b),

		(a+a) > (2*a),
		(a+(-a)) > (0),
		(false+a) > (false),
		(X+a) > (X),
		(0+a) > (a),
		(true+a) > (a),
		(isValid(a+b)) > (a&b),
		(~(a+b)) > (~a|~b),
		(a + add(b)) > (a + b),
		(isValid(-a)) > (isValid(a)),

		(a*inv(a)) > (1),
		(false*a) > (false),
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),
		(true+a) > (a),
		(isValid(a*b)) > (a&b),
		(~(a*b)) > (~a|~b),
		(a * mult(b)) > (a * b),
		(isValid(inv(a))) > (isValid(a)),
	
		(a > b) > (b < a),
		(a >= b) > (b <= a),

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

	rules.top = tidy(rules, {rules.top}, true).map(rules.top);
	return rules;
}

Expression rewriteUndirected() {
	// matches all variables and Expressions
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);

	Expression X = Expression::X();
	Expression U = Expression::U();

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

	rules.top = tidy(rules, {rules.top}, true).map(rules.top);
	return rules;
}

/*void valrdyRewrite(vector<Expression> &rewrite) {

}*/

}
