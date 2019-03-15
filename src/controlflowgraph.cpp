/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <algorithm>
#include <ast.hpp>
#include <cctype>
#include <controlflowgraph.hpp>
#include <expect.hpp>
#include <iostream>
#include <set>
#include <token.hpp>

int BBlock::blockCounter = 0;

void Scope::print() {
	std::cout << "Scope(prefix: " << prefix << ", tmpCounter: " << tmpCounter
						<< ")" << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Value& value) {
	switch (value.type) {
		case Value::Type::nil:
			out << "NIL";
			break;
		case Value::Type::string:
			out << value.str;
			break;
		case Value::Type::number:
			out << value.number;
			break;
		case Value::Type::boolean:
			out << value.boolean;
			break;
		default:
			expect(0, "Not implemented");
	}
	return out;
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
std::string GlobalScope::addConstant(const Value& value) {
	std::string index = std::string("_c") + (char)value.type + "_" + value.str;
	std::replace_if(index.begin(), index.end(),
									[](char ch) { return !isalnum(ch); }, '_');
	if (constants[index].type !=
			Value::Type::UNK)  // There is already a constant with this index
		expect(constants[index].str == value.str,
					 "Two different constants generated the same name: " + index +
							 ", test: '" + constants[index].str + "' != '" + value.str + "'");
	constants[index] = value;
	return index;
}

void GlobalScope::addGlobal(const std::string& value) {
	auto it = std::find(globals.begin(), globals.end(), value);
	if (it == globals.end())
		globals.push_back(value);
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
	out << name << " := " << '\'' << lhs << '\'' << " " << op << " " << '\''
			<< rhs << '\'';
}

void ThreeAddr::toASM(std::ostream& out) const {
	out << "\t// Expand: " << name << " := " << lhs << " " << op << " " << rhs
			<< std::endl;
#define _(...) out << "\t" << __VA_ARGS__ << std::endl;
	_("movq %[" << lhs << "], \t%%rax");
	_("movq %[" << rhs << "], \t%%rbx");
	switch (op) {
		case Operation::constant:
			_("// copy is a dummy operation");
			break;
		default:
			out << "\t// Unknown op = '" << op << "'!" << std::endl;
			_("nop");
			break;
	}

	// NOTE: The never clean %rax at the end of each statement as this value will
	//       be used as the return value.
	_("movq %%rax, \t%[" << name << "]");
#undef _
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
	out << "." << name << ":" << std::endl;

	for (size_t i = 0; i < instructions.size(); i++)
		instructions[i].toASM(out);

	if (fExit) {
		out << "test %rax, %rax" << std::endl;
		out << "jnz ." << tExit->name << std::endl;
		out << "jmp ." << fExit->name << std::endl;
	} else if (tExit)
		out << "jmp ." << tExit->name << std::endl;
}
