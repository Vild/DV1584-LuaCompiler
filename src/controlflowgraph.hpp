/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once

#include <map>
#include <memory>
#include <vector>

namespace ast {
class Node;
class RootNode;
}  // namespace ast

class BBlock;

struct Scope {
	std::string prefix;
	size_t tmpCounter = 0;
	std::vector<std::string> variables;

	Scope(std::string prefix) : prefix(prefix) {}

	std::string makeName() { return prefix + "_" + std::to_string(tmpCounter++); }

	void print();
};

struct NIL {};
struct Function {};
struct Object {};
struct Array {};
struct Ref {};

struct Value {
	// This is a char because the helps generate the constant names
	enum class Type : char {
		UNK = (char)0,
		nil = 'N',
		string = 's',
		number = 'n',
		boolean = 'b',
		function = 'f',
		object = 'o',
		array = 'a',
		ref = 'r'
	} type;

	// Could probably use a union, but in that case need to work around
	// std::string being a non-trivial type

	std::string str;  // will also be used to generate the constant name
	double number;
	bool boolean;
	Function* function;
	Object* object;
	Array* array;
	Ref* ref;

	Value() : type(Type::UNK) {}
	Value(NIL nil) : type(Type::nil), str("NIL") {}
	Value(std::string str) : type(Type::string), str(str) {}
	Value(double number)
			: type(Type::number), str(std::to_string(number)), number(number) {}
	Value(bool boolean)
			: type(Type::boolean),
				str(boolean ? "true" : "false"),
				boolean(boolean) {}
	Value(Function* function)
			: type(Type::function), str("Function"), function(function) {}
	Value(Object* object) : type(Type::object), str("Object"), object(object) {}
	Value(Array* array) : type(Type::array), str("Array"), array(array) {}
	Value(Ref* ref) : type(Type::ref), str("Ref"), ref(ref) {}
};
std::ostream& operator<<(std::ostream& out, const Value& value);

struct GlobalScope {
	std::map<std::string, Value> constants;
	std::vector<std::string> globals;
	std::vector<std::shared_ptr<Scope>> scopes;
	std::map<std::string, BBlock*> bblocks;

	~GlobalScope();

	std::string addConstant(const Value& value);
	void addGlobal(const std::string& value);
};

GlobalScope getBBlocks(std::shared_ptr<ast::RootNode> root);

// clang-format off
#define enumMembers(o)																									\
	/* Values (lhs) */																										\
	o(constant) o(emptyTable) o(preMinus) o(not_) o(pound)								\
																																				\
	/* Math (lhs & rhs) */																								\
	o(plus) o(minus) o(mul) o(div) o(pow) o(mod)													\
																																				\
	/* Compare (lhs & rhs) */																							\
	o(less) o(lequal) o(greater) o(gequal) o(equal) o(notequal)						\
																																				\
	/* Misc */																														\
	o(call) o(indexof) o(indexofRef) o(concatTable) o(functionArg) o(returnValue)
// clang-format off

enum class Operation {
#define o(x) x,
	enumMembers(o)
#undef o
};

inline std::ostream& operator<<(std::ostream& out, const Operation op) {
	static const char* str[] = {
#define o(x) #x,
			enumMembers(o)
#undef o
	};
	out << std::string(str[(int)op]);
	return out;
}
#undef enumMembers

class ThreeAddr {
public:
	std::string name;
	Operation op;
	std::string lhs, rhs;

	ThreeAddr(std::string name, Operation op, std::string lhs, std::string rhs)
			: name(name), op(op), lhs(lhs), rhs(rhs) {}

	void dump() const;
	void toDot(int id, std::ostream& out) const;
	void toASM(std::ostream& out, const BBlock* block) const;
};

class BBlock {
private:
	static int blockCounter;

public:
	std::shared_ptr<Scope> scope;
	std::vector<ThreeAddr> instructions;
	BBlock* tExit;
	BBlock* fExit;
	std::string name;

	BBlock(std::shared_ptr<Scope> scope)
			: scope{scope},
				tExit{nullptr},
				fExit{nullptr},
				name{"blk" + std::to_string(blockCounter++)} {}

	void dump() const;
	void toDot(std::ostream& out) const;
	void toASM(std::ostream& out) const;
};
