#include "action.h"
#include "algorithm.h"

namespace arithmetic
{

Action::Action() {
	this->lvalue = Expression::vdd();
	this->rvalue = Expression::vdd();
}

Action::Action(Expression rvalue) {
	this->lvalue = Expression::vdd();
	this->rvalue = rvalue;
}

Action::Action(Expression lvalue, Expression rvalue) {
	this->lvalue = lvalue;
	this->rvalue = rvalue;
}

Action::~Action() {
}

bool Action::isInfeasible() const {
	return rvalue.isNull();
}

bool Action::isVacuous() const {
	return lvalue.isConstant()
		and rvalue.isConstant();
}

bool Action::isPassive() const {
	return lvalue.isConstant();
}

void Action::applyVars(const Mapping<size_t> &m) {
	lvalue.applyVars(m);
	rvalue.applyVars(m);
}

void Action::applyVars(const Mapping<int> &m) {
	lvalue.applyVars(m);
	rvalue.applyVars(m);
}

void Action::evaluate(State &next, const State &curr, TypeSet types) {
	if (lvalue.isUndef()) {
		cout << "Action::evaluate(): undef lvalue" << endl;
		return;
	}
	next.svIntersect(
		arithmetic::evaluate(lvalue, lvalue.top, curr, types).ref,
		arithmetic::evaluate(rvalue, rvalue.top, curr, types).val);
}

bool areSame(Action a0, Action a1) {
	return areSame(a0.lvalue, a1.lvalue) and areSame(a0.rvalue, a1.rvalue);
}

ostream &operator<<(ostream &os, const Action &a) {
	os << a.lvalue << "=" << a.rvalue;
	return os;
}

Parallel::Parallel() {
}

Parallel::Parallel(Expression expr) {
	actions.push_back(Action(expr));
}

Parallel::Parallel(Expression lvalue, Expression rvalue) {
	actions.push_back(Action(lvalue, rvalue));
}

Parallel::Parallel(std::initializer_list<Action> exprs) : actions(exprs) {
}

Parallel::~Parallel() {

}

vector<Action>::iterator Parallel::begin() {
	return actions.begin();
}

vector<Action>::iterator Parallel::end() {
	return actions.end();
}

vector<Action>::const_iterator Parallel::begin() const {
	return actions.begin();
}

vector<Action>::const_iterator Parallel::end() const {
	return actions.end();
}

Action &Parallel::operator[](int index) {
	return actions[index];
}

const Action &Parallel::operator[](int index) const {
	return actions[index];
}

bool Parallel::isInfeasible() const {
	if (actions.empty()) {
		return false;
	}

	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (i->isInfeasible()) {
			return true;
		}
	}
	return false;
}

bool Parallel::isVacuous() const {
	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (not i->isVacuous()) {
			return false;
		}
	}
	return true;
}

bool Parallel::isPassive() const {
	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (not i->isPassive()) {
			return false;
		}
	}
	return true;
}

/*void Parallel::getMask(vector<int> &cov) const {
	for (auto act = actions.begin(); act != actions.end(); act++) {
		cov.push_back(act->variable);
	}
	sort(cov.begin(), cov.end());
	cov.erase(unique(cov.begin(), cov.end()), cov.end());
}

Parallel Parallel::mask(vector<int> cov) const {
	Parallel p0 = *this;
	for (int i = (int)p0.actions.size()-1; i >= 0; i--) {
		if (find(cov.begin(), cov.end(), p0.actions[i].variable) != cov.end()) {
			p0.actions.erase(p0.actions.begin()+i);
		}
	}
	return p0;
}*/

State Parallel::evaluate(const State &curr, TypeSet types) {
	State result;
	for (auto i = actions.begin(); i != actions.end(); i++) {
		i->evaluate(result, curr, types);
	}
	return result;
}

Expression Parallel::guard() {
	Expression result = Expression::vdd();
	for (auto a = actions.begin(); a != actions.end(); a++) {
		if (not a->rvalue.isConstant()) {
			result = result & a->rvalue;
		}
	}
	return result;
}

void Parallel::applyVars(const Mapping<size_t> &m) {
	for (int i = 0; i < (int)actions.size(); i++) {
		actions[i].applyVars(m);
	}
}

void Parallel::applyVars(const Mapping<int> &m) {
	for (int i = 0; i < (int)actions.size(); i++) {
		actions[i].applyVars(m);
	}
}

Parallel &Parallel::operator&=(const Action &c0) {
	actions.push_back(c0);
	return *this;
}

Parallel &Parallel::operator&=(const Parallel &c0) {
	actions.insert(actions.end(), c0.actions.begin(), c0.actions.end());
	return *this;
}

bool areSame(Parallel p0, Parallel p1) {
	if (p0.actions.size() != p1.actions.size()) {
		return false;
	}

	for (int i = 0; i < (int)p0.actions.size(); i++) {
		if (not areSame(p0.actions[i], p1.actions[i])) {
			return false;
		}
	}
	return true;
}

ostream &operator<<(ostream &os, const Parallel &p) {
	for (int i = 0; i < (int)p.actions.size(); i++) {
		if (i != 0) {
			os << ",";
		}
		os << p.actions[i];
	}
	return os;
}

Choice::Choice()
{

}

Choice::Choice(bool skip) {
	if (skip) {
		terms.push_back(Parallel());
	}
}

Choice::Choice(std::initializer_list<Parallel> exprs) : terms(exprs) {
}

Choice::~Choice()
{

}

vector<Parallel>::iterator Choice::begin() {
	return terms.begin();
}

vector<Parallel>::iterator Choice::end() {
	return terms.end();
}

vector<Parallel>::const_iterator Choice::begin() const {
	return terms.begin();
}

vector<Parallel>::const_iterator Choice::end() const {
	return terms.end();
}

Parallel &Choice::operator[](int index) {
	return terms[index];
}

const Parallel &Choice::operator[](int index) const {
	return terms[index];
}

bool Choice::isInfeasible() const {
	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (not i->isInfeasible()) {
			return false;
		}
	}

	return true;
}

bool Choice::isVacuous() const {
	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (i->isVacuous()) {
			return true;
		}
	}

	return false;
}

bool Choice::isPassive() const {
	if (terms.empty()) {
		return false;
	}

	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (not i->isPassive()) {
			return false;
		}
	}

	return true;
}

Region Choice::evaluate(const State &curr, TypeSet types) {
	Region result;
	for (auto i = terms.begin(); i != terms.end(); i++) {
		result.states.push_back(i->evaluate(curr, types));
	}
	return result;
}

Expression Choice::guard() {
	if (terms.empty()) {
		return Expression::vdd();
	}

	Expression result = Expression::gnd();
	for (auto t = terms.begin(); t != terms.end(); t++) {
		result = result | t->guard();
	}
	return result;
}

void Choice::applyVars(const Mapping<size_t> &m) {
	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].applyVars(m);
	}
}

void Choice::applyVars(const Mapping<int> &m) {
	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].applyVars(m);
	}
}

Choice &Choice::operator&=(const Action &c0) {
	for (auto i = terms.begin(); i != terms.end(); i++) {
		*i &= c0;
	}
	return *this;
}

Choice &Choice::operator&=(const Parallel &c0) {
	for (auto i = terms.begin(); i != terms.end(); i++) {
		*i &= c0;
	}
	return *this;
}

Choice &Choice::operator&=(const Choice &c0) {
	return (*this = *this & c0);
}

Choice &Choice::operator|=(const Action &c0) {
	terms.push_back({c0});
	return *this;
}

Choice &Choice::operator|=(const Parallel &c0) {
	terms.push_back(c0);
	return *this;
}

Choice &Choice::operator|=(const Choice &c0) {
	terms.insert(terms.end(), c0.terms.begin(), c0.terms.end());
	return *this;
}

bool areSame(Choice c0, Choice c1) {
	if (c0.terms.size() != c1.terms.size()) {
		return false;
	}

	for (int i = 0; i < (int)c0.terms.size(); i++) {
		if (not areSame(c0.terms[i], c1.terms[i])) {
			return false;
		}
	}
	return true;
}

ostream &operator<<(ostream &os, const Choice &c) {
	for (int i = 0; i < (int)c.terms.size(); i++) {
		if (i != 0) {
			os << ":";
		}
		os << c.terms[i];
	}
	return os;
}

Parallel operator&(const Action &c0, const Action &c1) {
	return Parallel({c0, c1});
}

Parallel operator&(const Action &c0, const Parallel &c1) {
	Parallel result({c0});
	result &= c1;
	return result;
}

Parallel operator&(Parallel c0, const Action &c1) {
	c0 &= c1;
	return c0;
}

Parallel operator&(Parallel c0, const Parallel &c1) {
	c0 &= c1;
	return c0;
}

Choice operator|(const Action &c0, const Action &c1) {
	return Choice({{c0}, {c1}});
}

Choice operator|(const Action &c0, const Parallel &c1) {
	return Choice({{c0}, c1});
}

Choice operator|(const Parallel &c0, const Action &c1) {
	return Choice({c0, {c1}});
}

Choice operator|(const Parallel &c0, const Parallel &c1) {
	return Choice({c0, c1});
}

Choice operator&(const Action &c0, const Choice &c1) {
	Choice result;
	for (auto i = c1.begin(); i != c1.end(); i++) {
		result.terms.push_back(c0 & *i);
	}
	return result;
}

Choice operator&(Choice c0, const Action &c1) {
	for (auto i = c0.begin(); i != c0.end(); i++) {
		*i &= c1;
	}
	return c0;
}

Choice operator&(const Parallel &c0, const Choice &c1) {
	Choice result;
	for (auto i = c1.begin(); i != c1.end(); i++) {
		result.terms.push_back(c0 & *i);
	}
	return result;
}

Choice operator&(Choice c0, const Parallel &c1) {
	for (auto i = c0.begin(); i != c0.end(); i++) {
		*i &= c1;
	}
	return c0;
}

Choice operator&(const Choice &c0, const Choice &c1) {
	Choice result;
	for (auto i = c0.begin(); i != c0.end(); i++) {
		for (auto j = c1.begin(); j != c1.end(); j++) {
			result.terms.push_back(*i & *j);
		}
	}
	return result;
}

Choice operator|(const Action &c0, const Choice &c1) {
	Choice result({{c0}});
	result |= c1;
	return result;
}

Choice operator|(Choice c0, const Action &c1) {
	c0 |= c1;
	return c0;
}

Choice operator|(const Parallel &c0, const Choice &c1) {
	Choice result({c0});
	result |= c1;
	return result;
}

Choice operator|(Choice c0, const Parallel &c1) {
	c0 |= c1;
	return c0;
}

Choice operator|(Choice c0, const Choice &c1) {
	c0 |= c1;
	return c0;
}

}
