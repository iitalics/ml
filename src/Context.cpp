#include <iostream>
#include "Global.h"
#include "Environment.h"
#include "ValueAllocator.h"


namespace ml {


#ifdef ML_USE_CUSTOM_ALLOCATOR
ValueAllocator* Context::allocator;
#endif
Value* Context::true_ = nullptr;
Value* Context::false_ = nullptr;
Value* Context::void_ = nullptr;


#ifdef ML_USE_CUSTOM_ALLOCATOR
# define CREATE_VALUE		allocator->alloc
#else
# define CREATE_VALUE		new Value
#endif


Context::Context (Value* parentEnv)
{
#ifdef ML_USE_CUSTOM_ALLOCATOR
	if (!allocator)
	{
		allocator = new ValueAllocator();
		MLdebug("using custom allocator");
	}
#endif

	envVal_ = CREATE_VALUE(Value::Type::Environment, this);
	envVal_->env_ = new Environment(parentEnv);
	take_(envVal_);
}
Context::~Context ()
{
	purge();
}


Value* Context::take_ (Value* val)
{
	owned_.push_front(val);
	return val;
}
int Context::purge ()
{
	int p = 0;
	for (auto v : owned_)
	{
		p++;
		v->destroy();
	}
	owned_.clear();
	return p;
}






Value* Context::makeInt (int_t t)
{
	auto v = CREATE_VALUE(Value::Type::Int, this);
	v->int_.value = t;	
	return take_(v);
}
Value* Context::makeReal (real_t t)
{
	auto v = CREATE_VALUE(Value::Type::Real, this);
	v->real_.value = t;	
	return take_(v);
}
Value* Context::makeFunction (const std::string& name, 
				const std::vector<Value::Type>& types, Value::FuncHandler handler)
{
	auto v = CREATE_VALUE(Value::Type::NativeFunc, this);
	int nargs = types.size();
	v->native_ = {	nargs, 
					handler,
					nargs ? new Value::Type[nargs] : nullptr,
					new std::string(name) };
	for (int i = 0; i < nargs; i++)
		v->native_.types[i] = types[i];
	return take_(v);
}
Value* Context::makeFunction (const LambdaFuncData& data)
{
	auto v = CREATE_VALUE(Value::Type::LambdaFunc, this);
	v->lambda_ = new LambdaFuncData(data);
	v->lambda_->env = envVal_;
	return take_(v);
}
Value* Context::apply (Value* func, const std::vector<Value*>& args)
{
	return apply(func, (Value**) args.data(), args.size());
}
Value* Context::apply (Value* func, Value** args, int nargs) 
{
	if (nargs == 0)
		return func;

	auto v = CREATE_VALUE(Value::Type::PartialFunc, this);
	v->partial_.base = func;
	v->partial_.nargs = nargs;
	v->partial_.args = new Value*[nargs];

	for (int i = 0; i < nargs; i++)
		v->partial_.args[i] = args[i];

	return take_(v);
}
Value* Context::makeTrue ()
{
	if (!true_)
	{
		true_ = CREATE_VALUE(Value::Type::Bool);
		true_->bool_ = true;
	}
	return true_;
}
Value* Context::makeFalse ()
{
	if (!false_)
	{
		false_ = CREATE_VALUE(Value::Type::Bool);
		false_->bool_ = false;
	}
	return false_;
}
Value* Context::makeVoid ()
{
	if (!void_)
		void_ = CREATE_VALUE(Value::Type::Void);

	return void_;
}




void Context::takeControl (Value* val, Context* old)
{
	// avoid taking globals or null or malformed commands
	if (val == nullptr || old == nullptr || old == this)
		return;

	// only take what you are meant to take!!
	if (val->owner != old)
		return;

	
	old->loseControl(val);
	
	take_(val)->owner = this;

	if (val->type == Value::Type::Environment)
	{
		// iterate keys
		for (Value* v : *val->env_)
			takeControl(v, old);
		takeControl(val->env_->parentValue(), old);
	}
	if (val->type == Value::Type::PartialFunc)
	{
		for (int i = 0; i < val->partial_.nargs; i++)
			takeControl(val->partial_.args[i], old);
	}
}
void Context::loseControl (Value* val)
{
	val->owner = nullptr;
	owned_.remove(val);
}



void Context::collectGarbage (const std::vector<Value*>& keep)
{
	Context dummy;
	for (auto& v : keep)
		dummy.takeControl(v, this);
	dummy.takeControl(envVal_, this);

	purge(); //  :O !!!

	takeControl(envVal_, &dummy);
	for (auto& v : keep)
		takeControl(v, &dummy);

	// dummy.~Context();
}


};
