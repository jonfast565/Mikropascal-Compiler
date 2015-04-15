#ifndef SYNTAXTREE_HPP_
#define SYNTAXTREE_HPP_

#include "Standard.hpp"
#include "Rules.hpp"
#include "Tokens.hpp"
#include "Symbols.hpp"

class AbstractNode;
class AbstractTree;

using AbstractNodePtr = shared_ptr<AbstractNode>;
using AbstractNodeList = vector<AbstractNodePtr>;
using AbstractListPtr = shared_ptr<AbstractNodeList>;
using AbstractNodeStack = stack<AbstractNodePtr>;
using AbstractStackPtr = shared_ptr<AbstractNodeStack>;
using AbstractTreePtr = shared_ptr<AbstractTree>;
using TokenPtr = shared_ptr<Token>;
using TokenList = vector<TokenPtr>;
using TokenListPtr = shared_ptr<TokenList>;

class AbstractNode {
private:
	bool is_root;
	bool is_rule;
	AbstractNodePtr parent_node;
	AbstractListPtr child_nodes;
	ParseType parse_type;
	TokenPtr token;
public:
	AbstractNode();
	AbstractNode(AbstractNodePtr parent_node, ParseType parse_type);
	AbstractNode(ParseType parse_type);
	AbstractNode(TokenPtr token);
	virtual ~AbstractNode() = default;
	void add_child_node(AbstractNodePtr child_node);
	void set_is_root(bool is_root);
	void set_parent(AbstractNodePtr parent_node);
	bool get_is_root();
	bool get_is_rule();
	bool get_is_epsilon();
	ParseType get_parse_type();
	TokenPtr get_token();
	AbstractNodePtr get_parent();
	AbstractNodeList::iterator get_child_begin();
	AbstractNodeList::iterator get_child_end();
};

class AbstractTree {
private:
	AbstractNodePtr root_node;
	AbstractNodePtr iterable;
	AbstractNodePtr get_current_parent();
	void display_tree_rec();
public:
	AbstractTree();
	AbstractTree(AbstractNodePtr root);
	virtual ~AbstractTree() = default;
	void add_move_child(AbstractNodePtr child_node);
	void goto_parent();
	void display_tree();
	void push_children(AbstractNodePtr current_node,
		AbstractStackPtr current_symbols);
	AbstractNodePtr get_root_node();
};

#endif