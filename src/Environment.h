#pragma once
#include "Context.h"
#include <string>
#include <map>
#include <unordered_map>


namespace ml {

class Environment
{
public:
	using map_t = std::map<std::string, Value*>;

	static Environment* global ();
	
	Environment (Value* parent);
	
	bool add (const std::string& name, Value* val);
	bool get (Value*& out, const std::string& name);
	
	inline Value* parentValue () { return parent_; }
	Environment* parent ();


	class iterator
	{
	private:
		map_t::iterator it_;	
	public:
		iterator (decltype(it_)&& it)
			: it_(it) {}
		Value* operator* () { return it_->second; }
		iterator& operator++ () { it_++; return *this; }
		bool operator!= (const iterator& o) { return it_ != o.it_; }
	};
	inline iterator begin () { return iterator(data_.begin()); }
	inline iterator end () { return iterator(data_.end()); }
private:
	map_t data_;
	Value* parent_;

	static Context global_;
	void populateGlobal (Context* ctx); // located in GlobalEnvironment.cpp
	void addProc (Context*, const std::string&,
			Value::FuncHandler, const std::vector<Value::Type>&);
};



};
