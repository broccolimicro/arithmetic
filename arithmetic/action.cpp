#include "action.h"

namespace arithmetic
{

Action::Action()
{
	behavior = -1;
	variable = -1;
	channel = -1;
}

Action::Action(Expression expr)
{
	this->variable = -1;
	this->channel = -1;
	this->expr = expr;
	this->behavior = ASSIGN;
}

Action::Action(int variable, Expression expr)
{
	this->channel = -1;
	this->variable = variable;
	this->behavior = ASSIGN;
	this->expr = expr;
}

Action::Action(int channel, int variable, Expression expr)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = RECEIVE;
}

Action::Action(int channel, Expression expr, int variable)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = SEND;
}

Action::~Action()
{

}

bool Action::isInfeasible() const {
	return expr.isNull();
}

bool Action::isVacuous() const {
	return behavior != Action::SEND
		and behavior != Action::RECEIVE
		and variable < 0
		and expr.isConstant();
}

bool Action::isPassive() const {
	return behavior != Action::SEND
		and behavior != Action::RECEIVE
		and variable < 0;
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

	if (channel >= 0 and channel < (int)uidMap.size()) {
		channel = uidMap[channel];
	} else {
		channel = -1;
	}

	expr.apply(uidMap);
}

ostream &operator<<(ostream &os, const Action &a) {
	if (a.behavior == Action::ASSIGN) {
		os << "v" << a.variable << "=" << a.expr;
	} else if (a.behavior == Action::SEND) {
		os << "c" << a.channel << "!" << a.expr;
	} else if (a.behavior == Action::RECEIVE) {
		os << "c" << a.channel << "?v" << a.variable;
	} else {
		os << "skip";
	}
	return os;
}

Parallel::Parallel()
{

}

Parallel::Parallel(Expression expr)
{
	actions.push_back(Action(expr));
}

Parallel::Parallel(int variable, Expression expr)
{
	actions.push_back(Action(variable, expr));
}

Parallel::Parallel(int channel, int variable, Expression expr)
{
	actions.push_back(Action(channel, variable, expr));
}

Parallel::Parallel(int channel, Expression expr, int variable)
{
	actions.push_back(Action(channel, expr, variable));
}

Parallel::~Parallel()
{

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
		if (act->behavior == Action::ASSIGN or act->behavior == Action::RECEIVE) {
			cov.push_back(act->variable);
		}
		if (act->behavior == Action::SEND or act->behavior == Action::RECEIVE) {
			cov.push_back(act->channel);
		}
	}
	sort(cov.begin(), cov.end());
	cov.erase(unique(cov.begin(), cov.end()), cov.end());
}

Parallel Parallel::mask(vector<int> cov) const {
	Parallel p0 = *this;
	for (int i = (int)p0.actions.size()-1; i >= 0; i--) {
		bool remove = false;
		if (p0.actions[i].behavior == Action::ASSIGN or p0.actions[i].behavior == Action::RECEIVE) {
			remove = find(cov.begin(), cov.end(), p0.actions[i].variable) != cov.end();
		}
		if (p0.actions[i].behavior == Action::SEND or p0.actions[i].behavior == Action::RECEIVE) {
			remove = find(cov.begin(), cov.end(), p0.actions[i].channel) != cov.end();
		}
		if (remove) {
			p0.actions.erase(p0.actions.begin()+i);
		}
	}
	return p0;
}*/

State Parallel::evaluate(const State &curr)
{
	map<int, Value> sent; // data sent along the requests
	map<int, Value> recv; // data received along the enables
	// Determine the Value for the data being sent in either the request or the acknowledge
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].behavior == Action::SEND) {
			auto loc = sent.insert(pair<int, Value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second = Value::X();
			}
		} else if (actions[i].behavior == Action::RECEIVE) {
			auto loc = recv.insert(pair<int, Value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second = Value::X();
			}
		}
	}

	State result;
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].variable < 0) {
			continue;
		}

		if (actions[i].behavior == Action::SEND) {
			map<int, Value>::iterator loc = recv.find(actions[i].channel);
			if (loc != recv.end()) {
				result.svIntersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == Action::RECEIVE) {
			map<int, Value>::iterator loc = sent.find(actions[i].channel);
			if (loc != sent.end()) {
				result.svIntersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == Action::ASSIGN) {
			result.svIntersect(actions[i].variable, actions[i].expr.evaluate(curr));
		}
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
