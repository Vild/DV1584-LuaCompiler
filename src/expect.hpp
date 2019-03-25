#pragma once

#include <cstdio>
#include <cstdlib>

#define expect(EXPRESSION, msg) \
	((!!(EXPRESSION)) ? (void)0 : _expect(#EXPRESSION, __FILE__, __LINE__, msg))

inline void _expect [[noreturn]] (const char* expression,
                                  const char* file,
                                  int line,
                                  const std::string& msg) {
	fprintf(stderr, "Assertion '%s' failed, file '%s' line '%d'.\n%s\n",
	        expression, file, line, msg.c_str());
	abort();
}
