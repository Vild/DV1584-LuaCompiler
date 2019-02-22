#include <evaluate.hpp>
#include <ast.hpp>
#include <token.hpp>
#include <expect.hpp>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>

static std::shared_ptr<ast::NilNode> nilNode;

static int indent = 0;
#define debug() Debug debug(this->toString().c_str(), scope)

struct Debug {
	Debug(const char* func, Scope& scope) {
		//scope.print();
		/*indent++;
		for (int i = 0; i < indent; i++) std::cout << "  ";
		std::cout << func << std::endl;*/
	}
	~Debug() { indent--; }
};



struct Object : public ast::Node {
	std::map<std::string, ast::NodePtr> objects;

	Object() {}

	void set(ast::NodePtr key, ast::NodePtr value) {
		objects[key->toString()] = value;
	}

	ast::NodePtr get(ast::NodePtr key){
		auto it = objects.find(key->toString());
		if (it != objects.end())
			return it->second;
		return nullptr;
	}

	virtual ast::NodePtr visit(ast::NodePtr self, Scope& scope) override {
		return self;
	}
};


Scope::~Scope() { /*print();*/ }
void Scope::print() {
	std::cout << "Scope:" << std::endl;
	for (std::pair<std::string, std::shared_ptr<ast::Node>> kv : environment) {
		std::cout << "\t'" << kv.first << "' = '" << kv.second->toString() << "'" << std::endl;
	}
	std::cout << "/Scope" << std::endl;
}

void Scope::set(std::shared_ptr<ast::Node> key, std::shared_ptr<ast::Node> value) {
	environment[key->toString()] = value;
}
void Scope::searchAndSet(std::shared_ptr<ast::Node> key, std::shared_ptr<ast::Node> value) {
	auto str = key->toString();
	auto it = environment.find(str);
	if (it != environment.end())
		environment[str] = value;
	else if (parent)
		parent->searchAndSet(key, value);
	else
		environment[str] = value;
}

std::shared_ptr<ast::Node> Scope::get(std::shared_ptr<ast::Node> key) {
	auto str = key->toString();
	auto it = environment.find(str);
	if (it != environment.end())
		return it->second;
	else if (parent)
		return parent->get(key);
	else
		return nullptr;
}

static std::shared_ptr<ast::ReturnNode> builtin_print(std::shared_ptr<ast::ExpressionListNode> args) {
	bool first = true;
	for (ast::NodePtr child : args->children) {
		if (!first)
			std::cout << "\t";
		first = false;
		auto number = std::dynamic_pointer_cast<ast::NumberNode>(child);
		auto string = std::dynamic_pointer_cast<ast::StringNode>(child);
		if (number)
			std::cout << std::fixed << std::setprecision(1) << number->value;
		else if (string)
			std::cout << string->value;
		else
			std::cout << "<UNK>" << child->toString() << "</UNK>";
	}
	std::cout << std::endl;
	return std::make_shared<ast::ReturnNode>(nilNode);
}

static std::shared_ptr<ast::ReturnNode> builtin_io_write(std::shared_ptr<ast::ExpressionListNode> args) {
	for (ast::NodePtr child : args->children) {
		auto number = std::dynamic_pointer_cast<ast::NumberNode>(child);
		auto string = std::dynamic_pointer_cast<ast::StringNode>(child);
		if (number)
			std::cout << std::fixed << std::setprecision(1) << number->value;
		else if (string) {
			//std::string str = string->value;
			//std::replace(str.begin(), str.end(), '+', ' ');
			//std::cout << str;
			bool nextIsSeq = false;
			for (char ch : string->value) {
				if (nextIsSeq) {
					switch (ch) {
					case 'a':
						std::cout << '\a';
						break;
					case 'b':
						std::cout << '\b';
						break;
					case 'f':
						std::cout << '\f';
						break;
					case 'n':
						std::cout << '\n';
						break;
					case 'r':
						std::cout << '\r';
						break;
					case 't':
						std::cout << '\t';
						break;
					case 'v':
						std::cout << '\v';
						break;
					default:
						std::cout << ch;
						break;
					}
					nextIsSeq = false;
					continue;
				}
				if (ch == '\\')
					nextIsSeq = true;
				else
					std::cout << ch;
			}
		} else
			std::cout << "<UNK>" << child->toString() << "</UNK>";
	}
	return std::make_shared<ast::ReturnNode>(nilNode);
}

static std::shared_ptr<ast::ReturnNode> builtin_io_read(std::shared_ptr<ast::ExpressionListNode> args) {
	auto arg0 = std::dynamic_pointer_cast<ast::StringNode>(args->children[0]);
	expect(arg0->value == "*number", "Unknown read command");

	double number;
	std::cin >> number;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	return std::make_shared<ast::ReturnNode>(std::make_shared<ast::NumberNode>(number));
}

void evaluate(std::shared_ptr<ast::RootNode> root) {
	if (!nilNode)
		nilNode = std::make_shared<ast::NilNode>();

	Scope scope{};

	auto print = std::make_shared<ast::FunctionNode>(builtin_print);
	auto printName = std::make_shared<ast::VariableRefNode>(ast::token::NameToken("print"));
	scope.set(printName, print);

	{
		auto read = std::make_shared<ast::FunctionNode>(builtin_io_read);
		auto readName = std::make_shared<ast::VariableRefNode>(ast::token::NameToken("read"));

		auto write = std::make_shared<ast::FunctionNode>(builtin_io_write);
		auto writeName = std::make_shared<ast::VariableRefNode>(ast::token::NameToken("write"));

		auto io = std::make_shared<Object>();
		io->set(readName, read);
		io->set(writeName, write);

		auto ioName = std::make_shared<ast::VariableRefNode>(ast::token::NameToken("io"));
		scope.set(ioName, io);
	}

	// Initialize print / io.print here

	root->visit(root, scope);
}

std::shared_ptr<ast::Node> ast::NumberNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::StringNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::BoolNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::NilNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::AssignValueNode::visit(ast::NodePtr self, Scope& scope) {
	debug();

	auto key = this->key()->visit(this->key(), scope);
	auto value = this->value()->visit(this->value(), scope);
	scope.searchAndSet(key, value);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::BinOPNode::visit(ast::NodePtr self, Scope& scope)  {
	debug();
	std::shared_ptr<ast::Node> l = left()->visit(left(), scope);
	while (true) {
		auto var = std::dynamic_pointer_cast<ast::VariableRefNode>(l);
		if (!var) break;

		l = scope.get(var);
	}
	std::shared_ptr<ast::Node> r = right()->visit(right(), scope);
	while (true) {
		auto var = std::dynamic_pointer_cast<ast::VariableRefNode>(r);
		if (!var) break;

		r = scope.get(var);
	}

	auto getFloats = [l, r]() -> std::pair<double, double> {
		std::pair<double, double> ret;
		auto n1 = dynamic_cast<ast::NumberNode*>(l.get());
		if (n1)
			ret.first = n1->value;
		else {
			auto s1 = dynamic_cast<ast::StringNode*>(l.get());
			if (s1) {
				const char* str = s1->value.c_str();
				char* err = nullptr;
				ret.first = strtod(str, &err);
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
				ret.second = strtod(str, &err);
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
		return std::make_shared<ast::NumberNode>(pow(lr.first, lr.second));
	}
	case OP::mod: {
		auto lr = getFloats();
		return std::make_shared<ast::NumberNode>(fmod(lr.first, lr.second));
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

std::shared_ptr<ast::Node> ast::BreakNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::ChunkNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	Scope chunkScope;
	chunkScope.parent = &scope;
	Scope* theScope = &scope;
	if (addScope)
		theScope = &chunkScope;

	for (auto child : children) {
		auto obj = child->visit(child, *theScope);

		auto breakN = std::dynamic_pointer_cast<ast::BreakNode>(obj);
		if (breakN)
			return std::make_shared<ast::ReturnNode>(nilNode);

		auto returnN = std::dynamic_pointer_cast<ast::ReturnNode>(obj);
		if (returnN) {
			auto val = returnN->returnValue()->visit(returnN->returnValue(), *theScope);
			auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(val);
			if (ref)
				val = theScope->get(ref);
			return std::make_shared<ast::ReturnNode>(val);
		}
	}
	return nilNode;
}

std::shared_ptr<ast::Node> ast::FunctionCallNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	auto f = this->function()->visit(this->function(), scope);
	{
		auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(f);
		if (ref)
			f = scope.get(f);
	}

	std::shared_ptr<ast::FunctionNode> function = std::dynamic_pointer_cast<ast::FunctionNode>(f);
	expect(function, "Function is not valid!");

	if (function->externalFunction) {
		std::shared_ptr<ast::ExpressionListNode> args = std::make_shared<ast::ExpressionListNode>();
		std::shared_ptr<ast::ExpressionListNode> arguments = std::dynamic_pointer_cast<ast::ExpressionListNode>(this->arguments());
		for (ast::NodePtr child : arguments->children) {
			auto x = child->visit(child, scope);
			auto obj = std::dynamic_pointer_cast<ast::VariableRefNode>(x);
			if (obj)
				args->children.push_back(scope.get(obj));
			else
				args->children.push_back(x);
		}

		auto obj = function->externalFunction(args);
		auto returnN = std::dynamic_pointer_cast<ast::ReturnNode>(obj);
		return returnN->returnValue();
	} else {
		Scope argScope;
		argScope.parent = &scope;
		std::shared_ptr<ast::ExpressionListNode> arguments = std::dynamic_pointer_cast<ast::ExpressionListNode>(this->arguments());
		std::shared_ptr<ast::NameListNode> nameArguments = std::dynamic_pointer_cast<ast::NameListNode>(function->arguments());

		for (int i = 0; i < nameArguments->children.size(); i++) {
			ast::NodePtr value = (i < arguments->children.size()) ? arguments->children[i]->visit(arguments->children[i], scope) : nilNode;
			auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(value);
			if (ref)
				value = scope.get(ref);
			argScope.set(nameArguments->children[i], value);
		}

		auto obj = function->body()->visit(function->body(), argScope);
		auto returnN = std::dynamic_pointer_cast<ast::ReturnNode>(obj);
		return returnN->returnValue();
	}
}

std::shared_ptr<ast::Node> ast::FunctionNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::IndexOfNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	auto obj = this->object()->visit(this->object(), scope);
	auto idx = this->index()->visit(this->index(), scope);

	{
		auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(obj);
		obj = scope.get(obj);
	}

	{
		auto o = std::dynamic_pointer_cast<Object>(obj);
		if (o) {
			auto val = o->get(idx);
			return val->visit(val, scope);
		}
	}

	return nilNode;
}

std::shared_ptr<ast::Node> ast::LocalAssignValueNode::visit(ast::NodePtr self, Scope& scope) {
	debug();

	auto key = this->key()->visit(this->key(), scope);
	auto value = this->value()->visit(this->value(), scope);
	scope.set(key, value);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::ReturnNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::RootNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	for (auto child : children)
		child->visit(child, scope);

	return nilNode;
}

std::shared_ptr<ast::Node> ast::TableNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return table()->visit(table(), scope);
}

std::shared_ptr<ast::Node> ast::UnOPNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return nilNode;
}

std::shared_ptr<ast::Node> ast::ValueNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::ExpressionListNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::NameListNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::FieldListNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::VariableRefNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	return self;
}

std::shared_ptr<ast::Node> ast::ForLoopNode::visit(ast::NodePtr self, Scope& scope) {
	debug();
	Scope loopScope;
	loopScope.parent = &scope;

	double from, to;
	{
		ast::NodePtr fromNode = this->from();
		auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(fromNode);
		if (ref)
			fromNode = scope.get(ref);
		auto fromN = std::dynamic_pointer_cast<ast::NumberNode>(fromNode);
		expect(fromN, "From is not number");
		from = fromN->value;
	}
	{
		ast::NodePtr toNode = this->to();
		auto ref = std::dynamic_pointer_cast<ast::VariableRefNode>(toNode);
		if (ref)
			toNode = scope.get(ref);
		auto toN = std::dynamic_pointer_cast<ast::NumberNode>(toNode);
		expect(toN, "To is not number");
		to = toN->value;
	}

	for (double idx = from; idx <= to; idx += 1) {
		loopScope.set(counter(), std::make_shared<ast::NumberNode>(idx));
		body()->visit(body(), loopScope);
	}

	return nilNode;
}


std::shared_ptr<ast::Node> ast::IfNode::visit(ast::NodePtr self, Scope& scope) {
	debug();

	ast::NodePtr check = this->check()->visit(this->check(), scope);
	auto boolCheck = std::dynamic_pointer_cast<ast::BoolNode>(check);

	expect(boolCheck, "check is not a boolNode");
	if (boolCheck->value)
		return body()->visit(body(), scope);
	else
		return elseBody()->visit(elseBody(), scope);
}
