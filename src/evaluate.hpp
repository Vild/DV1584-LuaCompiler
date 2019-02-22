#pragma once

#include <memory>
#include <map>

namespace ast {
	class Node;
	class RootNode;
}

struct Scope {
	Scope* parent = nullptr;
	std::map<std::string, std::shared_ptr<ast::Node>> environment;

	~Scope();
	void print();

	void set(std::shared_ptr<ast::Node> key, std::shared_ptr<ast::Node> value);
	void searchAndSet(std::shared_ptr<ast::Node> key, std::shared_ptr<ast::Node> value);
	std::shared_ptr<ast::Node> get(std::shared_ptr<ast::Node> key);
};

void evaluate(std::shared_ptr<ast::RootNode> root);
