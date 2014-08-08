#include "Global.h"
#include "Environment.h"
#include "Value.h"
#include "Context.h"
#include "Error.h"

namespace ml {



Environment::Environment (Value* parent)
	: parent_(parent)
{ }

bool Environment::add (const std::string& name, Value* val)
{
	if (data_.find(name) != data_.end())
		return false;
	
	data_[name] = val;
	return true;
}

bool Environment::get (Value*& out, const std::string& name)
{
	auto it = data_.find(name);

	if (it == data_.end())
	{
		if (parent_ == nullptr)
		{
			if (this == global())
				return false;
			else
				return global()->get(out, name);
		}
		else
			return parent_->env_->get(out, name);
	}
	
	out = it->second;
	return true;
}

Environment* Environment::parent ()
{
	if (parent_ == nullptr)
		return nullptr;
	else
		return parent_->env_;
}


};
