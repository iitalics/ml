#pragma once
#include <sstream>

namespace ml {

struct Span;
struct Token;
class Context;
class Lexer;


class Error
{
public:
	Error ();
	
	std::ostream& die ();
	std::ostream& die (const Span& span);
	std::ostream& die (const Token& tok);
	std::ostream& die (Lexer& lex);
	std::ostream& die (Context* ctx);	
	
	void dump ();
	std::string getMessage () const;
	
	// TODO: save metadata such as line #, call stack etc
	
private:
	std::ostringstream ss_;
	int breakPos_;
	
	inline std::ostream& break_ ()
	{
		breakPos_ = ss_.str().size();
		return ss_;
	}
};



};
