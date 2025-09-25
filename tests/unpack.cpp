#include <gtest/gtest.h>

#include <arithmetic/algorithm.h>
#include <arithmetic/expression.h>
#include <arithmetic/rewrite.h>
#include <common/mapping.h>
#include <common/text.h>

using namespace arithmetic;
using namespace std;

/*TEST(Unpack, Arrays) {
	


}*/

TEST(Unpack, Structs) {
	Expression a = Expression::varOf(0);
	Expression b = Expression::varOf(1);
	Expression c = Expression::varOf(2);
	Expression d = Expression::varOf(3);

	auto rules = RuleSet({
		(isValid(construct("test", {a, b}))) > a,
		(~construct("test", {a, b})) > (~a),
		(construct("test", {a, b}) & construct("test", {c, d})) > (a&c),
		(construct("test", {a, b}) | construct("test", {c, d})) > (a|c),

		(isTrue(construct("test", {a, b}))) > (a&isTrue(b)),
		(!construct("test", {a, b})) > construct("test", {a, !b}),
		(construct("test", {a, b}) && construct("test", {c, d})) > construct("test", {a&c, b&&d}),
		(construct("test", {a, b}) || construct("test", {c, d})) > construct("test", {((a&isTrue(b))|(c&isTrue(d))), b||d}),

		(construct("test", {a, b}) + construct("test", {c, d})) > construct("test", {a&c, b+d}),
		(construct("test", {a, b}) - construct("test", {c, d})) > construct("test", {a&c, b-d}),
		(construct("test", {a, b}) * construct("test", {c, d})) > construct("test", {a&c, b*d}),
		(construct("test", {a, b}) / construct("test", {c, d})) > construct("test", {a&c, b/d}),
		
		(construct("test", {a, b}) == construct("test", {c, d})) > construct("test", {a&c, b==d}),
		(construct("test", {a, b}) != construct("test", {c, d})) > construct("test", {a&c, b!=d}),
		(construct("test", {a, b}) < construct("test", {c, d})) > construct("test", {a&c, b<d}),
		(construct("test", {a, b}) > construct("test", {c, d})) > construct("test", {a&c, b>d}),
		(construct("test", {a, b}) <= construct("test", {c, d})) > construct("test", {a&c, b<=d}),
		(construct("test", {a, b}) >= construct("test", {c, d})) > construct("test", {a&c, b>=d}),
	});

	Expression x = construct("test", {Expression::varOf(0), Expression::varOf(1)});
	Expression y = construct("test", {Expression::varOf(2), Expression::varOf(3)});
	Expression z = construct("test", {Expression::varOf(4), Expression::varOf(5)});
	Expression w = construct("test", {Expression::varOf(6), Expression::varOf(7)});

	Expression dut = isTrue(x==y||z==w);
	cout << dut << endl;
	dut.minimize(rules);
	dut.minimize();
	cout << dut << endl;
}

