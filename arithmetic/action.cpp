#include "action.h"

namespace arithmetic
{

action::action()
{
	behavior = -1;
	variable = -1;
	channel = -1;
}

action::action(Expression expr)
{
	this->variable = -1;
	this->channel = -1;
	this->expr = expr;
	this->behavior = ASSIGN;
}

action::action(int variable, Expression expr)
{
	this->channel = -1;
	this->variable = variable;
	this->behavior = ASSIGN;
	this->expr = expr;
}

action::action(int channel, int variable, Expression expr)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = RECEIVE;
}

action::action(int channel, Expression expr, int variable)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = SEND;
}

action::~action()
{

}

bool action::is_infeasible() const {
	return expr.is_null();
}

bool action::is_vacuous() const {
	return behavior != action::SEND
		and behavior != action::RECEIVE
		and variable < 0
		and expr.is_constant();
}

bool action::is_passive() const {
	return behavior != action::SEND
		and behavior != action::RECEIVE
		and variable < 0;
}

void action::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	if (variable >= 0 and variable < (int)uid_map.size()) {
		variable = uid_map[variable];
	} else {
		variable = -1;
	}

	if (channel >= 0 and channel < (int)uid_map.size()) {
		channel = uid_map[channel];
	} else {
		channel = -1;
	}

	expr.apply(uid_map);
}

ostream &operator<<(ostream &os, const action &a) {
	if (a.behavior == action::ASSIGN) {
		os << "v" << a.variable << "=" << a.expr;
	} else if (a.behavior == action::SEND) {
		os << "c" << a.channel << "!" << a.expr;
	} else if (a.behavior == action::RECEIVE) {
		os << "c" << a.channel << "?v" << a.variable;
	} else {
		os << "skip";
	}
	return os;
}

parallel::parallel()
{

}

parallel::parallel(Expression expr)
{
	actions.push_back(action(expr));
}

parallel::parallel(int variable, Expression expr)
{
	actions.push_back(action(variable, expr));
}

parallel::parallel(int channel, int variable, Expression expr)
{
	actions.push_back(action(channel, variable, expr));
}

parallel::parallel(int channel, Expression expr, int variable)
{
	actions.push_back(action(channel, expr, variable));
}

parallel::~parallel()
{

}

action &parallel::operator[](int index) {
	return actions[index];
}

const action &parallel::operator[](int index) const {
	return actions[index];
}

bool parallel::is_infeasible() const {
	if (actions.empty()) {
		return false;
	}

	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (i->is_infeasible()) {
			return true;
		}
	}
	return false;
}

bool parallel::is_vacuous() const {
	if (actions.empty()) {
		return true;
	}

	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (not i->is_vacuous()) {
			return false;
		}
	}
	return true;
}

bool parallel::is_passive() const {
	if (actions.empty()) {
		return true;
	}

	for (auto i = actions.begin(); i != actions.end(); i++) {
		if (not i->is_passive()) {
			return false;
		}
	}
	return true;
}

/*void parallel::get_mask(vector<int> &cov) const {
	for (auto act = actions.begin(); act != actions.end(); act++) {
		if (act->behavior == action::ASSIGN or act->behavior == action::RECEIVE) {
			cov.push_back(act->variable);
		}
		if (act->behavior == action::SEND or act->behavior == action::RECEIVE) {
			cov.push_back(act->channel);
		}
	}
	sort(cov.begin(), cov.end());
	cov.erase(unique(cov.begin(), cov.end()), cov.end());
}

parallel parallel::mask(vector<int> cov) const {
	parallel p0 = *this;
	for (int i = (int)p0.actions.size()-1; i >= 0; i--) {
		bool remove = false;
		if (p0.actions[i].behavior == action::ASSIGN or p0.actions[i].behavior == action::RECEIVE) {
			remove = find(cov.begin(), cov.end(), p0.actions[i].variable) != cov.end();
		}
		if (p0.actions[i].behavior == action::SEND or p0.actions[i].behavior == action::RECEIVE) {
			remove = find(cov.begin(), cov.end(), p0.actions[i].channel) != cov.end();
		}
		if (remove) {
			p0.actions.erase(p0.actions.begin()+i);
		}
	}
	return p0;
}*/

state parallel::evaluate(const state &curr)
{
	map<int, value> sent; // data sent along the requests
	map<int, value> recv; // data received along the enables
	// Determine the value for the data being sent in either the request or the acknowledge
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].behavior == action::SEND) {
			auto loc = sent.insert(pair<int, value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second = value::X();
			}
		} else if (actions[i].behavior == action::RECEIVE) {
			auto loc = recv.insert(pair<int, value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second = value::X();
			}
		}
	}

	state result;
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].variable < 0) {
			continue;
		}

		if (actions[i].behavior == action::SEND) {
			map<int, value>::iterator loc = recv.find(actions[i].channel);
			if (loc != recv.end()) {
				result.sv_intersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == action::RECEIVE) {
			map<int, value>::iterator loc = sent.find(actions[i].channel);
			if (loc != sent.end()) {
				result.sv_intersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == action::ASSIGN) {
			result.sv_intersect(actions[i].variable, actions[i].expr.evaluate(curr));
		}
	}

	return result;
}

Expression parallel::guard() {
	Expression result(true);
	for (auto a = actions.begin(); a != actions.end(); a++) {
		if (not a->expr.is_constant()) {
			result = result & a->expr;
		}
	}
	return result;
}

void parallel::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	for (int i = 0; i < (int)actions.size(); i++) {
		actions[i].apply(uid_map);
	}
}

ostream &operator<<(ostream &os, const parallel &p) {
	for (int i = 0; i < (int)p.actions.size(); i++) {
		if (i != 0) {
			os << ",";
		}
		os << p.actions[i];
	}
	return os;
}

choice::choice()
{

}

choice::choice(parallel c)
{
	terms.push_back(c);
}

choice::~choice()
{

}

parallel &choice::operator[](int index) {
	return terms[index];
}

const parallel &choice::operator[](int index) const {
	return terms[index];
}

bool choice::is_infeasible() const {
	if (terms.empty()) {
		return true;
	}

	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (not i->is_infeasible()) {
			return false;
		}
	}

	return true;
}

bool choice::is_vacuous() const {
	if (terms.empty()) {
		return false;
	}

	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (i->is_vacuous()) {
			return true;
		}
	}

	return false;
}

bool choice::is_passive() const {
	if (terms.empty()) {
		return false;
	}

	for (auto i = terms.begin(); i != terms.end(); i++) {
		if (not i->is_passive()) {
			return false;
		}
	}

	return true;
}

region choice::evaluate(const state &curr) {
	region result;
	for (auto term = terms.begin(); term != terms.end(); term++) {
		result.states.push_back(term->evaluate(curr));
	}
	return result;
}

Expression choice::guard() {
	if (terms.empty()) {
		return Expression(true);
	}

	Expression result(false);
	for (auto t = terms.begin(); t != terms.end(); t++) {
		result = result | t->guard();
	}
	return result;
}

void choice::apply(vector<int> uid_map) {
	if (uid_map.empty()) {
		return;
	}

	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].apply(uid_map);
	}
}

ostream &operator<<(ostream &os, const choice &c) {
	for (int i = 0; i < (int)c.terms.size(); i++) {
		if (i != 0) {
			os << ":";
		}
		os << c.terms[i];
	}
	return os;
}

}
