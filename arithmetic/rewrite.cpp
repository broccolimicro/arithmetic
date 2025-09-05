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
		(not (not a)) > (isValid(a)),
		(isValid(not a)) > (not a),
		(not isValid(a)) > (not a),
		(isValid(isValid(a))) > (isValid(a)),

		(a and a) > (isValid(a)),
		(a and not a) > (false),
		(true and a) > (isValid(a)),
		(U and a) > (isValid(a)),
		(false and a) > (false),
		(X and a) > (X),
		(isValid(a) and b) > (a and b),
		(isValid(a and b)) > (a and b),
		(a and (-b)) > (a and b),
		(a or (-b)) > (a or b),
		(not (-b)) > (not b),
		(a and (b+c)) > (a and b and c),
		(a or (b+c)) > (a or (b and c)),
		(not (a+b)) > (not a or not b),
		(a and (b*c)) > (a and b and c),
		(a or (b*c)) > (a or (b and c)),
		(not (a*b)) > (not a or not b),
		(a and inv(b)) > (a and b),
		(a or inv(b)) > (a or b),
		(not inv(a)) > (not a),
		(a and (~b)) > (a and b),
		(a or (~b)) > (a or b),
		(not (~b)) > (not b),
		(a and (b & c)) > (a and b and c),
		(a or (b & c)) > (a or (b and c)),
		(not (b & c)) > (not b or not c),
		(a and (b | c)) > (a and b and c),
		(a or (b | c)) > (a or (b and c)),
		(not (b | c)) > (not b or not c),

		(a or a) > (isValid(a)),
		(a or not a) > (true),
		(true or a) > (true),
		(false or a) > (isValid(a)),
		(isValid(a) or b) > (a or b),
		(isValid(a or b)) > (a or b),
		
		(a-b) > (a+(-b)),
		(a+a) > (2*a),
		(a+(-a)) > (0),
		(false+a) > (false),
		(X+a) > (X),
		(0+a) > (a),
		(true+a) > (a),
		(isValid(a+b)) > (a and b),
		(not (a+b)) > (not a or not b),
		(isValid(-a)) > (isValid(a)),

		(a*inv(a)) > (1),
		(a/b) > (a*inv(b)),
		(false*a) > (false),
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),
		(true+a) > (a),
		(isValid(a*b)) > (a and b),
		(not (a*b)) > (not a or not b),
		(isValid(inv(a))) > (isValid(a)),
	
		(a > b) > (b < a),
		(a >= b) > (b <= a),
		(a < b) > isNegative(a-b),
		(a <= b) > not isNegative(b-a),

		(a ^ a) > (0),
		(a & a) > (a),
		(0 & a) > (0),
		(0 | a) > (a),
		(-1 & a) > (a),
		(-1 | a) > (-1),
		(a | a) > (a),
		(~~a) > (a)
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

		(not (not a)) > (isValid(a)),
		(isValid(not a)) > (not a),
		(not isValid(a)) > (not a),
		(isValid(isValid(a))) > (isValid(a)),
		(a and (-b)) > (a and b),
		(a or (-b)) > (a or b),
		(not (-b)) > (not b),
		(a and (b+c)) > (a and b and c),
		(a or (b+c)) > (a or (b and c)),
		(not (a+b)) > (not a or not b),
		(a and (b*c)) > (a and b and c),
		(a or (b*c)) > (a or (b and c)),
		(not (a*b)) > (not a or not b),
		(a and inv(b)) > (a and b),
		(a or inv(b)) > (a or b),
		(not inv(a)) > (not a),
		(a and (~b)) > (a and b),
		(a or (~b)) > (a or b),
		(not (~b)) > (not b),
		(a and (b & c)) > (a and b and c),
		(a or (b & c)) > (a or (b and c)),
		(not (b & c)) > (not b or not c),
		(a and (b | c)) > (a and b and c),
		(a or (b | c)) > (a or (b and c)),
		(not (b | c)) > (not b or not c),


		(a and a) > (isValid(a)),
		(a and not a) > (false),
		(true and a) > (isValid(a)),
		(U and a) > (isValid(a)),
		(false and a) > (false),
		(X and a) > (X),
		(isValid(a) and b) > (a and b),
		(isValid(a and b)) > (a and b),

		(a or a) > (isValid(a)),
		(a or not a) > (true),
		(true or a) > (true),
		(false or a) > (isValid(a)),
		(isValid(a) or b) > (a or b),
		(isValid(a or b)) > (a or b),
		
		(a+a) > (2*a),
		(a+(-a)) > (0),
		(false+a) > (false),
		(X+a) > (X),
		(0+a) > (a),
		(true+a) > (a),
		(isValid(a+b)) > (a and b),
		(not (a+b)) > (not a or not b),
		(isValid(-a)) > (isValid(a)),

		(a*inv(a)) > (1),
		(false*a) > (false),
		(X*a) > (X),
		(0*a) > (0),
		(1*a) > (a),
		(-1*a) > (-a),
		(true+a) > (a),
		(isValid(a*b)) > (a and b),
		(not (a*b)) > (not a or not b),
		(isValid(inv(a))) > (isValid(a)),
	
		(a > b) > (b < a),
		(a >= b) > (b <= a),

		(a ^ a) > (0),
		(a & a) > (a),
		(0 & a) > (0),
		(0 | a) > (a),
		(-1 & a) > (a),
		(-1 | a) > (-1),
		(a | a) > (a),
		(~~a) > (a)
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
		(not booleanAnd(a)) == booleanOr(not a),
		(not booleanOr(a)) == booleanAnd(not a),
		(a and booleanOr(b)) == booleanOr(a and b),
		(~wireAnd(a)) == wireOr(~a),
		(~wireOr(a)) == wireAnd(~a),
		(a & wireOr(b)) == wireOr(a & b),
		(-add(a)) == add(-a),
		(-(a * b)) == ((-a)*b),
		(a * add(b)) == add(a * b),
		booleanXor(a, b) == ((a and (not b)) or ((not a) and b)),
		(a ^ b) == ((a & (~b)) | ((~a) & b)),
	}));

	rules.top = tidy(rules, {rules.top}, true).map(rules.top);
	return rules;
}

/*void valrdyRewrite(vector<Expression> and rewrite) {

}*/

}
