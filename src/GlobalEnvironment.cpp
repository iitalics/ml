#include "Global.h"
#include "Environment.h"
#include "Value.h"
#include "Context.h"
#include "Error.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
// it hurts to live

namespace ml {

using type = Value::Type;


static bool proc_add (Value*& out, Context* ctx, Value** args, Error& err)
{
	if (args[0]->isType(Value::Type::Int) &&
			args[1]->isType(Value::Type::Int))
		out = ctx->makeInt(args[0]->int_.value +
							args[1]->int_.value);
	else
		out = ctx->makeReal(args[0]->numberValue() +
							args[1]->numberValue());
	return true;
}
static bool proc_sub (Value*& out, Context* ctx, Value** args, Error& err)
{
	if (args[0]->isType(Value::Type::Int) &&
			args[1]->isType(Value::Type::Int))
		out = ctx->makeInt(args[0]->int_.value -
							args[1]->int_.value);
	else
		out = ctx->makeReal(args[0]->numberValue() -
							args[1]->numberValue());
	return true;
}
static bool proc_mul (Value*& out, Context* ctx, Value** args, Error& err)
{
	if (args[0]->isType(Value::Type::Int) &&
			args[1]->isType(Value::Type::Int))
		out = ctx->makeInt(args[0]->int_.value *
							args[1]->int_.value);
	else
		out = ctx->makeReal(args[0]->numberValue() *
							args[1]->numberValue());
	return true;
}
static bool proc_div (Value*& out, Context* ctx, Value** args, Error& err)
{
	if (args[1]->numberValue() == 0)
	{
		err.die(ctx) << "unwilling to divide by zero";
		return false;
	}
	out = ctx->makeReal(args[0]->numberValue() /
						args[1]->numberValue());
	return true;
}
static int compare (Value* a, Value* b)
{
	if (a->isType(type::Number) && b->isType(type::Number))
	{
		auto an = a->numberValue();
		auto bn = b->numberValue();

		if (an == bn) return 0;
		if (an > bn) return 1;
		return -1;
	}
	if (a->type != b->type)
		return 1;
	switch (a->type)
	{
	case type::Bool:
		return a->bool_ == b->bool_ ? 0 : 1;

	case type::Void:
		return 0;
	
	default:
		return a == b ? 0 : 1;
	}
}
static bool proc_eql (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp == 0);
	return true;
}
static bool proc_neq (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp != 0);
	return true;
}
static bool proc_grt (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp > 0);
	return true;
}
static bool proc_les (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp < 0);
	return true;
}
static bool proc_gre (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp >= 0);
	return true;
}
static bool proc_lse (Value*& out, Context* ctx, Value** args, Error& err)
{
	int cmp = compare(args[0], args[1]);
	out = ctx->makeBool(cmp <= 0);
	return true;
}



Context Environment::global_;
static bool populated_ = false;

Environment* Environment::global ()
{
	if (!populated_)
	{
		global_.env()->populateGlobal(&global_);
		populated_ = true;
	}

	return global_.env();
}
void Environment::addProc (Context* ctx, const std::string& name,
					Value::FuncHandler ha, const std::vector<Value::Type>& ty)
{
	auto func = ctx->makeFunction(name, ty, ha);
	add(name, func);
}


void Environment::populateGlobal (Context* ctx)
{
	addProc(ctx, "+",  proc_add, { type::Number, type::Number });
	addProc(ctx, "-",  proc_sub, { type::Number, type::Number });
	addProc(ctx, "*",  proc_mul, { type::Number, type::Number });
	addProc(ctx, "/",  proc_div, { type::Number, type::Number });
	addProc(ctx, ">",  proc_grt, { type::Number, type::Number });
	addProc(ctx, "<",  proc_les, { type::Number, type::Number });
	addProc(ctx, ">=", proc_gre, { type::Number, type::Number });
	addProc(ctx, "<=", proc_lse, { type::Number, type::Number });
	addProc(ctx, "==", proc_eql, { type::Any, type::Any });
	addProc(ctx, "!=", proc_neq, { type::Any, type::Any });
}


};
