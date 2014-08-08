#pragma once
#include <list>
#include <vector>
#include "Value.h"

namespace ml {

class Environment;

class Context
{
public:
	Context (Value* parentEnv = nullptr);
	inline Context (Context* parent)
		: Context(parent->envValue()) {}
	~Context ();
	
	void collectGarbage (const std::vector<Value*>& keep);
	int purge ();

	Value* makeTrue ();
	Value* makeFalse ();
	Value* makeVoid ();
	inline Value* makeBool (bool t) { return t ? makeTrue() : makeFalse(); }
	// make numbers
	Value* makeInt (int_t t);
	Value* makeReal (real_t n);	

	// make native function
	Value* makeFunction (const std::string& name,
							const std::vector<Value::Type>& types,
							Value::FuncHandler handler);

	// ignores data.env
	Value* makeFunction (const LambdaFuncData& data);

	// make partial application
	Value* apply (Value* func, const std::vector<Value*>& args);
	Value* apply (Value* func, Value** args, int nargs); 
	


	void loseControl (Value* val);
	void takeControl (Value* val, Context* old);
	inline void takeControl (Value* val) { takeControl(val, val->owner); }
	
	inline Environment* env () { return envVal_->env_; }
	inline Value* envValue () { return envVal_; }
private:
	static Value* true_;
	static Value* false_;
	static Value* void_;

	std::list<Value*> owned_;
	Value* envVal_;	
	Value* take_ (Value* v);
};

};
