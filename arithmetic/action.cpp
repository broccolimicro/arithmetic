#include "action.h"

namespace arithmetic
{

Action::Action() {
	variable = -1;
}

Action::Action(Expression expr) {
	this->variable = -1;
	this->expr = expr;
}

Action::Action(int variable, Expression expr) {
	this->variable = variable;
	this->expr = expr;
}

Action::~Action() {
}

bool Action::isInfeasible() const {
	return expr.isNull();
}

bool Action::isVacuous() const {
	return variable < 0
		and expr.isConstant();
}

bool Action::isPassive() const {
	return variable < 0;
}

void Action::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	if (variable >= 0 and variable < (int)uidMap.size()) {
		variable = uidMap[variable];
	} else {
		variable = -1;
	}

	expr.apply(uidMap);
}

ostream &operator<<(ostream &os, const Action &a) {
	if (a.variable >= 0) {
		os << "v" << a.variable << "=";
	}
	os << a.expr;
	return os;
}

Parallel::Parallel() {
}

Parallel::Parallel(Expression expr) {
	actions.push_back(Action(expr));
}

Parallel::Parallel(int variable, Expression expr) {
	actions.push_back(Action(variable, expr));
}

Parallel::~Parallel() {

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
	if (actions.empty()) {
		return true;
	}

	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (not i->isVacuous()) {
			return false;
		}
	}
	return true;
}

bool Parallel::isPassive() const {
	if (actions.empty()) {
		return true;
	}

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

State Parallel::evaluate(const State &curr) {
	// Determine the Value for the data being sent in either the request or the acknowledge

	State result;
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].variable < 0) {
			continue;
		}

		result.svIntersect(actions[i].variable, actions[i].expr.evaluate(curr));
	}

	return result;
}

Expression Parallel::guard() {
	Expression result(true);
	for (auto a = actions.begin(); a != actions.end(); a++) {
		if (not a->expr.isConstant()) {
			result = result & a->expr;
		}
	}
	return result;
}

void Parallel::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)actions.size(); i++) {
		actions[i].apply(uidMap);
	}
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

Choice::Choice(Parallel c)
{
	terms.push_back(c);
}

Choice::~Choice()
{

}

Parallel &Choice::operator[](int index) {
	return terms[index];
}

const Parallel &Choice::operator[](int index) const {
	return terms[index];
}

bool Choice::isInfeasible() const {
	if (terms.empty()) {
		return true;
	}

	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (not i->isInfeasible()) {
			return false;
		}
	}

	return true;
}

bool Choice::isVacuous() const {
	if (terms.empty()) {
		return false;
	}

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

Region Choice::evaluate(const State &curr) {
	Region result;
	for (auto term = terms.begin(); term != terms.end(); term++) {
		result.states.push_back(term->evaluate(curr));
	}
	return result;
}

Expression Choice::guard() {
	if (terms.empty()) {
		return Expression(true);
	}

	Expression result(false);
	for (auto t = terms.begin(); t != terms.end(); t++) {
		result = result | t->guard();
	}
	return result;
}

void Choice::apply(vector<int> uidMap) {
	if (uidMap.empty()) {
		return;
	}

	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].apply(uidMap);
	}
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

}
