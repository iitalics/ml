#include "Global.h"
#include "Parser.h"
#include "Lexer.h"
#include "Error.h"
#include "Context.h"
#include "Environment.h"
#include <stack>
#include <iostream>


namespace ml {



Parser::Parser (Lexer& lex)
	: lex_(lex) {}



bool Parser::expected_ (const std::string& tok, Error& err)
{
	err.die(lex_) << "expected " << tok << ", got " << lex_.current().str();
	return false;
}
bool Parser::expected_ (int n, Error& err)
{
	err.die(lex_) << "expected '" << Token::str(n) << "', got " << lex_.current().str();
	return false;
}
bool Parser::unexpected_ (Error& err)
{
	err.die(lex_) << "unexpected '" << lex_.current().str() << "'";
	return false;
}
bool Parser::eat_ (int tok, Error& err)
{
	if (lex_.current().tok == tok)
		return lex_.advance(err);
	else
		return expected_(tok, err);
}


bool Parser::isExp_ ()
{
	switch (lex_.current().tok)
	{
	case Token::k_if:
		return true;
	case Token::k_match:
	case Token::k_let:
		return false; // huehue

	default:
		return isTerm_();
	}
}
bool Parser::isTerm_ ()
{
	switch (lex_.current().tok)
	{
	case Token::t_id:
	case Token::t_number:
	case Token::t_number_real:
	case '(':
	case Token::k_true:
	case Token::k_false:
		return true;
	
	default:
		return false;
	}
}
bool Parser::isOperator_ ()
{
	switch (lex_.current().tok)
	{
	case '+': case '-': case '*': case '/':
	case Token::t_eql: case Token::t_neq:
	case Token::t_geq: case Token::t_leq:
	case '<': case '>':
	//case Token::t_append: case '$': case ':':
		return true;
	
	default:
		return false;
	}
}




bool Parser::parseEnvironment (Context* ctx, bool expectTrailing, Error& err)
{
	// fn <id> <func>
	// let <id> = <exp>

	std::string name("");
	Exp::ptr exp;
	Span nameSpan;

	if (lex_.current().tok == Token::k_fn)
	{
		Exp::LambaData data;

		if (!lex_.advance(err))
			return false;
		nameSpan = lex_.current().span;
		if (!parseId(name, err))
			return false;
		
		// get function body
		if (!parseFunction(exp, err))
			return false;
	}
	else if (lex_.current().tok == Token::k_let)
	{
		err.die(lex_) << "unable to parse global constant";
		return false;
	}
	else if (!expectTrailing)
	{
		if (lex_.current().tok == Token::t_eof)
			return true;
		err.die(lex_) << "unexpected trailing '" << lex_.current().str() << "'";
		return false;
	}



	Value* val;
	if (!exp->eval(val, ctx, err))
		return false;
	if (!ctx->env()->add(name, val))
	{
		err.die(nameSpan) << "cannot override existing '" << name << "'";
		return false;
	}
	
	return parseEnvironment(ctx, expectTrailing, err);
}

bool Parser::parseFunction (Exp::ptr& out, Error& err)
{
	// [<id>...] = <exp>

	Exp::LambaData data;

	while (lex_.current().tok == Token::t_id)
	{
		data.args.push_back(lex_.current().val_string);
		if (!lex_.advance(err))
			return false;
	}
	
	if (!eat_('=', err))
		return false;

	if (!parseExpression(data.body, err))
		return false;
	
	out = Exp::makeLambda(data);
	return true;
}
bool Parser::parseIf (Exp::ptr& out, Error& err)
{
	// if <exp> then <exp> else <exp>
	
	Exp::ptr a, b, c;
	
	if (!eat_(Token::k_if, err) ||
			!parseExpression(a, err) ||
			!eat_(Token::k_then, err) ||
			!parseExpression(b, err) ||
			!eat_(Token::k_else, err) ||
			!parseExpression(c, err))
		return false;

	out = Exp::makeIf(a, b, c);
	return true;
}








struct SYard
{
	std::stack<int> operators;
	std::stack<Exp::ptr> exps;

	int precedence (int op)
	{
		switch (op)
		{
		case '*': case '/':
			return 3;
		case '+': case '-':
			return 2;
		case Token::t_eql: case Token::t_neq:
		case Token::t_leq: case Token::t_geq:
		case '<': case '>':
			return 1;
		default:
			return 0;
		}
	}
	void popMath ()
	{
		Exp::ptr a, b;
		int op;
	   
		op = operators.top();
		operators.pop();

		b = exps.top();
		exps.pop();
		a = exps.top();
		exps.pop();

		std::string opName(Token::str(op));
		
		exps.push(Exp::makeApplication(
					Exp::makeVariable(opName, true),
					{ a, b }));	
	}
	void pushOp (int op)
	{
		while (!operators.empty())
		{
			int top = operators.top();
			if (precedence(top) >= precedence(op))
				popMath();
			else
				break;
		}

		operators.push(op);
	}
	void pushExp (Exp::ptr exp)
	{
		exps.push(exp);
	}
	Exp::ptr empty ()
	{
		while (!operators.empty())
			popMath();

		return exps.top();
	}
};



bool Parser::parseExpression (Exp::ptr& out, Error& err)
{
	if (!isExp_())
		return expected_("expression", err);
	
	switch (lex_.current().tok)
	{
	case Token::k_if:
		return parseIf(out, err);

	default:
		break;
	}

	return parseSYard(out, err);
}
bool Parser::parseSYard (Exp::ptr& out, Error& err)
{
	// <app> [ [<op> <app>]... ]

	Exp::ptr exp;
	SYard yard;
	
	if (!parseApplication(exp, err))
		return false;
	yard.pushExp(exp);

	while (isOperator_())
	{
		auto op = lex_.current().tok;
		if (!lex_.advance(err))
			return false;
		if (!parseApplication(exp, err))
			return false;

		yard.pushOp(op);
		yard.pushExp(exp);
	}

	out = yard.empty();
	return true;
}
bool Parser::parseApplication (Exp::ptr& out, Error& err)
{
	// <term> [<term>...]
	
	std::vector<Exp::ptr> args;
	Exp::ptr func, arg;
	
	if (!parseTerm(func, err))
		return false;
	
	while (isTerm_())
	{
		if (!parseTerm(arg, err))
			return false;
		
		args.push_back(arg);
	}
	
	if (args.empty())
		out = func;
	else
		out = Exp::makeApplication(func, args);
	
	return true;
}


bool Parser::parseTerm (Exp::ptr& out, Error& err)
{
	if (!isTerm_())
		return expected_("term", err);
	
	switch (lex_.current().tok)
	{	
		case Token::t_number:
			out = Exp::makeInt(lex_.current().val_int);
			break;
		
		case Token::t_number_real:
			out = Exp::makeReal(lex_.current().val_real);
			break;

		case Token::t_id:
			out = Exp::makeVariable(lex_.current().val_string);
			break;

		case '(':
			return parseTuple(out, err);

		case Token::k_true:
			out = Exp::makeGenerator("bool-true", &Context::makeTrue);
			break;
			
		case Token::k_false:
			out = Exp::makeGenerator("bool-false", &Context::makeFalse);
			break;
			
		default:
			return unexpected_(err);
	}
	return lex_.advance(err);
}

bool Parser::parseTuple (Exp::ptr& out, Error& err)
{
	// ( [<exp> [, <exp>]...] )

	std::vector<Exp::ptr> values;
	auto sp = lex_.current().span;

	if (!eat_('(', err))
		return false;
	if (!parseCommaExpressions(values, err))
		return false;
	if (!eat_(')', err))
		return false;

	if (values.size() == 0)
	{
		// "void" = ()
		out = Exp::makeGenerator("void", &Context::makeVoid);
		return true;
	}
	else if (values.size() == 1)
	{
		out = values[0];
		return true;
	}
	else
	{
		err.die(sp) << "tuples unsupported";
		return false;
	}
}

bool Parser::parseId (std::string& out, Error& err)
{
	if (lex_.current().tok != Token::t_id)
		return expected_(Token::t_id, err);

	out = lex_.current().val_string;
	return lex_.advance(err);
}

bool Parser::parseCommaExpressions (std::vector<Exp::ptr>& out, Error& err)
{
	// [<exp> [, <exp>]...]
	
	Exp::ptr exp;

	if (!isExp_())
		return true;
	
	if (!parseExpression(exp, err))
		return false;
	out.push_back(exp);

	while (lex_.current().tok == ',')
	{
		if (!lex_.advance(err))
			return false;
		if (!parseExpression(exp, err))
			return false;

		out.push_back(exp);
	}
	return true;
}


};

