#include "Global.h"
#include "Value.h"
#include "Context.h"
#include "Environment.h"
#include "Error.h"
#include "Expression.h"
#include "ValueAllocator.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include "left_vector.hpp"

namespace ml {



Value::Value (Type t, Context* ctx)
	: type(t), owner(ctx)
#ifdef ML_USE_CUSTOM_ALLOCATOR
	  ,allocator(nullptr)
#endif
{ }

Value::~Value ()
{
#ifdef ML_USE_CUSTOM_ALLOCATOR
	if (allocator != nullptr)
		destroy();
#endif
}

void Value::destroy ()
{
	switch (type)
	{
	case Type::Environment:
		delete env_;
		break;
	
	case Type::PartialFunc:
		delete[] partial_.args;
		break;

	case Type::LambdaFunc:
		delete lambda_;
		break;

	case Type::NativeFunc:
		delete[] native_.types;
		delete native_.name;
		break;

	default: break;
	}

#ifdef ML_USE_CUSTOM_ALLOCATOR
	if (allocator)
	{
		allocator->destroyed(this);
		allocator = nullptr;
	}
#else
	delete this;
#endif
}


bool Value::isType (Type t) const
{
	if (t == Type::Any)
		return true;

	if (t == Type::Number)
		return type == Type::Int ||
				type == Type::Real;

	if (t == Type::Func)
		return type == Type::PartialFunc ||
				type == Type::NativeFunc ||
				type == Type::LambdaFunc;

	return t == type;
}


std::string Value::str () const
{
	std::ostringstream ss;

	switch (type)
	{
	case Type::Void:
		return "()";

	case Type::Int:
		ss << int_.value;
		return ss.str();

	case Type::Real:
		ss << std::setprecision(10) << real_.value;
		if ((int_t)(real_.value) == real_.value)
			ss << ".0"; // fuck
		return ss.str();

	case Type::Environment:
		return "<Environment>";

	case Type::NativeFunc:
	case Type::LambdaFunc:
	case Type::PartialFunc:
		ss << "<Function";
		if (type == Type::NativeFunc)
			ss << " '" << *native_.name << "'";
		ss << ">";
		return ss.str();
	
	case Type::Bool:
		if (bool_)
			return "true";
		else
			return "false";
		
	default:
		return "??";
	}
}

std::string Value::str (Type t)
{
	switch (t)
	{
	case Type::Void: return "Void";
	case Type::Int: return "Int";
	case Type::Real: return "Real";
	case Type::Bool: return "Bool";
	case Type::Environment: return "Environment";
	case Type::Number: return "Number";
	case Type::LambdaFunc:
	case Type::NativeFunc:
	case Type::Func: return "Func";
	case Type::PartialFunc: return "PartialFunc";
	case Type::Any: 
	default: return "Any";
	}
}

int Value::numArgs () const
{
	if (type == Type::NativeFunc)
		return native_.nargs;
	
	if (type == Type::LambdaFunc)
		return lambda_->argNames.size();

	if (type == Type::PartialFunc)
		return partial_.base->numArgs() - partial_.nargs;

	return 0;
}


bool Value::trivialEval () const
{
	if (type == Type::PartialFunc)
	{
		switch (partial_.base->type)
		{
		case Type::LambdaFunc:
		case Type::NativeFunc:
			return partial_.nargs < partial_.base->numArgs();

		default:
			return false;
		}
	}

	if (type == Type::LambdaFunc ||
			type == Type::NativeFunc)
		return numArgs() > 0;

	return true;
}
bool Value::eval (Value*& out, Context* ctx, Error& err)
{
	/// optimization
	if (trivialEval())
	{
		out = this;
		return true;
	}
	else
		return partialEval(out, ctx, this, err);
}

bool Value::apply (Value*& out, Context* ctx, Value** args, Error& err)
{
	if (type == Type::NativeFunc)
	{
		auto buf = std::unique_ptr<Value*[]>(new Value*[native_.nargs]);
		for (int i = 0; i < native_.nargs; i++)
		{
			if (!args[i]->eval(buf[i], ctx, err))
				return false;

			if (!buf[i]->isType(native_.types[i]))
			{
				// TODO: make this error message not complete shit
				err.die(ctx) << "invalid argument #" << (i + 1) << " to function '"
					         << *native_.name << "', expected "
							 << str(native_.types[i]);
				return false;
			}
		}
		
		return native_.handler(out, ctx, buf.get(), err);
	}

	if (type == Type::LambdaFunc)
	{
		Context subcontext(lambda_->env);
		Environment* subenv = subcontext.env();
		
		int i = 0;
		for (auto& arg : lambda_->argNames)
			subenv->add(arg, args[i++]);
		
		if (!lambda_->body->eval(out, &subcontext, err))
			return false;

		ctx->takeControl(out, &subcontext);
		return true;
	}


	err.die(ctx) << "cannot apply value " << str();
	return false;
}

bool Value::partialEval (Value*& out, Context* ctx, Value* base, Error& err)
{
	left_vector<Value*> args;
	unsigned int nargs;
	int gctimer = 0;

	for (;;)
	{
		switch (base->type)
		{
		case Type::PartialFunc:
			args.insert(base->partial_.args,
					    base->partial_.nargs);
			
			base = base->partial_.base;
			break;

		case Type::LambdaFunc:
		case Type::NativeFunc:
			nargs = base->numArgs();

			if (args.size() < nargs)
			{
				// create partial application
				out = ctx->apply(base, args.data(), args.size());
				return true;	
			}
			else
			{
				if (!base->apply(base, ctx, args.data(), err))
					return false;

				args.erase(nargs);
				break;
			}

		default:
			if (args.size() == 0)
			{
				out = base;
				return true;
			}
			else
			{
				err.die(ctx) << "cannot apply value " << base->str();
				return false;
			}
		}
		if (++gctimer > 256)
		{
			gctimer = 0;
			std::vector<Value*> keep(args.begin(), args.end());
			keep.push_back(base);
			ctx->collectGarbage(keep);
		}
	}
}


bool Value::condition () const
{
	switch (type)
	{
	case Type::Int:
		return int_.value != 0;
	
	case Type::Real:
		return real_.value != 0;

	case Type::Bool:
		return bool_;

	case Type::Void:
		return false;
	
	default:
		return true;
	}
}


};
