#pragma once
#include "Expression.h"


namespace ml {

class Lexer;


class Parser
{
public:
	Parser (Lexer& lex);
	
	bool parseEnvironment (Context* ctx, bool expectTrailing, Error& err);
	
	bool parseExpression (Exp::ptr& out, Error& err);
	bool parseSYard (Exp::ptr& out, Error& err);
	bool parseApplication (Exp::ptr& out, Error& err);
	bool parseTerm (Exp::ptr& out, Error& err);
	bool parseTuple (Exp::ptr& out, Error& err);
	bool parseFunction (Exp::ptr& out, Error& err);
	bool parseIf (Exp::ptr& out, Error& err);

	bool parseCommaExpressions (std::vector<Exp::ptr>& out, Error& err);
	bool parseId (std::string& out, Error& err);
private:
	Lexer& lex_;
	
	
	bool isExp_ ();
	bool isTerm_ ();
	bool isOperator_ ();
	
	bool expected_ (const std::string& tok, Error& err);
	bool expected_ (int n, Error& err);
	bool unexpected_ (Error& err);

	bool eat_ (int tok, Error& err);
};





};
