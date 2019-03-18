/* -*- mode: c++; c-set-style: cc-mode -*- */

#include <algorithm>
#include <ast.hpp>
#include <expect.hpp>
#include <string>

#define debug() Debug debug(this->toString().c_str())

static int indent = 0;
struct Debug {
	Debug(const char* func) {
		// scope.print();
		/*indent++;
		for (int i = 0; i < indent; i++)
			if (!i)
				std::cout << "| ";
			else
				std::cout << "  ";
		std::cout << func << std::endl;*/
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
	return gs.addConstant(Value(value));
}
std::string ast::StringNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	return gs.addConstant(Value(value));
}

std::string ast::BoolNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	return gs.addConstant(Value(value));
}
std::string ast::NilNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	return gs.addConstant(Value(NIL()));
}

std::string ast::AssignValuesNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	auto& left = keys()->children;
	auto& right = values()->children;

	std::vector<std::string> tmpVars(right.size());
	for (size_t i = 0; i < right.size(); i++)
		tmpVars[i] = right[i]->convert(out, gs);
	for (size_t i = 0; i < left.size(); i++) {
			//TODO: Add check if indexof, then rewire to indexofRef? or use indexOfAssign? or something
		std::string l = left[i]->convert(out, gs);
		out->instructions.push_back(
				ThreeAddr(l, Operation::constant, tmpVars[i], tmpVars[i]));
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

		case OP::less:
			operation = Operation::less;
			break;
		case OP::lessOrEqual:
			operation = Operation::lequal;
			break;
		case OP::more:
			operation = Operation::greater;
			break;
		case OP::moreOrEqual:
			operation = Operation::gequal;
			break;
		case OP::equal:
			operation = Operation::equal;
			break;
		case OP::notEqual:
			operation = Operation::notequal;
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
	std::string tmp;

	// Hack because test6 contains a print that takes two arguments
	for (auto child : arguments()->children) {
		std::string r = child->convert(out, gs);
		tmp = out->scope->makeName();
		out->instructions.push_back(ThreeAddr(tmp, Operation::call, l, r));
	}
	return tmp;
}
std::string ast::FunctionNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	// TODO: move
	static int i = 0;

	std::string funName = "_func" + std::to_string(i++);

	auto funcScope = std::make_shared<Scope>(funName);
	gs.scopes.push_back(funcScope);

	auto funcBlock = gs.bblocks[funName] = new BBlock(funcScope);

	// TODO: Make better
	auto& children = arguments()->children;
	for (size_t i = 0; i < children.size(); i++) {
		auto val = std::to_string(i);
		funcBlock->instructions.push_back(ThreeAddr(
				children[i]->convert(funcBlock, gs), Operation::functionArg, val, val));
	}

	body()->convert(funcBlock, gs);

	return funName;
}
std::string ast::IndexOfNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string l = object()->convert(out, gs);
	std::string r = index()->convert(out, gs);
	std::string rTmp = gs.addConstant(Value{r});
	std::string tmp = out->scope->makeName();
	out->instructions.push_back(ThreeAddr(tmp, Operation::indexof, l, rTmp));
	return tmp;
}
std::string ast::LocalAssignValueNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ReturnNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	BBlock* returnBlock = new BBlock(out->scope);
	out->tExit = returnBlock;

	std::string val = returnValue()->convert(returnBlock, gs);
	std::string tmp = out->scope->makeName();
	returnBlock->instructions.push_back(
			ThreeAddr(tmp, Operation::constant, val, val));

	out = returnBlock;

	return "";
}
std::string ast::TableNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string tbl = out->scope->makeName();
	std::string len = std::to_string(children.size());
	out->instructions.push_back(ThreeAddr(tbl, Operation::emptyTable, len, len));
	for (auto& child : children) {
		auto name = child->convert(out, gs);
		out->instructions.push_back(
				ThreeAddr(tbl, Operation::concatTable, tbl, name));
	}
	return tbl;
}
std::string ast::UnOPNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	std::string val = value()->convert(out, gs);
	std::string tmp = out->scope->makeName();

	Operation operation;

	using OP = ast::token::UnOPToken::OP;
	switch (op.op) {
		case OP::pound:
			operation = Operation::pound;
			break;

		default:
			expect(0, op.toString() + ": is not implemented");
	}

	out->instructions.push_back(ThreeAddr(tmp, operation, val, val));
	return tmp;
}
std::string ast::ValueNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	if (dynamic_cast<ast::token::NilToken*>(value.get()))
		return gs.addConstant(Value(NIL()));
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::VariableListNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	expect(0, "NOT IMPLEMENTED!");
}
std::string ast::ExpressionListNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();

	if (children.size() == 1)
		return children[0]->convert(out, gs);

	// In test6 io.write takes two arguments so this is needed, sadly :c
	// aka THE INSTRUCTIONS ARE WRONG!
	std::string tbl = out->scope->makeName();
	std::string len = std::to_string(children.size());
	out->instructions.push_back(ThreeAddr(tbl, Operation::emptyTable, len, len));
	for (auto& child : children) {
		auto name = child->convert(out, gs);
		out->instructions.push_back(
				ThreeAddr(tbl, Operation::concatTable, tbl, name));
	}
	return tbl;
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

	// TODO: fix hack
	bool backupVar =
			std::find(vars.begin(), vars.end(), counterName) != vars.end() ||
			std::find(gs.globals.begin(), gs.globals.end(), counterName) !=
					gs.globals.end();
	if (backupVar) {
		std::string backupVarName = out->scope->makeName();
		out->instructions.push_back(ThreeAddr(backupVarName, Operation::constant,
																					counterName, counterName));
		doneBlock->instructions.push_back(ThreeAddr(
				counterName, Operation::constant, backupVarName, backupVarName));
	} else {
		vars.push_back(counterName);
		auto nil = gs.addConstant(Value(NIL()));
		doneBlock->instructions.push_back(
				ThreeAddr(counterName, Operation::constant, nil,
									nil));  // TODO: replace with real nil value
	}

	// counter = start
	out->instructions.push_back(
			ThreeAddr(counterName, Operation::constant, start, start));

	// if (counter <= end) goto body; else goto done
	compareBlock->instructions.push_back(
			ThreeAddr(out->scope->makeName(), Operation::lequal, counterName, end));

	body()->convert(bodyBlock, gs);
	{
		std::string tmp = out->scope->makeName();
		bodyBlock->instructions.push_back(
				ThreeAddr(tmp, Operation::plus, counterName, "1"));
		bodyBlock->instructions.push_back(
				ThreeAddr(counterName, Operation::constant, tmp, tmp));
	}
	bodyBlock->tExit = compareBlock;

	out = doneBlock;
	return "";
}
std::string ast::IfNode::convert(BBlock*& out, GlobalScope& gs) {
	debug();
	check()->convert(out, gs);
	BBlock* truePathEndNode = new BBlock(out->scope);
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
	auto bodyBlock = new BBlock(out->scope);
	auto compareBlock = new BBlock(out->scope);
	auto doneBlock = new BBlock(out->scope);

	out->tExit = bodyBlock;

	check()->convert(compareBlock, gs);
	compareBlock->tExit = bodyBlock;
	compareBlock->fExit = doneBlock;

	body()->convert(bodyBlock, gs);
	bodyBlock->tExit = compareBlock;

	out = doneBlock;
	return "";
}
