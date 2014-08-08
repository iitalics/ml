#include "Global.h"
#include "Expression.h"
#include "Error.h"
#include "Context.h"
#include "Environment.h"

namespace ml {



Expression::~Expression () {}

bool Expression::eval (Value*& out, Context* ctx, Error& err)
{
	err.die() << "Exp \"" << type() << "\" eval() unimplemented";
	return false;
}



namespace Exp {


class NumberExpression
	: public Expression
{
public:
	template <typename T>
	NumberExpression (bool real, T num)
		: real_(real)
	{
		if (real)
			realNum_ = (real_t) num;
		else
			intNum_ = (int_t) num;
	}
	
	virtual ~NumberExpression () { }
	virtual std::string type () const { return "constant-number"; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		if (real_)
			out = ctx->makeReal(realNum_);
		else
			out = ctx->makeInt(intNum_);

		return true;
	}
private:
	union {
		real_t realNum_;
		int_t intNum_;
	};
	bool real_;
};
ptr makeInt (int_t num)
{ return std::make_shared<NumberExpression>(false, num); }
ptr makeReal (real_t num)
{ return std::make_shared<NumberExpression>(true, num); }



class VarExpression
	: public Expression
{
public:
	VarExpression (const std::string& var, bool global)
		: var_(var), global_(global), cache_(nullptr)
	{
		if (global_)
		{
			if (!Environment::global()->get(cache_, var_))
				cache_ = nullptr;
		}
	}

	virtual ~VarExpression () { }
	ptr makeLambda (const LambaData& data);
	virtual std::string type () const { return "variable"; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		if (cache_)
		{
			out = cache_;
			return true;
		}
		auto env = global_ ?
					Environment::global() :
					ctx->env();
		
		if (!env->get(out, var_))
		{
			err.die(ctx) << "could not find variable '" << var_ << "'";
			return false;
		}
		else
			return true;
	}
private:
	std::string var_;
	bool global_;
	Value* cache_;
};
ptr makeVariable (const std::string& var, bool g)
{ return std::make_shared<VarExpression>(var, g); }




class ApplyExpression
	: public Expression
{
public:
	ApplyExpression (ptr base, const std::vector<ptr>& args)
		: base_(base), args_(args) {}

	virtual ~ApplyExpression () { }
	virtual std::string type () const { return "application"; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		Value* base, *arg;
		std::vector<Value*> args;
		args.reserve(args_.size());
		bool allTrivial = true;

		if (!base_->eval(base, ctx, err))
			return false;
		
		for (auto& e : args_)
			if (!e->eval(arg, ctx, err))
				return false;
			else
			{
				args.push_back(arg);
				if (!arg->trivialEval())
					allTrivial = false;
			}

		// eager application when trivial 
		if (allTrivial && base->isType(Value::Type::NativeFunc) &&
				int(args.size()) == base->native_.nargs)
		{
			return base->apply(out, ctx, args.data(), err);
		}

		out = ctx->apply(base, args);
		return true;
	}
private:
	ptr base_;
	std::vector<ptr> args_;
};
ptr makeApplication (ptr base, const std::vector<ptr>& args) 
{ return std::make_shared<ApplyExpression>(base, args); }


class LambdaExpression
	: public Expression
{
public:
	LambdaExpression (const LambaData& data)
		: lambda_(data) {}
	
	virtual ~LambdaExpression () { }
	virtual std::string type () const { return "lambda"; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		LambdaFuncData data
			{
				lambda_.args,
				lambda_.body,
				nullptr
			};

		out = ctx->makeFunction(data);
		return true;
	}
private:
	LambaData lambda_;
};

ptr makeLambda (const LambaData& data)
{ return std::make_shared<LambdaExpression>(data); }



class GenExpression
	: public Expression
{
public:
	GenExpression (const std::string& name, Generator gen)
		: type_("constant-" + name), gen_(gen) {}

	virtual ~GenExpression () { }
	virtual std::string type () const { return type_; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		out = (ctx->*gen_)();
		return true;
	}
private:
	std::string type_;
	Generator gen_;
};
ptr makeGenerator (const std::string& name, Generator gen)
{ return std::make_shared<GenExpression>(name, gen); }




class IfExpression
	: public Expression
{
public:
	IfExpression (ptr a, ptr b, ptr c)
		: cond_(a), then_(b), else_(c) {}
	
	virtual ~IfExpression () { }
	virtual std::string type () const { return "control-if"; }

	virtual bool eval (Value*& out, Context* ctx, Error& err)
	{
		Value* cond;

		if (!cond_->eval(cond, ctx, err))
			return false;
		if (!cond->eval(cond, ctx, err))
			return false;
		if (cond->condition())
			return then_->eval(out, ctx, err);
		else
			return else_->eval(out, ctx, err);	
	}
private:
	ptr cond_, then_, else_;
};
ptr makeIf (ptr a, ptr b, ptr c)
{ return std::make_shared<IfExpression>(a, b, c); }


};
};
