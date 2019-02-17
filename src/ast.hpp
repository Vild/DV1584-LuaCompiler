/* -*- mode: c++; c-set-style: cc-mode -*- */

#pragma once

#include <memory>

namespace ast {
	typedef std::unique_ptr<Node> NodePtr;

	class Node {
	public:
		std::vector<NodePtr> children;
		int id;

		Node() {
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
				child.toDot(out);
			}
		}
		virtual std::string_view toString() {
			return typeid(this).name();
		}
	};

	typedef std::unique_ptr<ast::RootNode> RootNodePtr;
	class RootNode : public Node {};

	typedef std::unique_ptr<ast::ChunkNode> ChunkNodePtr;
	class ChunkNode : public Node {};
	class ReturnNode : public Node {
	public:
		NodePtr returnValue() { return children[0]; }

		ReturnNode() {};
		ReturnNode(NodePtr returnValue) : children{returnValue} {}
	};
	class BreakNode : public Node {};

	class ReturnNode : public Node {
		NodePtr key() { return children[0]; }
		NodePtr value() { return children[1]; }

		ReturnNode() {};
		ReturnNode(NodePtr key, NodePtr value) : children{key, value} {}
	};
}
