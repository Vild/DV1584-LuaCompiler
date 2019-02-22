/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once

#include <memory>
#include <cxxabi.h>
#include <evaluate.hpp>
#include <token.hpp>
#include <vector>

namespace ast {
	class Node;
	typedef std::shared_ptr<Node> NodePtr;

	class Node {
	public:
		std::vector<NodePtr> children;
		int id;

		Node() : Node({}) { }

		Node(std::initializer_list<NodePtr> l) : children(l) {
			static int idCounter = 0;
			id = idCounter++;
		}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) = 0;

		virtual void print(std::ostream& out, int indent = 0) {
			for (int i = 0; i < indent; i++)
				out << "  ";
			out << toString() << std::endl;
			for (const NodePtr& child : children)
				child->print(out, indent + 1);
		}

		virtual void toDot(std::ostream& out) {
			out << "\tnode" << id << "[label=\"" << toString() << "\"];" << std::endl;
			for (const NodePtr& child : children) {
				out << "\tnode" << id << " -> node" << child->id << ";" << std::endl;
				child->toDot(out);
			}
		}
		virtual std::string toString() {
			int status;
			std::string str = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
			return str;
		}
	};

	class StringNode : public Node {
	public:
		std::string value;

		StringNode() {}
		StringNode(ast::token::QuotedToken text) : value(text.value) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			return Node::toString() + ": " + value;
		}
	};

	class NilNode : public Node {
	public:
		NilNode() {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class BoolNode : public Node {
	public:
		bool value;

		BoolNode() {}
		BoolNode(bool value) : value(value) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			return Node::toString() + ": " + std::to_string(value);
		}
	};

	class NumberNode : public Node {
	public:
		double value;

		NumberNode() {}
		NumberNode(double value) : value(value) {}
		NumberNode(ast::token::NumberToken text) : value(text.value) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			return Node::toString() + ": " + std::to_string(value);
		}
	};

	class RootNode : public Node {
	public:
		RootNode() {}
		RootNode(std::initializer_list<NodePtr> l) : Node(l) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class ChunkNode : public Node {
	public:
		ChunkNode() {}
		ChunkNode(std::initializer_list<NodePtr> l) : Node(l) {}

		bool addScope = true;

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class ReturnNode : public Node {
	public:
		NodePtr returnValue() { return children[0]; }

		ReturnNode() {};
		ReturnNode(NodePtr returnValue) : Node{returnValue} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class BreakNode : public Node {
	public:
		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class LocalAssignValueNode : public Node {
	public:
		NodePtr key() { return children[0]; }
		NodePtr value() { return children[1]; }

		LocalAssignValueNode() {};
		LocalAssignValueNode(NodePtr key, NodePtr value) : Node{key, value} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class AssignValueNode : public Node {
	public:
		NodePtr key() { return children[0]; }
		NodePtr value() { return children[1]; }

		AssignValueNode() {};
		AssignValueNode(NodePtr key, NodePtr value) : Node{key, value} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class VariableRefNode : public Node {
	public:
		NodePtr variable() { return children[0]; }
		ast::token::NameToken name;
		bool isToken;

		VariableRefNode() {};
		VariableRefNode(NodePtr variable) : Node{variable}, isToken(false) {}
		VariableRefNode(ast::token::NameToken name) : Node{}, name(name), isToken(true) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			if (!isToken)
				return Node::toString();
			return Node::toString() + ": " + name.value;
		}
	};

	class IndexOfNode : public Node {
	public:
		NodePtr object() { return children[0]; }
		NodePtr index() { return children[1]; }

		IndexOfNode() {};
		IndexOfNode(NodePtr object, NodePtr index) : Node{object, index} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class ExpressionListNode : public Node {
	public:
		ExpressionListNode() {}
		ExpressionListNode(std::initializer_list<NodePtr> l) : Node(l) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class NameListNode : public Node {
	public:
		NameListNode() {}
		NameListNode(std::initializer_list<NodePtr> l) : Node(l) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class ValueNode : public Node {
	public:
		std::unique_ptr<ast::token::Token> value;

		ValueNode() {}
		template <typename T>
		ValueNode(T value) : value{new T(value)} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			int status;
			std::string str = abi::__cxa_demangle(typeid(*value.get()).name(), 0, 0, &status);

			return Node::toString() + ": " + str;
		}
	};

	class BinOPNode : public Node {
	public:
		NodePtr left() { return children[0]; }
		ast::token::BinOPToken op;
		NodePtr right() { return children[1]; }

		BinOPNode() {};
		BinOPNode(NodePtr left, ast::token::BinOPToken op, NodePtr right) : Node{left, right}, op(op) {}
		BinOPNode(NodePtr left, ast::token::MinusOPToken op, NodePtr right) : Node{left, right}, op(ast::token::BinOPToken(ast::token::BinOPToken::OP::minus)) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			return Node::toString() + ": " + op.toString();
		}
	};

	class UnOPNode : public Node {
	public:
		ast::token::UnOPToken op;
		NodePtr value() { return children[0]; }

		UnOPNode() {};
		UnOPNode(ast::token::UnOPToken op, NodePtr right) : Node{right}, op(op) {}
		UnOPNode(ast::token::MinusOPToken, NodePtr right) : Node{right}, op(ast::token::UnOPToken{ast::token::UnOPToken::OP::minus}) {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;

		virtual std::string toString() override {
			return Node::toString() + ": " + op.toString();
		}
	};

	class FunctionCallNode : public Node {
	public:
		NodePtr function() { return children[0]; }
		NodePtr arguments() { return children[1]; }

		FunctionCallNode() {};
		FunctionCallNode(NodePtr function, NodePtr arguments) : Node{function, arguments} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class FunctionNode : public Node {
	public:
		typedef std::shared_ptr<ReturnNode> (*ExternalFunction_t)(std::shared_ptr<ast::ExpressionListNode> arguments);
		NodePtr arguments() { return children[0]; }
		NodePtr body() { return children[1]; }

		FunctionNode() {};
		FunctionNode(NodePtr arguments, NodePtr body) : Node{arguments, body} {}
		FunctionNode(ExternalFunction_t externalFunction) : Node{}, externalFunction(externalFunction) {}

		ExternalFunction_t externalFunction = nullptr;

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class TableNode : public Node {
	public:
		TableNode() {};

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class ForLoopNode : public Node {
	public:
		NodePtr counter() { return children[0]; }
		NodePtr from() { return children[1]; }
		NodePtr to() { return children[2]; }
		NodePtr body() { return children[3]; }

		ForLoopNode() {};
		ForLoopNode(NodePtr counter, NodePtr from, NodePtr to, NodePtr body) : Node{counter, from, to, body} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class IfNode : public Node {
	public:
		NodePtr check() { return children[0]; }
		NodePtr body() { return children[1]; }
		NodePtr elseBody() { return children[2]; }

		IfNode() {};
		IfNode(NodePtr check, NodePtr body, NodePtr elseBody) : Node{check, body, elseBody} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};

	class RepeatNode : public Node {
	public:
		NodePtr body() { return children[0]; }
		NodePtr check() { return children[1]; }

		RepeatNode() {};
		RepeatNode(NodePtr body, NodePtr check) : Node{body, check} {}

		virtual std::shared_ptr<Node> visit(ast::NodePtr self, Scope& scope) override;
	};
}
