#pragma once
#include <climits>
#include <string>
#include <iostream>
#include <vector>

namespace ml {


class Lexer;
class Error;


/// represents a location in the code
struct Span
{
	using pos_t = unsigned long;
	
	static pos_t constexpr bad_pos = ULONG_MAX;
	
	inline Span ()
		: lex(nullptr), pos(bad_pos) {}
	
	explicit inline Span (Lexer* lex_, pos_t pos_ = bad_pos)
		: lex(lex_), pos(pos_) {}
	
	inline bool valid () const { return lex != nullptr && pos != bad_pos; }
	
	Lexer* lex;
	pos_t pos;
	
};


struct Token
{
	enum
	{
		// 0-255 = characters
		t_eof = 0,
		_t_ = 256,
		
		/// tokens with metadata
		t_id,
		t_number,
		t_number_real,
		
		_seq_,
		
		/// two or three character sequences
		t_eql, t_neq,
		t_leq, t_geq,
		t_rarrow, t_append,
		
		_k_,
		
		/// keywords
		k_fn, k_let,
		k_match, k_in,
		k_if, k_then,
		k_else, k__, // _ is a keyword
		k_true, k_false,
	};
	
	inline Token (int tok_ = t_eof, const Span& s = Span())
		: tok(tok_),
			span(s),
			val_string("") {}
	
	std::string str () const;
	
	/// get string generalization of token type
	static std::string str (int t);
	
	
	int tok;
	Span span;
	
	
	/// internal data
	std::string val_string;
	union {
		int_t val_int;
		real_t val_real;
	};
};

class Lexer
{
public:
	Lexer ();
	~Lexer ();
	
	inline std::string getFilename () const { return filename_; }
	inline const std::string& getData () const { return data_; }
	inline Token& current () { return cur_; }
	inline Token current () const { return cur_; }
	inline Span span () { return Span(this, pos_); }
	inline bool eof () const { return pos_ >= len_; }
	
	bool open (const std::string& filename, Error& err);
	bool init (const std::string& data, Error& err);
	inline bool init (const std::string& fn, const std::string& data, Error& err)
	{
		filename_ = fn;
		return init(data, err);
	}
	bool advance (Error& err);
	
private:
	std::string filename_;
	std::string data_;
	Span::pos_t pos_;
	Span::pos_t len_;
	Token cur_;
	
	void triml_ ();
	int parseDigit_ (char c, int base);
	bool parseNumber_ (Error& err);
	bool parseId_ (Error& err);
	
	inline char c () const { return data_[pos_]; }
	inline Token& set_ (int t, const Span& s) { return cur_ = Token(t, s); }
	inline Token& set_ (int t) { return set_(t, span()); }
};


/// syntax rules
struct Syntax
{
	static char comment;
	
	struct Tok
	{
		int tok;
		std::string str;
	};
	
	static std::vector<Tok> sequences;
	static std::vector<Tok> keywords;
	
	static bool isSpace (char c);
	static bool isDigit (char c);
	static bool isSymbol (char c);
};



};
