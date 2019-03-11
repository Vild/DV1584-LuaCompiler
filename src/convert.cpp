/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <string>
#include <ast.hpp>
#include <expect.hpp>
#include <algorithm>

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


std::string ast::RootNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	for (auto child : children)
		child->convert(out, gs);

	return "";
}

std::string ast::NumberNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	
	return std::to_string(value);
}
std::string ast::StringNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	return value;
}

std::string ast::BoolNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::NilNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}

std::string ast::AssignValuesNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	auto& left = keys()->children;
	auto& right = values()->children;

	std::vector<std::string> tmpVars(right.size());
	for (size_t i = 0; i < right.size(); i++)
		tmpVars[i] = right[i]->convert(out, gs);
	for (size_t i = 0; i < left.size(); i++) {
		std::string l = left[i]->convert(out, gs);
		out->instructions.push_back(ThreeAddr(l, Operation::constant, tmpVars[i], tmpVars[i]));
		gs.globals.push_back(l);
	}
	return "";
}
std::string ast::BinOPNode::convert(BBlock*& out, GlobalScope& gs) {
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

	case OP::equal:
		operation = Operation::equal;
		break;
	default:
		expect(0, op.toString() + ": is not implemented");
	}

	out->instructions.push_back(ThreeAddr(tmp, operation, l, r));
	return tmp;
}
std::string ast::BreakNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ChunkNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	for (auto child : children)
		child->convert(out, gs);

	return "";
}
std::string ast::FunctionCallNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string l = function()->convert(out, gs);
	std::string r = arguments()->convert(out, gs);
	std::string tmp = out->scope->makeName();
	out->instructions.push_back(ThreeAddr(tmp, Operation::call, l, r));
	return tmp;
}
std::string ast::FunctionNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::IndexOfNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string l = object()->convert(out, gs);
	std::string r = index()->convert(out, gs);
	std::string tmp = out->scope->makeName();
	out->instructions.push_back(ThreeAddr(tmp, Operation::indexof, l, r));
	return tmp;
}
std::string ast::LocalAssignValueNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ReturnNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::TableNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string tbl = out->scope->makeName();
	std::string len = std::to_string(children.size());
	out->instructions.push_back(ThreeAddr(tbl, Operation::emptyTable, len, len));
	for (auto& child : children) {
		auto name = child->convert(out, gs);
		out->instructions.push_back(ThreeAddr(tbl, Operation::concatTable, tbl, name));
	}
	return "";
}
std::string ast::UnOPNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ValueNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::VariableListNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ExpressionListNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(children.size() == 1, "ExpressionListNode can only have one child");
	return children[0]->convert(out, gs);
}
std::string ast::NameListNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::VariableRefNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	return isToken ? name.value : children[0]->convert(out, gs);
}
std::string ast::ForLoopNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	auto compareBlock = new BBlock(out->scope);
	auto bodyBlock = new BBlock(out->scope);
	auto doneBlock = new BBlock(out->scope);

	out->tExit = compareBlock;
	compareBlock->tExit = bodyBlock;
	compareBlock->fExit = doneBlock;

	auto& vars = out->scope->variables;

	std::string start = from()->convert(out, gs);
	std::string end = to()->convert(out, gs);

	std::string counterName = counter()->convert(compareBlock, gs);

	//TODO: fix hack
	bool backupVar = std::find(vars.begin(), vars.end(), counterName) != vars.end() ||
		std::find(gs.globals.begin(), gs.globals.end(), counterName) != gs.globals.end();
	if (backupVar) {
		std::string backupVarName = out->scope->makeName();
		out->instructions.push_back(ThreeAddr(backupVarName, Operation::constant, counterName, counterName));
		doneBlock->instructions.push_back(ThreeAddr(counterName, Operation::constant, backupVarName, backupVarName));
	} else {
		vars.push_back(counterName);
		doneBlock->instructions.push_back(ThreeAddr(counterName, Operation::constant, "NIL", "NIL")); //TODO: replace with real nil value
	}

	// counter = start
	out->instructions.push_back(ThreeAddr(counterName, Operation::constant, start, start));

	// if (counter <= end) goto body; else goto done
	compareBlock->instructions.push_back(ThreeAddr(out->scope->makeName(), Operation::lequal, counterName, end));

	body()->convert(bodyBlock, gs);
	{
		std::string tmp = out->scope->makeName();
		bodyBlock->instructions.push_back(ThreeAddr(tmp, Operation::plus, counterName, "1"));
		bodyBlock->instructions.push_back(ThreeAddr(counterName, Operation::constant, tmp, tmp));
	}
	bodyBlock->tExit = compareBlock;

	out = doneBlock;
	return "";
}
std::string ast::IfNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	check()->convert(out, gs);
	BBlock* truePathEndNode =new BBlock(out->scope);
	out->tExit = truePathEndNode;
	body()->convert(truePathEndNode, gs);
	BBlock* falsePathEndNode = new BBlock(out->scope);
	out->fExit = falsePathEndNode;
	elseBody()->convert(falsePathEndNode, gs);
	BBlock* exitBlock = new BBlock(out->scope);
	truePathEndNode->tExit = falsePathEndNode->tExit = exitBlock;
	out = exitBlock;
	return "";
}
std::string ast::RepeatNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
