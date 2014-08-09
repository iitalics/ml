#include "Global.h"
#include "ValueAllocator.h"
#include <iostream>

namespace ml {

#ifdef ML_USE_CUSTOM_ALLOCATOR

ValueAllocator::ValueAllocator ()
	: slab(new Value[NumValues]),
	  left(0),
	  next(nullptr)
{
	for (int i = 0; i < NumValues; i++)
		slab[i].allocator = nullptr;
}


ValueAllocator::~ValueAllocator ()
{
	delete next;
	delete[] slab;
}


Value* ValueAllocator::alloc (Value::Type t, Context* ctx)
{
	int pos = NumValues;
	int search = 0;

	for (int i = left; i < NumValues; i++)
	{
		left++;
		if (slab[i].allocator == nullptr)
		{
			pos = i;
			break;
		}
		else
			search++;
	}

	if (pos >= NumValues)
	{
		if (next == nullptr)
			next = new ValueAllocator();
		
		return next->alloc(t, ctx);
	}

	Value* out = slab + pos;
	out->type = t;
	out->owner = ctx;
	out->allocator = this;
	return out;
}

void ValueAllocator::destroyed (Value* v)
{
	int pos = v - slab;

	if (pos < 0 || pos >= NumValues) return; // not in slab?!

	if (pos < left)
		left = pos;

	v->allocator = nullptr;
}

#endif
};
