#pragma once
#include "Value.h"

namespace ml {

class Context;

class ValueAllocator
{
public:
	enum
	{
		NumValues = 128
	};
	ValueAllocator ();
	~ValueAllocator ();

	Value* alloc (Value::Type type = Value::Type::Void,
						Context* ctx = nullptr);
	void destroyed (Value* v);
private:
	Value* slab;
	int left;
	ValueAllocator* next;
};

};
