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

int errors = 0;

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
	auto newFunc = [&out](std::string name, BBlock* block) {
		if (!name.size())
			return;
		auto& vars = block->scope->variables;
		auto size = vars.size() * 16 /* Each variable is 16-bytes */;
		out << ".global " << name << std::endl;
		out << ".type " << name << ", %function" << std::endl;
		out << name << ": " << std::endl;
		out << "\tpushq %rbp" << std::endl;
		out << "\tmov %rsp, %rbp" << std::endl;
		if (size) {
			out << "\tsub $" << size << ", %rsp" << std::endl;
			out << "\tand $-16, %rsp" << std::endl;
			for (size_t i = 0; i < vars.size(); i++) {
				std::string variable = vars[i];
				out << "\t// " << variable << " is at &(-"
						<< (i + 1) * 16 /* The size of a variable */ << "(%rbp))"
						<< std::endl;
			}
		}
	};
	auto endFunc = [&out](std::string name) {
		if (!name.size())
			return;
		out << ".L" << name << "_return:" << std::endl;
		out << "\tleave" << std::endl;
		out << "\tret" << std::endl;
		out << "\n.size ., .-" << name << std::endl;
		out << std::endl;
	};
	visitAllBlocks(
			gs, [&out, &sName, newFunc, endFunc](std::string name, BBlock* block) {
				if (sName != name) {
					endFunc(sName);
					sName = name;
					newFunc(sName, block);
				}
				block->toASM(out);
			});
	endFunc(sName);
}

int main(int argc, char** argv) {
	std::string name = "stdin";
	if (argc > 1 && std::string(argv[1]) != "-") {
		name = argv[1];
		yyin = fopen(argv[1], "rb");
		if (!yyin) {
			std::cerr << argv[1] << " does not exist!" << std::endl;
			return 1;
		}
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

	// dumpCFG(gs);
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
		out << "\t.set __NR_exit, " << __NR_exit << std::endl;
		out << "\t.set __NR_write, " << __NR_write << std::endl;
		out << "\t.set __NR_read, " << __NR_read << std::endl;
		{
			std::ifstream in("runtime/prologue.s");
			std::copy(std::istreambuf_iterator<char>(in),
								std::istreambuf_iterator<char>(),
								std::ostreambuf_iterator<char>(out));
		}

		out << std::endl;

		toASM(out, gs);

		out << std::endl;

		{
			std::ifstream in("runtime/epilogue.s");
			std::copy(std::istreambuf_iterator<char>(in),
								std::istreambuf_iterator<char>(),
								std::ostreambuf_iterator<char>(out));
		}

		out << std::endl;

		{
			std::ifstream in("runtime/builtin.s");
			std::copy(std::istreambuf_iterator<char>(in),
								std::istreambuf_iterator<char>(),
								std::ostreambuf_iterator<char>(out));
		}

		out << std::endl;

		out << ".section .rodata\n";
		for (const std::pair<std::string, Value>& kv : gs.constants) {
			out << ".align 8" << std::endl;
			out << kv.first << ": " << std::endl;
			out << "\t.quad '" << (char)kv.second.type << "'" << std::endl;
			switch (kv.second.type) {
				case Value::Type::nil:
					out << "\t.quad 0";
					break;
				case Value::Type::string:
					out << "\t.quad 1f" << std::endl;
					out << "\t\t1: .asciz \"" << kv.second.str << "\"";
					break;
				case Value::Type::number:
					out << "\t.double " << kv.second.number;
					break;
				case Value::Type::boolean:
					out << "\t.quad " << (kv.second.boolean ? "1" : "0");
					break;
				default:
					out << " // Not implemented";
					break;
			}
			out << std::endl;
		}

		out << ".bss\n";
		for (const std::string& str : gs.globals) {
			out << ".align 8" << std::endl;
			out << str << ": " << std::endl;
			out << "\t.quad 0" << std::endl;  // TYPE::UNK
			out << "\t.quad 0" << std::endl;
		}
		for (auto scope : gs.scopes) {
			for (int i = 0; i < scope->tmpCounter; i++) {
				out << ".align 8" << std::endl;
				out << scope->prefix << "_" << i << ": " << std::endl;
				out << "\t.quad 0" << std::endl;  // TYPE::UNK
				out << "\t.quad 0" << std::endl;
			}
		}

		// Need to be last
		out << "// Define structure layout\n"
					 "\t.struct 0\n"
					 "type:\n"
					 "\t.struct type + 8\n"
					 "data:\n"
					 "\t.struct 0\n"
					 "obj_size:\n"
					 "\t.struct obj_size + 8\n"
					 "obj_data:\n"
					 "\t.struct 0\n"
					 "obj_data_name:\n"
					 "\t.struct obj_data_name + 8\n"
					 "obj_data_var:\n"
					 "\t.struct obj_data_var + 8\n"
					 "obj_data_sizeof:\n"
					 "\t.struct 0"
				<< std::endl;
	}

	fclose(yyin);

	if (errors)
		std::cout << std::endl << std::endl << std::endl << std::endl;
	return errors;
}
