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
	int tmpCounter = 0;
	std::vector<std::string> variables;

	Scope(std::string prefix) : prefix(prefix) {}

	std::string makeName() {
		return prefix + "_" + std::to_string(tmpCounter++);
	}

	void print();
};

struct GlobalScope {
	std::vector<std::string> globals;
	std::vector<std::shared_ptr<Scope>> scopes;
	std::map<std::string, BBlock*> bblocks;

	~GlobalScope();
};

GlobalScope getBBlocks(std::shared_ptr<ast::RootNode> root);

#define enumMembers(o)													\
	/* Values (lhs) */														\
	o(constant)																		\
	o(emptyTable)																		\
	o(preMinus)																		\
	o(not_)																				\
	o(pound)																			\
	/* Math (lhs & rhs) */												\
	o(plus)																				\
	o(minus)																			\
	o(mul)																				\
	o(div)																				\
	o(pow)																				\
	o(mod)																				\
	/* Compare (lhs & rhs) */											\
	o(less)																				\
	o(lequal)																			\
	o(greater)																		\
	o(gequal)																			\
	o(equal)																			\
	o(notequal)																		\
	/* Misc */																		\
	o(call)																				\
	o(indexof)																		\
	o(concatTable)

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
	void toASM(std::ostream& out) const;
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
