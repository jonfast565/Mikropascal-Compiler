/*
 * SemanticAnalyzer.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#include "SemanticAnalyzer.hpp"

// AST Stuff
AbstractTree::AbstractTree() {
	this->root_node = AbstractNodePtr(new AbstractNode());
	this->iterable = this->root_node;
}

AbstractTree::AbstractTree(AbstractNodePtr root) {
	this->root_node = root;
	this->iterable = this->root_node;
}

void AbstractTree::add_move_child(AbstractNodePtr child_node) {
	this->iterable->add_child_node(child_node);
	child_node->set_parent(this->iterable);
	this->iterable = child_node;
}

void AbstractTree::goto_parent() {
	if (this->iterable->get_parent() != nullptr) {
		this->iterable = this->iterable->get_parent();
	}
}

AbstractNodePtr AbstractTree::get_current_parent() {
	if (this->iterable->get_parent() != nullptr) {
		return this->iterable->get_parent();
	}
	return nullptr;
}

void AbstractTree::display_tree() {
    // display tree at the root, which is where it should be
    report_msg("AST Printable Tree");
    display_tree_rec();
}

void AbstractTree::display_tree_rec() {
    // naturally... recursive
    AbstractStackPtr loop = AbstractStackPtr(new AbstractNodeStack());
    AbstractStackPtr reversal = AbstractStackPtr(new AbstractNodeStack());
    loop->push(this->iterable);
    while (!loop->empty()) {
        // get the top and pop
        AbstractNodePtr current = loop->top();
        loop->pop();
        if (current->get_is_rule()) {
            report_msg_type("AST Rule",
                            get_rule_info(current->get_parse_type()));
            // push on in reverse
            for (auto i = current->get_child_begin();
                 i != current->get_child_end(); i++) {
                reversal->push(*i);
            }
            // reverse
            while(!reversal->empty()) {
                loop->push(reversal->top());
                reversal->pop();
            }
        } else if (current->get_is_epsilon()) {
            report_msg("AST Epsilon");
        } else {
            report_msg_type("AST Token",
                            get_token_info(current->get_token()->get_token()).first
                            + ": " + current->get_token()->get_lexeme());
        }
    }
}

AbstractNode::AbstractNode() {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = ROOT;
	this->is_root = true;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(ParseType parse_type) {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = parse_type;
	this->is_root = false;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(AbstractNodePtr parent_node, ParseType parse_type) {
	this->child_nodes = AbstractListPtr(new AbstractNodeList());
	this->parse_type = parse_type;
	this->is_root = false;
    this->is_rule = true;
    this->token = nullptr;
}

AbstractNode::AbstractNode(shared_ptr<Token> token) {
    this->child_nodes = nullptr;
    this->parse_type = LITERAL;
    this->is_root = false;
    this->is_rule = false;
    this->token = token;
}

void AbstractNode::add_child_node(AbstractNodePtr child_node) {
	this->child_nodes->push_back(child_node);
}

void AbstractNode::set_is_root(bool is_root) {
	this->is_root = is_root;
}

bool AbstractNode::get_is_root() {
    return this->is_root;
}

bool AbstractNode::get_is_rule() {
    return this->is_rule;
}

bool AbstractNode::get_is_epsilon() {
    return (this->parse_type == EPSILON);
}

ParseType AbstractNode::get_parse_type() {
	if (this->is_rule != false)
		return this->parse_type;
	else
		return NO_RULE;
}

TokenPtr AbstractNode::get_token() {
    if (is_rule)
        return nullptr;
    else
        return this->token;
}

void AbstractNode::set_parent(AbstractNodePtr parent_node) {
	this->parent_node = parent_node;
}

AbstractNodePtr AbstractNode::get_parent() {
	if (this->parent_node == nullptr) {
		return nullptr;
	} else {
		return this->parent_node;
	}
}

AbstractNodeList::iterator AbstractNode::get_child_begin() {
    return this->child_nodes->begin();
}

AbstractNodeList::iterator AbstractNode::get_child_end() {
    return this->child_nodes->end();
}

// Semantic Analyzer stuff
SemanticAnalyzer::SemanticAnalyzer() {
    // semantic analyzer and no AST
    this->ast = nullptr;
    this->symbols = SymTablePtr(new SymTable());
}

SemanticAnalyzer::SemanticAnalyzer(AbstractTreePtr program_syntax) {
    // semantic analyzer with AST
    this->ast = program_syntax;
    this->symbols = SymTablePtr(new SymTable());
}

void SemanticAnalyzer::attach_syntax(AbstractTreePtr program_syntax) {
    // set program syntax
    this->ast = program_syntax;
}

void SemanticAnalyzer::generate_symbols() {
    // create the symbol table and associated blocks
}

AbstractTreePtr SemanticAnalyzer::get_ast() {
    return this->ast;
}
