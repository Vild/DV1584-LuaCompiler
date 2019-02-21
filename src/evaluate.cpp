#include <evaluate.hpp>
#include <ast.hpp>
#include <token.hpp>
#include <expect.hpp>
#include <cassert>
#include <cmath>
#include <iostream>

static std::shared_ptr<ast::NilNode> nilNode;

static int indent = 0;
#define debug() Debug debug(this->toString().c_str())

struct Debug {
	Debug(const char* func) {
		indent++;
		for (int i = 0; i < indent; i++) std::cout << "  ";
		std::cout << func << std::endl;
	}
	~Debug() { indent--; }
};

Scope::~Scope() {
	std::cout << "Destroying scope!" << std::endl;
	for (std::pair<std::string, std::shared_ptr<ast::Node>> kv : environment) {
		std::cout << "\t'" << kv.first << "' = '" << kv.second->toString() << "'" << std::endl;
	}
}

void Scope::set(std::string key, std::shared_ptr<ast::Node> value) {
	environment[key] = value;
}
void Scope::searchAndSet(std::string key, std::shared_ptr<ast::Node> value) {
	auto it = environment.find(key);
	if (it != environment.end())
		environment[key] = value;
	else if (parent)
		parent->searchAndSet(key, value);
	else
		environment[key] = value;
}

std::shared_ptr<ast::Node> Scope::get(std::string key) {
	auto it = environment.find(key);
	if (it != environment.end())
		return it->second;
	else if (parent)
		return parent->get(key);
	else
		return nullptr;
}

void evaluate(std::shared_ptr<ast::RootNode> root) {
	if (!nilNode)
		nilNode = std::make_shared<ast::NilNode>();

	Scope scope{};

	// Initialize io.print here

	root->visit(scope);
}

std::shared_ptr<ast::Node> ast::NumberNode::visit(Scope& scope) {
	debug();
	return std::make_shared<ast::NumberNode>(*this);
}

std::shared_ptr<ast::Node> ast::StringNode::visit(Scope& scope) {
	debug();
	return std::make_shared<ast::StringNode>(*this);
}

std::shared_ptr<ast::Node> ast::BoolNode::visit(Scope& scope) {
	debug();
	return std::make_shared<ast::BoolNode>(*this);
}

std::shared_ptr<ast::Node> ast::NilNode::visit(Scope& scope) {
	debug();
	return std::make_shared<ast::NilNode>(*this);
}

std::shared_ptr<ast::Node> ast::AssignValueNode::visit(Scope& scope) {
	debug();

	std::shared_ptr<ast::VariableRefNode> keyNode = std::dynamic_pointer_cast<ast::VariableRefNode>(this->key()->visit(scope));
	expect(keyNode, "Key is nullptr");
	expect(keyNode->isToken, "Key is not a NameToken");
	std::string key = keyNode->name.value;
	std::shared_ptr<ast::Node> value = this->value()->visit(scope);
	scope.searchAndSet(key, value);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::BinOPNode::visit(Scope& scope)  {
	debug();
	std::shared_ptr<ast::Node> l = left()->visit(scope);
	while (true) {
		auto var = dynamic_cast<ast::VariableRefNode*>(l.get());
		if (!var) break;

		l = scope.get(var->name.value);
	}
	std::shared_ptr<ast::Node> r = right()->visit(scope);
	while (true) {
		auto var = dynamic_cast<ast::VariableRefNode*>(r.get());
		if (!var) break;

		r = scope.get(var->name.value);
	}

	auto getFloats = [l, r]() -> std::pair<float, float> {
		std::pair<float, float> ret;
		auto n1 = dynamic_cast<ast::NumberNode*>(l.get());
		if (n1)
			ret.first = n1->value;
		else {
			auto s1 = dynamic_cast<ast::StringNode*>(l.get());
			if (s1) {
				const char* str = s1->value.c_str();
				char* err = nullptr;
				ret.first = strtof(str, &err);
				expect(str != err, "Left does not contain a number!");
			} else
				expect(0, (std::string("Left is not a number or string it is: ") + l->toString()).c_str());
		}

		auto n2 = dynamic_cast<ast::NumberNode*>(r.get());
		if (n2)
			ret.second = n2->value;
		else {
			auto s2 = dynamic_cast<ast::StringNode*>(r.get());
			if (s2) {
				const char* str = s2->value.c_str();
				char* err = nullptr;
				ret.second = strtof(str, &err);
				expect(str != err, "Right does not contain a number!");
			} else
				expect(0, (std::string("Right is not a number or string it is: ") + r->toString()).c_str());

		}
		return ret;
	};

	auto getStrings = [l, r]() -> std::pair<std::string, std::string> {
		std::pair<std::string, std::string> ret;

		auto s1 = dynamic_cast<ast::StringNode*>(l.get());
		if (s1)
			ret.first = s1->value;
		else {
			auto n1 = dynamic_cast<ast::NumberNode*>(l.get());
			if (n1)
				ret.first = std::to_string(n1->value);
			else
				expect(0, (std::string("Left is not a number or string it is: ") + l->toString()).c_str());
		}

		auto s2 = dynamic_cast<ast::StringNode*>(r.get());
		if (s2)
			ret.second = s2->value;
		else {
			auto n2 = dynamic_cast<ast::NumberNode*>(r.get());
			if (n2)
				ret.second = std::to_string(n2->value);
			else
				expect(0, (std::string("Right is not a number or string it is: ") + r->toString()).c_str());
		}

		return ret;
	};

	auto getBools = [l, r]() -> std::pair<bool, bool> {
		std::pair<bool, bool> ret{true, true};
		auto b1 = dynamic_cast<ast::BoolNode*>(l.get());
		if (b1)
			ret.first = b1->value;
		else
			ret.first = !dynamic_cast<ast::NilNode*>(l.get());

		auto b2 = dynamic_cast<ast::BoolNode*>(r.get());
		if (b2)
			ret.second = b2->value;
		else
			ret.second = !dynamic_cast<ast::NilNode*>(r.get());

		return ret;
	};

	using OP = ast::token::BinOPToken::OP;

	switch (op.op) {
	case OP::UNK: {
		expect(0, "OP::UNK");
		break;
	}
	case OP::plus: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(lr.first + lr.second);
	}
	case OP::minus: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(lr.first - lr.second);
	}
	case OP::mul: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(lr.first * lr.second);
	}
	case OP::div: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(lr.first / lr.second);
	}
	case OP::pow: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(powf(lr.first, lr.second));
	}
	case OP::mod: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(fmodf(lr.first, lr.second));
	}
	case OP::dotdot: {
		auto lr = getStrings();
		return std::make_shared<ast::StringNode>(lr.first + lr.second);
	}
	case OP::less: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first < lr.second);
	}
	case OP::lessOrEqual: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first <= lr.second);
	}
	case OP::more: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first > lr.second);
	}
	case OP::moreOrEqual: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first >= lr.second);
	}
	case OP::equal: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first == lr.second);
	}
	case OP::notEqual: {
		auto lr = getFloats();
		return std::make_shared<ast::BoolNode>(lr.first != lr.second);
	}
	case OP::and_: {
		auto lr = getBools();
		if (!lr.first)
			return l;
		else
			return r;
	}
	case OP::or_: {
		auto lr = getBools();
		if (lr.first)
			return l;
		else
			return r;
	}
	}
	expect(0, "Not handled!");
	return nilNode;
}

std::shared_ptr<ast::Node> ast::BreakNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::ChunkNode::visit(Scope& scope) {
	debug();
	if (addScope) {
		Scope chunkScope;
		chunkScope.parent = &scope;
		for (auto child : children)
			child->visit(chunkScope);
	} else
		for (auto child : children)
			child->visit(scope);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::FunctionCallNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::FunctionNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::IndexOfNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::LocalAssignValueNode::visit(Scope& scope) {
	debug();

	std::shared_ptr<ast::VariableRefNode> keyNode = std::dynamic_pointer_cast<ast::VariableRefNode>(this->key()->visit(scope));
	expect(keyNode, "Key is nullptr");
	expect(keyNode->isToken, "Key is not a NameToken");
	std::string key = keyNode->name.value;
	std::shared_ptr<ast::Node> value = this->value()->visit(scope);
	scope.set(key, value);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::ReturnNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::RootNode::visit(Scope& scope) {
	debug();
	for (auto child : children)
		child->visit(scope);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::TableNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::UnOPNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::ValueNode::visit(Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::VariableRefListNode::visit(Scope& scope) {
	debug();
	expect(children.size() == 1, "TODO");
	return children[0]->visit(scope);
}

std::shared_ptr<ast::Node> ast::VariableRefNode::visit(Scope& scope) {
	debug();
	return std::make_shared<ast::VariableRefNode>(*this);
}

