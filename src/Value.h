#pragma once
#include <string>
#include <memory>
#include <vector>

namespace ml {

class Context;
class Environment;
class Error;
class Expression;
struct Value;

struct LambdaFuncData
{
	std::vector<std::string> argNames;
	std::shared_ptr<Expression> body;
	Value* env;
};

struct Value
{
	typedef bool (*FuncHandler) (Value*&, Context*, Value** args, Error&);

	enum class Type
	{
		Void,
		Environment,
		Int,
		Real,
		Bool,
		NativeFunc,
		LambdaFunc,
		PartialFunc,

		// auxillary
		Number,
		Any,
		Func
	};	
	
	
	Value (Type type = Type::Void, Context* owner = nullptr);
	~Value ();

	bool isType (Type t) const;

	std::string str () const;
	static std::string str (Type t);

	bool trivialEval () const;
	bool eval (Value*& out, Context* ctx, Error& err);
	

	// assumes correct number of arguments
	bool apply (Value*& out, Context* ctx, Value** args, Error& err);
	int numArgs () const;


	template <typename T=real_t>
	inline T numberValue ()
	{	if (type == Type::Int)
			return (T) int_.value;
		else
			return (T) real_.value;
	}

	bool condition () const;
	
	static bool partialEval (Value*& out, Context* ctx,
						Value* base, Error& err);



	Context* owner;
	Type type;
	
	union
	{
		struct
		{
			int_t value;
		} int_;
		struct
		{
			real_t value;
		} real_;
		struct
		{
			int nargs;
			Value* base;
			Value** args;
		} partial_;
		struct
		{
			int nargs;
			FuncHandler handler;
			Type* types;
			std::string* name;
		} native_;
		bool bool_;

		Environment* env_;
		LambdaFuncData* lambda_;
	};
};


};
