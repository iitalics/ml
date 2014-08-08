#include "Global.h"
#include "Lexer.h"
#include "Error.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <iomanip>

namespace ml {

std::vector<Syntax::Tok> Syntax::sequences
{
	{ Token::t_eql, "==" },
	{ Token::t_neq, "!=" },
	{ Token::t_leq, "<=" },
	{ Token::t_geq, ">=" },
	{ Token::t_rarrow, "=>" },
	{ Token::t_append, "++" }
};

std::vector<Syntax::Tok> Syntax::keywords
{
	{ Token::k_fn, "fn" },
	{ Token::k_let, "let" },
	{ Token::k_match, "match" },
	{ Token::k_in, "in" },
	{ Token::k_if, "if" },
	{ Token::k_then, "then" },
	{ Token::k_else, "else" },
	{ Token::k__, "_" },
	{ Token::k_true, "true" },
	{ Token::k_false, "false" },
};

bool Syntax::isSpace (char c) { return isspace(c) || !c; } // " \n\r\t\b\0"
bool Syntax::isDigit (char c) { return isdigit(c); }
bool Syntax::isSymbol (char c)
{
	return 	c == '_' ||
			c == '?' ||
			c == '@' ||
			c == '#' ||
		isdigit(c) ||
		isalpha(c);
}

char Syntax::comment = ';';


Lexer::Lexer ()
	: filename_ (""),
		pos_(Span::bad_pos) {}

Lexer::~Lexer ()
{
}


bool Lexer::open (const std::string& filename, Error& err)
{
	std::ifstream fs(filename);
	
	if (!fs.good())
	{
		err.die() << "could not open file '" << filename << "'";
		return false;
	}
	
	std::ostringstream ss;
	std::string line;
	
	std::getline(fs, line);
	ss << line;
	
	while (!fs.eof())
	{
		std::getline(fs, line);
		ss << std::endl << line;
	}
	
	return init(filename, ss.str(), err);
}
bool Lexer::init (const std::string& data, Error& err)
{
	len_ = data.size();
	pos_ = 0;
	data_ = data;
	
	return advance(err);
}



void Lexer::triml_ ()
{
	for (;;)
	{
		while (!eof() && Syntax::isSpace(c()))
			pos_++;
		
		if (c() == Syntax::comment)
			while (c() != '\n')
				pos_++;
		else
			break;
	}
}


int Lexer::parseDigit_ (char c, int base)
{
	int val;
	
	c = tolower(c);
	
	if (c >= '0' && c <= '9')
		val = c - '0';
	else if (c >= 'a' && c <= 'z')
		val = c - 'a' + 10;
	else
		return -1;
	
	if (val >= base)
		return -1;
	else
		return val;
}
bool Lexer::parseNumber_ (Error& err)
{
	auto sp = span();
	
	int_t n = 0;
	int digit, base = 10;
	bool required = false;
	
	if ((len_ - pos_) >= 2 &&
			data_.substr(pos_, 2) == "0x")
	{
		base = 16;
		pos_ += 2;
		required = true;
	}
	
	while (Syntax::isSymbol(c()) || required)
	{
		required = false;
		digit = parseDigit_(c(), base);
		
		if (digit == -1)
		{
			err.die(span()) << "invalid digit in number literal";
			return false;
		}
		
		n = (n * base) + digit;
		pos_++;
	}
	
	if (base == 10 && c() == '.')
	{
		pos_++;
		
		real_t r = n;
		int mag = 1;
		
		required = true;
		
		while (Syntax::isSymbol(c()) || required)
		{
			required = false;
			digit = parseDigit_(c(), base);
			
			if (digit == -1)
			{
				err.die(span()) << "invalid digit in number literal";
				return false;
			}
			
			mag *= 10;
			r += digit / (real_t) mag;
			pos_++;
		}
		
		set_(Token::t_number_real, sp)
			.val_real = r;
	}
	else
	{
		set_(Token::t_number, sp)
			.val_int = n;
	}
	
	return true;
}

bool Lexer::parseId_ (Error& err)
{
	std::ostringstream ss;
	auto sp = span();
	
	while (Syntax::isSymbol(c()))
	{
		ss << c();
		pos_++;
	}
	
	for (auto& kw : Syntax::keywords)
		if (kw.str == ss.str())
		{
			set_(kw.tok, sp);
			return true;
		}
	
	set_(Token::t_id, sp)
		.val_string = ss.str();
	return true;
}

bool Lexer::advance (Error& err)
{
	triml_();
	
	if (eof())
	{
		set_(Token::t_eof);
		return true;
	}
	
	for (auto& seq : Syntax::sequences)
		if ((len_ - pos_) >= seq.str.size() && 
				data_.substr(pos_, seq.str.size()) == seq.str)
		{
			set_(seq.tok);
			pos_ += seq.str.size();
			return true;
		}
	
	if (Syntax::isDigit(c()) ||
			((len_ - pos_) >= 2 &&
			 c() == '.' &&
			 Syntax::isDigit(data_[pos_ + 1])))
		return parseNumber_(err);
	
	if (Syntax::isSymbol(c()))
		return parseId_(err);
	
	set_(c());
	pos_++;
	return true;
}






std::string Token::str () const
{
	std::ostringstream ss;
	
	switch (tok)
	{
	case t_id:
		return val_string;
	
	case t_number:
		ss << val_int;
		return ss.str();
		
	case t_number_real:
		ss << std::setprecision(4) << val_real;
		return ss.str();
		
	default:
		return str(tok);
	}
}

std::string Token::str (int tok)
{
	switch (tok)
	{
	case t_number:
		return "#number";
	
	case t_number_real:
		return "#real";
	
	case t_id:
		return "#identifier";
	
	case t_eof:
		return "#eof";
	
	default: break;
	}
	
	if (tok < _t_)
		return std::string(1, (char) tok);
	
	if (tok > _k_)
		return Syntax::keywords[tok - _k_ - 1].str;
	
	if (tok > _seq_)
		return Syntax::sequences[tok - _seq_ - 1].str;
	
	return "??";
}

};
