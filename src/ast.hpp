/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once

#include <memory>

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

		virtual void print(std::ostream& out) {
			out << toString() << std::endl;
		}

		virtual void toDot(std::ostream& out) {
			out << "node" << id << "[label=\"" << toString() << "\"];" << std::endl;
			for (const NodePtr& child : children) {
				out << "node" << id << "-> node" << child->id << ";" << std::endl;
				child->toDot(out);
			}
		}
		virtual std::string_view toString() {
			return typeid(this).name();
		}
	};

	class RootNode : public Node {};
	class ChunkNode : public Node {};
	class ReturnNode : public Node {
	public:
		NodePtr returnValue() { return children[0]; }

		ReturnNode() {};
		ReturnNode(NodePtr returnValue) : Node{returnValue} {}
	};
	class BreakNode : public Node {};

	class AssignValueNode : public Node {
	public:
		NodePtr key() { return children[0]; }
		NodePtr value() { return children[1]; }

		AssignValueNode() {};
		AssignValueNode(NodePtr key, NodePtr value) : Node{key, value} {}
	};

	class VariableRefNode : public Node {
	public:
		NodePtr variable() { return children[0]; }

		VariableRefNode() {};
		VariableRefNode(NodePtr variable) : Node{variable} {}
	};

	class IndexOfNode : public Node {
	public:
		NodePtr object() { return children[0]; }
		NodePtr index() { return children[1]; }

		IndexOfNode() {};
		IndexOfNode(NodePtr object, NodePtr index) : Node{object, index} {}
	};

	class VariableRefListNode : public Node {
	public:
		VariableRefListNode() {};
	};

	class ValueNode : public Node {
	public:
		std::unique_ptr<ast::token::Token> value;

		ValueNode() {}
		template <typename T>
		ValueNode(T value) : value(std::make_unique<T>(value)) {}
	};

	class BinOPNode : public Node {
	public:
		NodePtr left() { return children[0]; }
		ast::token::BinOPToken op;
		NodePtr right() { return children[1]; }

		BinOPNode() {};
		BinOPNode(NodePtr left, ast::token::BinOPToken op, NodePtr right) : Node{left, right}, op(op) {}
	};

	class UnOPNode : public Node {
	public:
		ast::token::UnOPToken op;
		NodePtr value() { return children[0]; }

		UnOPNode() {};
		UnOPNode(ast::token::UnOPToken op, NodePtr right) : Node{right}, op(op) {}
	};

	class FunctionCall : public Node {
	public:
		NodePtr function() { return children[0]; }

		FunctionCall() {};
		FunctionCall(NodePtr function) : Node{function} {}
	};

}
