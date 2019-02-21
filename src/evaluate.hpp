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

	void set(std::string key, std::shared_ptr<ast::Node> value);
	void searchAndSet(std::string key, std::shared_ptr<ast::Node> value);
	std::shared_ptr<ast::Node> get(std::string key);
};

void evaluate(std::shared_ptr<ast::RootNode> root);
