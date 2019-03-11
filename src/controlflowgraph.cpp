/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <controlflowgraph.hpp>
#include <token.hpp>
#include <ast.hpp>
#include <expect.hpp>
#include <iostream>
#include <set>

int BBlock::blockCounter = 0;

void Scope::print() {
	std::cout << "Scope(prefix: " << prefix << ", varCounter: " << varCounter
						<< ")" << std::endl;
}

GlobalScope::~GlobalScope() {
	std::set<BBlock*> done, todo;
	for (std::pair<std::string, BBlock*> bb : bblocks)
		todo.insert(bb.second);
	while (todo.size() > 0) {
		auto first = todo.begin();
		BBlock* next = *first;
		todo.erase(first);
		done.insert(next);
		if (next->tExit != nullptr && done.find(next->tExit) == done.end())
			todo.insert(next->tExit);
		if (next->fExit != nullptr && done.find(next->fExit) == done.end())
			todo.insert(next->fExit);

		delete next;
	}
}

GlobalScope getBBlocks(std::shared_ptr<ast::RootNode> root) {
	GlobalScope gs;

	auto mainScope = std::make_shared<Scope>("__main");
	gs.scopes.push_back(mainScope);

	auto mainBlock = gs.bblocks["__main"] = new BBlock(mainScope);

	root->convert(mainBlock, gs);

	return gs;
}


void ThreeAddr::dump() const {
	toDot(0, std::cout);
	std::cout << std::endl;
}

void ThreeAddr::toDot(int id, std::ostream& out) const {
	if (id)
		out << '|' << "<i" << id << '>';
	out << name << " := " << '\'' << lhs << '\'' << " " << op << " " << '\'' << rhs << '\'';
}

void ThreeAddr::toASM(std::ostream& out) const {

}

void BBlock::dump() const {
	std::cout << "BBlock @ " << name << " (" << this << ')' << std::endl;
	for (const auto& i : instructions)
		i.dump();
	std::cout << "True:  " << (tExit ? tExit->name : "") << " (" << tExit << ')'
						<< std::endl;
	std::cout << "False: " << (fExit ? fExit->name : "") << " (" << fExit << ')'
						<< std::endl;
}

void BBlock::toDot(std::ostream& out) const {
	out << name << "[label=\"{<i0>";
	for (size_t i = 0; i < instructions.size(); i++)
		instructions[i].toDot(i, out);
	out << "}\"];" << std::endl;

	int lastID = instructions.size();
	if (lastID)
		lastID--;

	if (fExit) {
		out << name << ":i" << lastID << " -> " << tExit->name
				<< ":i0[color=green,label=\"true\"];" << std::endl;
		out << name << ":i" << lastID << " -> " << fExit->name
				<< ":i0[color=red,label=\"false\"];" << std::endl;
	} else if (tExit)
		out << name << ":i" << lastID << " -> " << tExit->name << ":i0;"
				<< std::endl;
}

void BBlock::toASM(std::ostream& out) const {
	out << name << ":" << std::endl;
	if (fExit) {
		out << "\tif (1 || 0)\n\t\tgoto "
				<< tExit->name << ";\n\telse\n\t\tgoto " << fExit->name << ";"
				<< std::endl;
	} else if (tExit)
		out << "\tgoto " << tExit->name << ";" << std::endl;
}
