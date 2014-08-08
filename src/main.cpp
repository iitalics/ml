#include <iostream>
#include "ML.h"


int main ()
{
	ml::Error err;
	ml::Lexer lex;
	ml::Token tok;
	
	if (!lex.open("test.txt", err))
		goto fail;

	{
		ml::Parser parser(lex);
	
		ml::Context ctx;
		ml::Value* mainFunc;
		ml::Value* output = ctx.makeVoid();
		
		if (!parser.parseEnvironment(&ctx, false, err))
			goto fail;
		
		
		if (!ctx.env()->get(mainFunc, "main"))
		{
			err.die() << "no main function";
			goto fail;
		}
		
		if (!mainFunc->eval(output, &ctx, err))
			goto fail;
		
		std::cout << "result: " << output->str() << std::endl;
	}
	
	return 0;
fail:
	err.dump();
	return -1;
}
