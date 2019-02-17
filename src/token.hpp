/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once
#include <iostream>
#include <fstream>
#include <string_view>

namespace ast::token {
	class Token {
	public:
		virtual void print(std::ostream& out) = 0;
		virtual void toDot(std::ostream& out) = 0;

		virtual std::string_view toString() = 0;
	};

	class BinOPToken : public Token {
	public:
#define enumMembers(o) \
	o(UNK) \
	o(plus) \
	o(mul) \
	o(div) \
	o(pow) \
	o(mod) \
	o(dotdot) \
	o(less) \
	o(lessOrEqual) \
	o(more) \
	o(moreOrEqual) \
	o(equal) \
	o(notEqual) \
	o(and_) \
	o(or_) \
	o(minus)

		enum class OP {
#define o(x) x,
			enumMembers(o)
#undef o
		};

		OP op;
		BinOPToken() : op(OP::UNK) {}
		BinOPToken(OP op) : op(op) {}

		virtual void print(std::ostream& out) {
			out << toString() << std::endl;
		}
		virtual void toDot(std::ostream& out) {
			static int id = 0;
			out << "BinOPToken" << id++ << "[label=\""<< toString() << "\"];" << std::endl;
		}

		virtual std::string_view toString() {
			static const char* OP_Str[] = {
#define o(x) "BinOPToken: " #x,
				enumMembers(o)
#undef o
			};

			return OP_Str[(int)op];
		}
#undef enumMembers
	};

	class MinusOPToken : public Token {
	public:
		virtual void print(std::ostream& out) {
			out << toString() << std::endl;
		}
		virtual void toDot(std::ostream& out) {
			static int id = 0;
			out << "MinusOPToken" << id++ << "[label=\""<< toString() << "\"];" << std::endl;
		}

		virtual std::string_view toString() {
			return "MinusOPToken";
		}
	};

	class UnOPToken : public Token {
	public:
#define enumMembers(o) \
	o(UNK) \
	o(not_) \
	o(pound) \
	o(minus)

		enum class OP {
#define o(x) x,
			enumMembers(o)
#undef o
		};

		OP op;
		UnOPToken() : op(OP::UNK) {}
		UnOPToken(OP op) : op(op) {}

		virtual void print(std::ostream& out) {
			out << toString() << std::endl;
		}
		virtual void toDot(std::ostream& out) {
			static int id = 0;
			out << "UnOPToken" << id++ << "[label=\""<< toString() << "\"];" << std::endl;
		}

		virtual std::string_view toString() {
			static const char* OP_Str[] = {
#define o(x) "UnOPToken: " #x,
				enumMembers(o)
#undef o
			};

			return OP_Str[(int)op];
		}
#undef enumMembers
	};

	class SemicolonToken : public Token {
	public:
		virtual void print(std::ostream& out) {
			out << toString() << std::endl;
		}
		virtual void toDot(std::ostream& out) {
			static int id = 0;
			out << "SemicolonToken" << id++ << "[label=\""<< toString() << "\"];" << std::endl;
		}

		virtual std::string_view toString() {
			return "SemicolonToken";
		}
	};
}
