#pragma once
#include "Lexer.h"
#include "Value.h"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

namespace ml {
class Context;
class Expression
{
public:
	virtual ~Expression () = 0;
	virtual std::string type () const = 0;
	virtual bool eval (Value*& out, Context* ctx, Error& err);
};




namespace Exp {
	using ptr = std::shared_ptr<Expression>;
	struct LambaData
	{
		inline LambaData ()
			: body(nullptr) {}
		
		std::vector<std::string> args;
		ptr body;
	};
	typedef Value* (Context::*Generator) ();
	

	ptr makeInt (int_t num);
	ptr makeReal (real_t real);	
	ptr makeVariable (const std::string& var, bool global = false);
	ptr makeApplication (ptr base, const std::vector<ptr>& args);
	ptr makeLambda (const LambaData& data);
	ptr makeGenerator (const std::string& name, Generator gen);

	ptr makeIf (ptr cond, ptr then, ptr otherwise);
};



};
