/* -*- mode: c++; c-set-style: cc-mode -*- */
#include <cstdlib>
#include <iostream>
#include <lua.tab.hpp>

#include <fstream>

ast::RootNodePtr root;
yy::location loc;

extern FILE* yyin;

void yy::parser::error(const location_type& loc, const std::string& err) {
	std::cerr << *loc.begin.filename <<	":" << loc.begin.line << ":" << loc.begin.column;
	/*if (loc.begin.line == loc.end.line) std::cerr << " --> " << loc.end.column;*/
	std::cerr << "\n" << err << std::endl;
	exit(1);
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

	std::cout << "Built a parse-tree:" << std::endl;

	root.print(std::cout);

	std::ofstream out("graph.dot");
	out << "digraph G {"  << std::endl;
	root.toDot(out);
	out << "}";

	fclose(yyin);
	return 0;
}
