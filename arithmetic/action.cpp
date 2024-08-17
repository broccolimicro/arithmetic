/*
 * action.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: nbingham
 */

#include "action.h"

namespace arithmetic
{

action::action()
{
	behavior = -1;
	variable = -1;
	channel = -1;
}

action::action(expression expr)
{
	this->variable = -1;
	this->channel = -1;
	this->expr = expr;
	this->behavior = assign;
}

action::action(int variable, expression expr)
{
	this->channel = -1;
	this->variable = variable;
	this->behavior = assign;
	this->expr = expr;
}

action::action(int channel, int variable, expression expr)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = receive;
}

action::action(int channel, expression expr, int variable)
{
	this->channel = channel;
	this->variable = variable;
	this->expr = expr;
	this->behavior = send;
}

action::~action()
{

}

parallel::parallel()
{

}

parallel::parallel(expression expr)
{
	actions.push_back(action(expr));
}

parallel::parallel(int variable, expression expr)
{
	actions.push_back(action(variable, expr));
}

parallel::parallel(int channel, int variable, expression expr)
{
	actions.push_back(action(channel, variable, expr));
}

parallel::parallel(int channel, expression expr, int variable)
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

bool parallel::is_tautology() const {
	if (actions.empty()) {
		return true;
	}

	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].behavior == action::send
			or actions[i].behavior == action::receive
			or (actions[i].behavior == action::assign
				and (not actions[i].expr.is_constant()
				or actions[i].expr.evaluate(state()).data != value::unknown))) {
			return false;
		}
	}
	return true;
}

/*void parallel::get_mask(vector<int> &cov) const {
	for (auto act = actions.begin(); act != actions.end(); act++) {
		if (act->behavior == action::assign or act->behavior == action::receive) {
			cov.push_back(act->variable);
		}
		if (act->behavior == action::send or act->behavior == action::receive) {
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
		if (p0.actions[i].behavior == action::assign or p0.actions[i].behavior == action::receive) {
			remove = find(cov.begin(), cov.end(), p0.actions[i].variable) != cov.end();
		}
		if (p0.actions[i].behavior == action::send or p0.actions[i].behavior == action::receive) {
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
		if (actions[i].behavior == action::send) {
			auto loc = sent.insert(pair<int, value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second.data = value::unstable;
			}
		} else if (actions[i].behavior == action::receive) {
			auto loc = recv.insert(pair<int, value>(actions[i].channel, actions[i].expr.evaluate(curr)));
			if (not loc.second) {
				loc.first->second.data = value::unstable;
			}
		}
	}

	state result;
	for (int i = 0; i < (int)actions.size(); i++) {
		if (actions[i].variable < 0) {
			continue;
		}

		if (actions[i].behavior == action::send) {
			map<int, value>::iterator loc = recv.find(actions[i].channel);
			if (loc != recv.end()) {
				result.sv_intersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == action::receive) {
			map<int, value>::iterator loc = sent.find(actions[i].channel);
			if (loc != sent.end()) {
				result.sv_intersect(actions[i].variable, loc->second);
			}
		} else if (actions[i].behavior == action::assign) {
			result.sv_intersect(actions[i].variable, actions[i].expr.evaluate(curr));
		}
	}

	return result;
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

bool choice::is_tautology() const {
	if (terms.empty()) {
		return false;
	}

	for (auto term = terms.begin(); term != terms.end(); term++) {
		if (term->is_tautology()) {
			return true;
		}
	}

	return false;
}

region choice::evaluate(const state &curr) {
	region result;
	for (auto term = terms.begin(); term != terms.end(); term++) {
		result.states.push_back(term->evaluate(curr));
	}
	return result;
}

}
