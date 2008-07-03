#include "hello.h"

const char *Hello::greet()
{
	static std::string ts;

	ts = "hello " + s;

	return ts.c_str();
}

void Hello::set_name(char const *n)
{
	s = n;
}
