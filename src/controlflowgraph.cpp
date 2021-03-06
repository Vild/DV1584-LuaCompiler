/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <algorithm>
#include <ast.hpp>
#include <cctype>
#include <controlflowgraph.hpp>
#include <cstring>
#include <expect.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <token.hpp>

// From main.cpp
extern int errors;

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

GlobalScope::GlobalScope() {
	addConstant(Value{NIL()});
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
std::string GlobalScope::addData(const Value& value) {
	std::string index = std::string("_d") + (char)value.type + "_" + value.str;
	std::replace_if(index.begin(), index.end(),
	                [](char ch) { return !isalnum(ch); }, '_');
	if (data[index].type !=
	    Value::Type::UNK)  // There is already a constant with this index
		expect(data[index].str == value.str,
		       "Two different data generated the same name: " + index +
		           ", test: '" + data[index].str + "' != '" + value.str + "'");
	data[index] = value;
	return index;
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

static void pushVar(std::ostream& out,
                    const BBlock* block,
                    std::string name,
                    std::string reg) {
	auto& vars = block->scope->variables;
	ptrdiff_t pos =
	    std::distance(vars.begin(), std::find(vars.begin(), vars.end(), name));

	if (pos < (ptrdiff_t)vars.size()) {  // a local variable
		out << "\tlea -" << (pos + 1) * 16 << "(%rbp), %" << reg << std::endl;
	} else if (strncmp(name.c_str(), block->scope->prefix.c_str(),
	                   block->scope->prefix.size()) == 0) {
		std::istringstream iss(name.substr(block->scope->prefix.size() + 1));
		size_t idx;
		iss >> idx;
		out << "\tlea -" << (vars.size() + idx + 1) * 16 << "(%rbp), %" << reg
		    << std::endl;
	} else {
		out << "\tmovq $" << name << ", %" << reg << std::endl;
	}
}

void ThreeAddr::toASM(std::ostream& out, const BBlock* block) const {
	out << "\t// Expand: " << name << " := " << lhs << " " << op << " " << rhs
	    << std::endl;
#define _(...) out << "\t" << __VA_ARGS__ << std::endl;
	if (op == Operation::functionArg) {
		pushVar(out, block, name, "rdx");  // Return place
		// The function will already have the input argument in %rsi so this will
		// work
		_("call copyOP");
		return;
	} else if (op == Operation::returnValue) {
		pushVar(out, block, lhs, "rdi");
		_("mov type(%rdi), %rax");
		_("mov data(%rdi), %rdx");
		_("leave");
		_("ret");
		return;
	}
	pushVar(out, block, lhs, "rdi");   // Arg 1
	pushVar(out, block, rhs, "rsi");   // Arg 2
	pushVar(out, block, name, "rdx");  // Return place
	switch (op) {
			/* Values (lhs) */
		case Operation::constant:
			_("call copyOP");
			break;
		case Operation::emptyTable:
			_("call emptyTableOP");
			break;
		case Operation::preMinus:
			_("call preMinusOP");
			break;
		case Operation::not_:
			_("call notOP");
			break;
		case Operation::pound:
			_("call poundOP");
			break;

			/* Math (lhs & rhs) */
		case Operation::plus:
			_("call plusOP");
			break;
		case Operation::minus:
			_("call minusOP");
			break;
		case Operation::mul:
			_("call mulOP");
			break;
		case Operation::div:
			_("call divOP");
			break;
		case Operation::pow:
			_("call powOP");
			break;
		case Operation::mod:
			_("call modOP");
			break;

			/* Compare (lhs & rhs) */
		case Operation::less:
			_("call lessOP");
			break;
		case Operation::lequal:
			_("call lequalOP");
			break;
		case Operation::greater:
			_("call greaterOP");
			break;
		case Operation::gequal:
			_("call gequalOP");
			break;
		case Operation::equal:
			_("call equalOP");
			break;
		case Operation::notequal:
			_("call notequalOP");
			break;

			/* Misc */
		case Operation::call:
			// TODO: implement check!
			_("call callOP");
			break;
		case Operation::indexof:
			_("call indexofOP");
			break;
		case Operation::indexofRef:
			_("call indexofRefOP");
			break;
		case Operation::concatTable:
			_("call concatTableOP");
			break;
		case Operation::functionArg:
			expect(0, "Should never reach this");
			break;
		case Operation::setValue:
			_("call setValueOP");
			break;
		default:
			out << "\t// Unknown op = '" << op << "'!" << std::endl;
			std::cout << "\x1b[1;31mUnknown op = '" << op << "'!\x1b[0m" << std::endl;
			errors++;
			_("int $3");
			break;
	}
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
	out << ".L" << name << ":" << std::endl;

	for (size_t i = 0; i < instructions.size(); i++)
		instructions[i].toASM(out, this);

	if (fExit) {
		expect(instructions.size() > 0, "BBlock has a fExit, but no instructions!");
		pushVar(out, this, instructions[instructions.size() - 1].name, "rax");
		out << "\tmov data(%rax), %rax" << std::endl;
		out << "\ttest %rax, %rax" << std::endl;
		out << "\tjnz .L" << tExit->name << std::endl;
		out << "\tjmp .L" << fExit->name << std::endl;
	} else if (tExit)
		out << "\tjmp .L" << tExit->name << std::endl;
	else
		out << "\tjmp .L" << scope->prefix << "_return" << std::endl;
}
