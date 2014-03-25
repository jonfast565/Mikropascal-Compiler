/*
 * SemanticAnalyzer.hpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#ifndef semanticanalyzer_h
#define semanticanalyzer_h

#include "Standard.hpp"
#include "Rules.hpp"
#include "Tokens.hpp"
#include "Symbols.hpp"

class AbstractNode;
class AbstractTree;
class SemanticAnalyzer;
class CodeBlock;

using AbstractNodePtr = shared_ptr<AbstractNode>;
using AbstractNodeList = vector<AbstractNodePtr>;
using AbstractListPtr = shared_ptr<AbstractNodeList>;
using AbstractNodeStack = stack<AbstractNodePtr>;
using AbstractStackPtr = shared_ptr<AbstractNodeStack>;
using AbstractTreePtr = shared_ptr<AbstractTree>;
using SemanticAnalyzerPtr = shared_ptr<SemanticAnalyzer>;
using TokenPtr = shared_ptr<Token>;
using CodeBlockPtr = shared_ptr<CodeBlock>;

// AST Stuff
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
    virtual ~AbstractNode(){};
    void add_child_node(AbstractNodePtr child_node);
    void set_is_root(bool is_root);
    bool get_is_root();
    bool get_is_rule();
    bool get_is_epsilon();
    ParseType get_parse_type();
    TokenPtr get_token();
    void set_parent(AbstractNodePtr parent_node);
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
	virtual ~AbstractTree(){};
    AbstractNodePtr get_root_node();
    void add_move_child(AbstractNodePtr child_node);
	void goto_parent();
    void display_tree();
    void push_children(AbstractNodePtr current_node,
                       AbstractStackPtr current_symbols);
};

class Generator {
public:
    virtual void generate_pre() = 0;
    virtual void generate_post() = 0;
};

class CodeBlock: public Generator {
private:
    string rule;
public:
    CodeBlock(string rule): rule(rule){};
    virtual ~CodeBlock() = default;
};

enum IOAction {
    IO_READ,
    IO_WRITE
};

class IOBlock: public CodeBlock {
private:
    SymbolListPtr args;
    IOAction action;
public:
    IOBlock(IOAction action): CodeBlock("IOBlock"), action(action) {
        this->args = SymbolListPtr(new SymbolList());
    }
    virtual ~IOBlock() = default;
};

enum LoopType {
    WHILELOOP,
    FORLOOP
};

class LoopBlock: public CodeBlock {
private:
    LoopType type;
public:
    LoopBlock(LoopType type): CodeBlock("LoopBlock"), type(type){};
    virtual ~LoopBlock() = default;
};

class ConditionalBlock: public CodeBlock {
private:
public:
    ConditionalBlock();
    virtual ~ConditionalBlock() = default;
};

class AssignmentBlock: public CodeBlock {
private:
    bool processed;
public:
    AssignmentBlock();
    virtual ~AssignmentBlock() = default;
    void convert_postfix();
};

enum ActivationType {
    PROCEDURE,
    FUNCTION
};

class ActivationBlock: public CodeBlock {
private:
    SymbolPtr record;
    ActivationType type;
public:
    ActivationBlock(ActivationType type, SymbolPtr record):
    CodeBlock("ActivationType"), type(type) {
        this->record = record;
    }
    virtual ~ActivationBlock() = default;
};

class SemanticAnalyzer {
private:
	AbstractTreePtr ast;
    SymTablePtr symbols;
    bool processed;
    typedef CodeBlockPtr (*CodeActionMethod)(AbstractNodePtr, CodeBlockPtr);
public:
    SemanticAnalyzer();
    SemanticAnalyzer(AbstractTreePtr program_syntax);
    virtual ~SemanticAnalyzer() = default;
    // debug
    CodeBlockPtr iterate_to_rule_generate(ParseType rule, CodeActionMethod method, CodeBlockPtr block = nullptr);
    AbstractTreePtr get_ast();
    void attach_syntax(AbstractTreePtr program_syntax);
    void push_children(AbstractNodePtr current_node, AbstractStackPtr current_symbols);
    void generate_symbols();
    void generate_blocks();
};

#endif
