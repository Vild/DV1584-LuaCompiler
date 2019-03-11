/* -*- mode: c++; c-set-style: cc-mode -*- */
#include <controlflowgraph.hpp>
#include <cstdlib>
#include <iostream>
#include <lua.tab.hpp>
#include <set>
#include <fstream>

std::shared_ptr<ast::RootNode> root;
yy::location loc;

extern FILE* yyin;

void yy::parser::error(const location_type& loc, const std::string& err) {
	std::cerr << *loc.begin.filename << ":" << loc.begin.line << ":"
						<< loc.begin.column;
	/*if (loc.begin.line == loc.end.line) std::cerr << " --> " <<
	 * loc.end.column;*/
	std::cerr << "\n" << err << std::endl;
	exit(1);
}

template <typename Func>
void visitAllBlocks(GlobalScope& gs, Func func) {
	std::set<BBlock*> done, todo;
	for (std::pair<std::string, BBlock*> bb : gs.bblocks)
		todo.insert(bb.second);
	while (todo.size() > 0) {
		// Pop an arbitrary element from todo set
		auto first = todo.begin();
		BBlock* next = *first;
		todo.erase(first);
		func(next);
		done.insert(next);
		if (next->tExit != nullptr && done.find(next->tExit) == done.end())
			todo.insert(next->tExit);
		if (next->fExit != nullptr && done.find(next->fExit) == done.end())
			todo.insert(next->fExit);
	}
}

void dumpCFG(GlobalScope& gs) {
	visitAllBlocks(gs, [](BBlock* block) { block->dump(); });
}

void toDot(std::ostream& out, GlobalScope& gs) {
	visitAllBlocks(gs, [&out](BBlock* block) { block->toDot(out); });
}

void toASM(std::ostream& out, GlobalScope& gs) {
	visitAllBlocks(gs, [&out](BBlock* block) { block->toASM(out); });
}

int main(int argc, char** argv) {
	std::string name = "stdin";
	if (argc > 1 && std::string(argv[1]) != "-") {
		name = argv[1];
		yyin = fopen(argv[1], "rb");
	}
	loc.initialize(&name);

	yy::parser parser;
	if (parser.parse())
		return 1;

	// std::cout << "Built a parse-tree:" << std::endl;

	// root->print(std::cout);

	std::ofstream out("parse.dot");
	out << "digraph G {" << std::endl;
	root->toDot(out);
	out << "}";
	out.close();

	// std::cout << "Evaluating:" << std::endl;

	GlobalScope gs = getBBlocks(root);

	dumpCFG(gs);
	{
    std::ofstream out("cfg.dot");
    out << "digraph bblocks {" << std::endl;
    out << "node [shape=record];" << std::endl;
    toDot(out, gs);
    out << "}" << std::endl;
	}


	fclose(yyin);
	return 0;
}
