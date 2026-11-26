#include "type.h"

#include "algorithm.h"

namespace arithmetic {

Signature::Signature(std::string name, std::vector<Value> params, std::vector<int> bounds) : name(name), params(params), bounds(bounds) {
}

Signature::~Signature() {
}

Instance::Instance(std::string name, Signature type, Value defaultValue) : name(name), type(type), defaultValue(defaultValue) {
}

Instance::~Instance() {
}

Type::Type(std::string name, std::vector<Instance> params, std::vector<Instance> members) : name(name), params(params), members(members) {
}

Type::~Type() {
}

SignatureTemplate::SignatureTemplate(std::string name, std::vector<Expression> params, std::vector<Expression> bounds) : name(name), params(params), bounds(bounds) {
}

SignatureTemplate::SignatureTemplate(Signature sig) {
}

SignatureTemplate::~SignatureTemplate() {
}

Signature SignatureTemplate::evaluate(State values, TypeSet types, Caller caller) const {
	Signature result(name);
	for (const auto &param : this->params) {
		result.params.push_back(arithmetic::evaluate(param, param.top, values, types, caller).val);
	}
	for (const auto &bound : this->bounds) {
		Value sz = arithmetic::evaluate(bound, bound.top, values, types, caller).val;
		if (sz.type != Value::INT) {
			error("", "type expected integer for array bound", __FILE__, __LINE__);
			result.params.push_back(-1);
		} else {
			result.params.push_back(sz.ival);
		}
	}
	return result;
}

InstanceTemplate::InstanceTemplate(std::string name, SignatureTemplate type, Expression defaultValue) : name(name), type(type), defaultValue(defaultValue) {
}

InstanceTemplate::~InstanceTemplate() {
}

Instance InstanceTemplate::evaluate(State values, TypeSet types, Caller caller) const {
	return Instance(name,
		type.evaluate(values, types, caller),
		arithmetic::evaluate(defaultValue, defaultValue.top, values, types, caller).val);
}

TypeTemplate::TypeTemplate(std::string name, std::vector<InstanceTemplate> params, std::vector<InstanceTemplate> members) : name(name), params(params), members(members) {
}

TypeTemplate::~TypeTemplate() {
}

Type TypeTemplate::evaluate(State values, TypeSet types, Caller caller) const {
	Type result(name);
	for (const auto &param : this->params) {
		result.params.push_back(param.evaluate(values, types, caller));
	}
	for (const auto &member : this->members) {
		result.members.push_back(member.evaluate(values, types, caller));
	}
	return result;
}

Signature typeSig() {
	return Signature("type");
}

SignatureTemplate typeTemplate() {
	return SignatureTemplate("type");
}

TypeTemplate typeType() {
	return TypeTemplate("type");
}

Signature chanSig(Value type) {
	return Signature("chan", {type});
}

SignatureTemplate chanTemplate(Expression type) {
	return SignatureTemplate("chan", {type});
}

TypeTemplate chanType() {
	TypeTemplate result("chan");
	result.params.push_back(InstanceTemplate("type", typeSig()));
	return result;
}

Signature wireSig() {
	return Signature("wire");
}

SignatureTemplate wireTemplate() {
	return SignatureTemplate("wire");
}

TypeTemplate wireType() {
	return TypeTemplate("wire");
}

Signature boolSig() {
	return Signature("bool");
}

SignatureTemplate boolTemplate() {
	return SignatureTemplate("bool");
}

TypeTemplate boolType() {
	return TypeTemplate("bool");
}

Signature ufixedSig(uint16_t width, int32_t offset, uint8_t radix) {
	return Signature("ufixed", {Value::intOf(width), Value::intOf(offset), Value::intOf(radix)});
}

Signature fixedSig(uint16_t width, int32_t offset, uint8_t radix) {
	return Signature("fixed", {Value::intOf(width), Value::intOf(offset), Value::intOf(radix)});
}

SignatureTemplate ufixedTemplate(Expression width, Expression offset, Expression radix) {
	return SignatureTemplate("ufixed", {width, offset, radix});
}

SignatureTemplate fixedTemplate(Expression width, Expression offset, Expression radix) {
	return SignatureTemplate("fixed", {width, offset, radix});
}

TypeTemplate ufixedType() {
	TypeTemplate result("ufixed");
	result.params.push_back(InstanceTemplate("width", ufixedSig(16)));
	result.params.push_back(InstanceTemplate("offset", fixedSig(32), Expression::intOf(0)));
	result.params.push_back(InstanceTemplate("radix", ufixedSig(8), Expression::intOf(2)));
	return result;
}

TypeTemplate fixedType() {
	TypeTemplate result("fixed");
	result.params.push_back(InstanceTemplate("width", ufixedSig(16)));
	result.params.push_back(InstanceTemplate("offset", fixedSig(32), Expression::intOf(0)));
	result.params.push_back(InstanceTemplate("radix", ufixedSig(8), Expression::intOf(2)));
	return result;
}

}
