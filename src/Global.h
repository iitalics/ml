#pragma once

#ifdef ML_DEBUG_ENABLED
# include <iostream>
# define MLdebug(X) \
	std::cerr << "debug :: " << X << std::endl
#else
# define MLdebug(X) static_asset(false, "DEBUGGING NOT ALLOWED")
#endif

namespace ml {

typedef double real_t;
typedef long long int_t;

struct complex_t
{
	real_t a;
	real_t b;
};





};