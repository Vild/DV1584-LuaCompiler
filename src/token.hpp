/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once
#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

// TODO: Add 'override' keyword to functions

namespace ast {
namespace token {
class Token {
public:
	std::string name;
	int id;

	Token(std::string name) : name(name) {
		static int idCounter = 0;
		id = idCounter++;

		// std::cerr << "Constructing Token: " << name << std::endl;
	}
	virtual void print(std::ostream& out) { out << toString() << std::endl; }

	virtual void toDot(std::ostream& out) {
		out << name << id << "[label=\"" << toString() << "\"];" << std::endl;
	}
	virtual std::string toString() { return name; }
};

class MinusOPToken : public Token {
public:
	MinusOPToken() : Token("MinusOPToken") {}
};

class BinOPToken : public Token {
public:
// clang-format off
#define enumMembers(o)																								\
	o(UNK) o(plus) o(mul) o(div) o(pow) o(mod) o(dotdot)								\
	o(less) o(lessOrEqual) o(more) o(moreOrEqual) o(equal) o(notEqual)	\
	o(and_) o(or_) o(minus)
	// clang-format on

	enum class OP {
#define o(x) x,
		enumMembers(o)
#undef o
	};

	OP op;
	BinOPToken() : Token("BinOPToken"), op(OP::UNK) {}
	BinOPToken(OP op) : Token("BinOPToken"), op(op) {}

	virtual std::string toString() {
		static const char* OP_Str[] = {
#define o(x) "BinOpToken: " #x,
		    enumMembers(o)
#undef o
		};

		return OP_Str[(int)op];
	}
#undef enumMembers
};

class UnOPToken : public Token {
public:
#define enumMembers(o) o(UNK) o(not_) o(pound) o(minus)

	enum class OP {
#define o(x) x,
		enumMembers(o)
#undef o
	};

	OP op;
	UnOPToken() : Token("UnOPToken"), op(OP::UNK) {}
	UnOPToken(OP op) : Token("UnOPToken"), op(op) {}

	virtual std::string toString() {
		static const char* OP_Str[] = {
#define o(x) "UnOPToken: " #x,
		    enumMembers(o)
#undef o
		};

		return OP_Str[(int)op];
	}
#undef enumMembers
};

// clang-format off
#define tokens(o)																												\
	o(Semicolon) o(Equals) o(Comma) o(Dot) o(Colon) o(SquareOpen)					\
	o(SquareClose) o(VariableList) o(ParenthesisOpen) o(ParenthesisClose)	\
	o(ListOpen) o(ListClose) o(Do) o(End) o(While) o(Repeat) o(Until)			\
	o(If) o(Then) o(ElseIf) o(Else) o(For) o(In) o(Function) o(Local)			\
	o(Return) o(Break) o(Nil) o(True) o(False)
// clang-format on

#define o(x)                          \
	class x##Token : public Token {     \
	public:                             \
		x##Token() : Token(#x "Token") {} \
	};
tokens(o)
#undef o
#undef tokens

    class QuotedToken : public Token {
public:
	std::string value;
	QuotedToken() : Token("QuotedToken"), value("") {}
	QuotedToken(std::string value) : Token("QuotedToken"), value(value) {}

	virtual std::string toString() {
		return std::string{"QuotedToken: "} + value;
	}
};
class NameToken : public Token {
public:
	std::string value;
	NameToken() : Token("NameToken"), value("") {}
	NameToken(std::string value) : Token("NameToken"), value(value) {}

	virtual std::string toString() {
		return std::string{"QuotedToken: "} + value;
	}
};
class NumberToken : public Token {
public:
	double value;
	NumberToken()
	    : Token("NumberToken"), value(std::numeric_limits<double>::quiet_NaN()) {}
	NumberToken(double value) : Token("NumberToken"), value(value) {}
	NumberToken(std::string value)
	    : Token("NumberToken"), value(atof(value.c_str())) {}

	virtual std::string toString() {
		return std::string{"QuotedToken: "} + std::to_string(value);
	}
};
}  // namespace token
}  // namespace ast
