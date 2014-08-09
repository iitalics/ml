#pragma once


//#define ML_USE_CUSTOM_ALLOCATOR
#define ML_DEBUG_ENABLED











#ifdef ML_DEBUG_ENABLED
# include <iostream>
# define MLdebug(X) \
	std::cerr << "debug :: " << X << std::endl
#else
# define MLdebug(X)
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
