/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <string>
#include <ast.hpp>
#include <expect.hpp>

#define debug() Debug debug(this->toString().c_str())

static int indent = 0;
struct Debug {
	Debug(const char* func) {
		// scope.print();
		indent++;
		for (int i = 0; i < indent; i++)
			if (!i)
				std::cout << "| ";
			else
				std::cout << "  ";
		std::cout << func << std::endl;
	}
	~Debug() { indent--; }
};


std::string ast::RootNode::convert(BBlock* out, GlobalScope& gs) {
	debug();

	for (auto child : children)
		child->convert(out, gs);

	return "";
}

std::string ast::NumberNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	
	return std::to_string(value);
}
std::string ast::StringNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	return value;
}

std::string ast::BoolNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::NilNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::AssignValuesNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	std::string l = left()->convert(out, gs);
	std::string r = right()->convert(out, gs);
	out->instructions.push_back(ThreeAddr(l, Operation::constant, r, r));
	return l;
}
std::string ast::BinOPNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	std::string l = left()->convert(out, gs);
	std::string r = right()->convert(out, gs);
	std::string tmp = out->scope->makeName();

	Operation operation;

	using OP = ast::token::BinOPToken::OP;
	switch (op.op) {
	case OP::plus:
		operation = Operation::plus;
		break;
	case OP::minus:
		operation = Operation::minus;
		break;
	case OP::mul:
		operation = Operation::mul;
		break;
	case OP::div:
		operation = Operation::div;
		break;
	case OP::pow:
		operation = Operation::pow;
		break;
	case OP::mod:
		operation = Operation::mod;
		break;
	default:
		expect(0, op.toString() + ": is not implemented");
	}

	out->instructions.push_back(ThreeAddr(tmp, operation, l, r));
	return tmp;
}
std::string ast::BreakNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ChunkNode::convert(BBlock* out, GlobalScope& gs) {
	debug();

	for (auto child : children)
		child->convert(out, gs);

	return "";
}
std::string ast::FunctionCallNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	std::string l = function()->convert(out, gs);
	std::string r = arguments()->convert(out, gs);
	std::string tmp = out->scope->makeName();
	out->instructions.push_back(ThreeAddr(tmp, Operation::call, l, r));
	return tmp;
}
std::string ast::FunctionNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::IndexOfNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::LocalAssignValueNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ReturnNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::TableNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::UnOPNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ValueNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::VariableListNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ExpressionListNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(children.size() == 1, "ExpressionListNode can only have one child");
	return children[0]->convert(out, gs);
}
std::string ast::NameListNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::VariableRefNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	return isToken ? name.value : children[0]->convert(out, gs);
}
std::string ast::ForLoopNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::IfNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::RepeatNode::convert(BBlock* out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
