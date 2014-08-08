#include "Global.h"
#include "Error.h"
#include "Lexer.h"
#include <iostream>
#include <memory>
#include <cctype>

namespace ml {

#define ML_ERROR_INDENT "  "
#define ML_ERROR_ERROR "error: "


Error::Error ()
	: breakPos_(0) {}

void Error::dump ()
{
	std::cerr << ss_.str() << std::endl;
	ss_.str("");
}
std::string Error::getMessage () const
{
	return ss_.str().substr(breakPos_);
}



std::ostream& Error::die ()
{
	ss_ << ML_ERROR_ERROR << std::endl
		<< ML_ERROR_INDENT;
	return break_();
}

std::ostream& Error::die (const Span& span)
{
	if (!span.valid())
		return die();
	else
	{
		auto& lex = *span.lex;
		auto& data = lex.getData();
		
		Span::pos_t pos = 0, len = data.size(), line_len;
		int line_num = 1, col_num;
		
		for (;;)
		{
			for (line_len = 0; pos + line_len < len && data[pos + line_len] != '\n'; line_len++)
				;
			
			if (pos + line_len < span.pos)
			{
				line_num++;
				pos += line_len + 1;
			}
			else
				break;
		}
		
		col_num = 1 + span.pos - pos;
		
		//                                               v---
		auto line_ptr = std::unique_ptr<char[]>(new char[line_len]);
		auto line = line_ptr.get();
		
		for (Span::pos_t i = 0; i < line_len; i++)
		{
			char c = data[pos + i];
			
			if (isspace(c))
				line[i] = ' ';
			else
				line[i] = c;
		}
		
		ss_ << ML_ERROR_ERROR "line " << line_num << " in '" << lex.getFilename() << "':" << std::endl
			<< std::string(line, line_len) << std::endl
			<< std::string(col_num - 1, ' ') << "^" << std::endl
			<< ML_ERROR_INDENT;
		return break_();
	}
}
std::ostream& Error::die (const Token& tok)
{
	// token width? nah.
	return die(tok.span);
}
std::ostream& Error::die (Lexer& lex) { return die(lex.current()); }


std::ostream& Error::die (Context* ctx)
{
	// TODO: stack trace
	//       line numbers?

	ss_ << "runtime ";
	return die();
}



};
