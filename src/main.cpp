/* -*- mode: c++; c-set-style: cc-mode -*- */
#include <controlflowgraph.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <lua.tab.hpp>
#include <set>

// syscall numbers __NR_*
#include <asm/unistd_64.h>

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
	auto doFunction = [func](std::string name, BBlock* first) {
		std::set<BBlock*> done, todo;
		todo.insert(first);
		while (todo.size() > 0) {
			// Pop an arbitrary element from todo set
			auto first = todo.begin();
			BBlock* next = *first;
			todo.erase(first);
			func(name, next);
			done.insert(next);
			if (next->tExit != nullptr && done.find(next->tExit) == done.end())
				todo.insert(next->tExit);
			if (next->fExit != nullptr && done.find(next->fExit) == done.end())
				todo.insert(next->fExit);
		}
	};
	for (std::pair<std::string, BBlock*> bb : gs.bblocks)
		doFunction(bb.first, bb.second);
}

void dumpCFG(GlobalScope& gs) {
	visitAllBlocks(gs, [](std::string, BBlock* block) { block->dump(); });
}

void toDot(std::ostream& out, GlobalScope& gs) {
	std::string sName;
	visitAllBlocks(gs, [&out, &sName](std::string name, BBlock* block) {
		if (sName != name) {
			sName = name;
			out << name << "[label=\"" << name
					<< "\",shape=ellipse,color=grey,style=filled];" << std::endl;
			out << name << " -> " << block->name << std::endl;
		}
		block->toDot(out);
	});
}

void toASM(std::ostream& out, GlobalScope& gs) {
	std::string sName;
	visitAllBlocks(gs, [&out, &sName](std::string name, BBlock* block) {
		if (sName != name) {
			if (sName.size()) {
				out << "\tret\n.size ., .-" << sName << std::endl;
			}
			sName = name;
			out << name << ".global " << name << "\n.type " << name << ", %function\n"
					<< name << ": " << std::endl;
		}
		block->toASM(out);
	});
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
		out << "graph [compound=true];" << std::endl;
		toDot(out, gs);

		out << "globals_lbl[label=\"Globals\",shape=ellipse,color=grey,style="
					 "filled];"
				<< std::endl;
		out << "globals_lbl -> globals;" << std::endl;
		out << "globals[label=\"{";
		bool first = true;
		for (const std::string& str : gs.globals) {
			if (!first)
				out << "|";
			first = false;
			out << str;
		}
		out << "}\"];" << std::endl;

		out << "constants_lbl[label=\"Constants\",shape=ellipse,color=grey,style="
					 "filled];"
				<< std::endl;
		out << "constants_lbl -> constants;" << std::endl;
		out << "constants[label=\"{";
		first = true;
		for (const std::pair<std::string, Value>& kv : gs.constants) {
			if (!first)
				out << "|";
			first = false;
			out << kv.first << "=" << kv.second;
		}
		out << "}\"];" << std::endl;
		out << "}" << std::endl;
	}

	{
			std::ofstream out("target-raw.s");
			out << ".code64\n"
					".text\n"
					"\n"
					".global printf\n"
					"\n"
					".global _start\n"
					".type _start, %function\n"
					"_start:\n"
					"\tmov $0, %rbp\n"
					"\tcall main\n"
					"\tmov %rax, %rdi\n"
					"\tmovq $"
					<< __NR_exit
					<< ", %rax\n"
					"\tsyscall\n"
					".size ., .-_start\n"
					"\n"
					".global main\n"
					".type main, %function\n"
					"main:\n"
					"\tpushq %rbp\n"
					"\tmov %rsp, %rbp\n"
					"\tcall __main\n"
					"\txor %rax, %rax\n"
					"\tpop %rbp\n"
					"\tret\n"
					".size ., .-main\n"
					<< std::endl;
			out << ".rodata\n";
			for (const std::pair<std::string, Value>& kv : gs.constants) {
					out << kv.first << ": ";
					switch (kv.second.type) {
					case Value::Type::nil:
							out << ".quad 0";
							break;
					case Value::Type::string:
							out << ".ascii \"" << kv.second.str << "\0\"";
							break;
					case Value::Type::number:
							out << ".double " << kv.second.number;
							break;
					case Value::Type::boolean:
							out << ".quad " << kv.second.boolean ? 1 : 0;
							break;
					default:
							expect(0, "Not implemented");
					}
					out << std::endl;
		}
		toASM(out, gs);
	}

	fclose(yyin);
	return 0;
}
