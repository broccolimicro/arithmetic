#pragma once

#include <vector>
#include <array>
#include <string>

#include "expression.h"
#include "value.h"

namespace arithmetic {

struct Signature {
	Signature(std::string name, std::vector<Value> params=std::vector<Value>(), std::vector<int> bounds=std::vector<int>());
	~Signature();

	std::string name;
	std::vector<Value> params;
	std::vector<int> bounds;
};

struct Instance {
	Instance(std::string name, Signature type, Value defaultValue=Value::undef());
	~Instance();

	std::string name;
	Signature type;
	Value defaultValue;
};

struct Type {
	Type(std::string name, std::vector<Instance> params=std::vector<Instance>(), std::vector<Instance> members=std::vector<Instance>());
	~Type();

	std::string name;
	std::vector<Instance> params;
	std::vector<Instance> members;
};

struct SignatureTemplate {
	SignatureTemplate(std::string name, std::vector<Expression> params=std::vector<Expression>(), std::vector<Expression> bounds=std::vector<Expression>());
	SignatureTemplate(Signature sig);
	~SignatureTemplate();

	std::string name;
	std::vector<Expression> params;
	std::vector<Expression> bounds;

	Signature evaluate(State values, TypeSet types, Caller caller) const;
};

struct InstanceTemplate {
	InstanceTemplate(std::string name, SignatureTemplate type, Expression defaultValue=Expression::undef());
	InstanceTemplate(Instance inst);
	~InstanceTemplate();

	std::string name;
	SignatureTemplate type;
	Expression defaultValue;

	Instance evaluate(State values, TypeSet types, Caller caller) const;
};

struct TypeTemplate {
	TypeTemplate(std::string name, std::vector<InstanceTemplate> params=std::vector<InstanceTemplate>(), std::vector<InstanceTemplate> members=std::vector<InstanceTemplate>());
	TypeTemplate(Type type);
	~TypeTemplate();

	std::string name;
	std::vector<InstanceTemplate> params;
	std::vector<InstanceTemplate> members;

	Type evaluate(State values, TypeSet types, Caller caller) const;
};

// Bootstrapping
Signature typeSig();
SignatureTemplate typeTemplate();
TypeTemplate typeType();

Signature chanSig(Value type);
SignatureTemplate chanTemplate(Expression type);
TypeTemplate chanType();

Signature wireSig();
SignatureTemplate wireTemplate();
TypeTemplate wireType();

Signature boolSig();
SignatureTemplate boolTemplate();
TypeTemplate boolType();

Signature ufixedSig(uint16_t width, int32_t offset=0, uint8_t radix=2);
Signature fixedSig(uint16_t width, int32_t offset=0, uint8_t radix=2);
SignatureTemplate ufixedTemplate(Expression width, Expression offset=Expression::intOf(0), Expression radix=Expression::intOf(2));
SignatureTemplate fixedTemplate(Expression width, Expression offset=Expression::intOf(0), Expression radix=Expression::intOf(2));
TypeTemplate ufixedType();
TypeTemplate fixedType();

}

